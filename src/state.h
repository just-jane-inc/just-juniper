#pragma once
#include <string>
#include <vector>

#include "constants.h"
#include "raylib.h"

#include "game_world.h"
#include "rngesus.h"

#define FLOOR_Y 24

const int SPEED = 16;
enum State { UNSET, IDLE, WALKING, SLEEPING, EATING, HEADPAT, HYDRATE };

void DrawTama(Texture2D texture, Vector2 position);

/**
 * state template class that all other states must inherit
 * and override.
 *
 * This is a comment! - ty999999
 */
class TamaState {
public:
  State state;
  Rectangle window;

  /**
   * @brief Called when a state is entered.
   * @param position the Vecto2 position that tama current is.
   * state must update tama position internally using this pointer.
   */
  virtual void EnterState(Vector2 *position) = 0;

  virtual void ExitState() = 0;

  /**
   * @brief provides state opportunity to preform its updates. returns UNSET
   * to signal no transition should occur (stay in this state) otherwise
   * returns the state we should transition to.
   *
   * @param frameCounter the current animation frame counter.
   */
  virtual State Update(long frameCounter) = 0;
  virtual void Draw() = 0;

protected:
  std::vector<Texture2D> _texturesLeft;
  std::vector<Texture2D> _texturesRight;
  int _counter;
};

class Idle : public TamaState {
public:
  Idle() {}

  Idle(std::string assetsDirectory) {
    state = IDLE;
    std::string path = assetsDirectory + "idle/idle.png";
    Image img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _texturesRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _texturesLeft.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _position = *position;
    counter = 0;

    _velocity = Flip(0.5) ? -1 : 1;
  };

  void ExitState() {}

  State Update(long frameCounter) {
    if (frameCounter % SPEED == 0) {
      counter += 1;
      if (Flip(0.01)) {
        _velocity *= -1;
      }
    } else {
      return UNSET;
    }

    if (counter < 128 || Flip(.80)) {
      return UNSET;
    }

    return Flip(0.75) ? WALKING : SLEEPING;
  }

  void Draw() {
    int idx = counter % _texturesRight.size();

    if (_velocity > 0) {
      DrawTama(_texturesRight[idx], _position);
    } else {
      DrawTama(_texturesLeft[idx], _position);
    }
  }

private:
  Vector2 _position;
  int counter;
  float _velocity;
};

class Walking : public TamaState {
public:
  Walking() {}

  Walking(std::string assetsDirectory) {
    state = WALKING;

    std::string path = assetsDirectory + "walking/walking.png";
    Image img = LoadImage(path.c_str());

    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _texturesRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _texturesLeft.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _counter = 0;
    _position = position;
    _velocity = 1;
  }

  void ExitState() {}

  State Update(long frameCounter) {
    if (frameCounter % SPEED == 0) {
      _counter += 1;

      if (Flip(0.01)) {
        _velocity *= -1;
      }

      _position->x += _velocity;

      if (IsColliding()) {
        _velocity = _velocity * -1;
        _position->x += 2 * _velocity;
      }

    } else {
      return UNSET;
    }

    if (_counter < 128 || Flip(0.80)) {
      return UNSET;
    }

    return Flip(0.75) ? IDLE : SLEEPING;
  }

  void Draw() {
    Texture2D frame;
    int idx = _counter % _texturesRight.size();
    if (_velocity > 0) {
      frame = _texturesRight[idx];
    } else {
      frame = _texturesLeft[idx];
    }

    DrawTama(frame, *_position);
  }

private:
  Vector2 *_position;
  int _counter;
  int _velocity;

  bool IsColliding() {
    return _position->x <= window.x
           || (_position->x + 48) >= window.x + window.width;
  }
};

class Eating : public TamaState {
public:
  Eating() {}

  Eating(std::string assetsDirectory) {
    state = EATING;
    _assetsDirectory = assetsDirectory;

    // the eating state needs to manage tama walking to food
    // as well as eating that food. for this she has two sets
    // of animations thata we load in here.
    std::string path = assetsDirectory + "eating/eating.png";
    Image img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _texturesRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _texturesLeft.push_back(LoadTextureFromImage(partImage));
    }

    path = assetsDirectory + "walking/walking.png";
    img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _walkingRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _walakingLeft.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _counter = 0;
    _position = position;

    // figure out if juniper is on left or right side of screen,
    // spawn food on opposite side, walk toward it then eat
    bool right = (_position->x / TamaConstant::TAMA_SIZE.x)
                 > TamaConstant::WINDOW_WIDTH / 2.0f;

    float foodx;
    // spawn food on the opposite side of the screen, if juniper
    // is on the right place food on the left using the x coordinate of the
    // screen. otherwise get the left side and subtract the width of the food
    // iteself.
    if (right) {
      foodx = TamaConstant::SCREEN_X;
    } else {
      foodx = TamaConstant::SCREEN_X + TamaConstant::SCREEN_WIDTH
              - _texturesRight[0].width;
    }

    _food = new Food(_assetsDirectory, foodx);
  }

  void ExitState() {
    _counter = 0;
    _velocity = 0;
    delete _food;
  }

  State Update(long frameCounter) {
    if (frameCounter % SPEED == 0) {
      _counter += 1;
    } else { // we only want to update on certain frames
      return UNSET;
    }

    bool right = IsFacingRight();
    bool isEating = IsEating();

    if (isEating) {
      // NO IDEA WHAT THIS IS WTF?
      if (_counter <= 7) {
        return UNSET;
      }

      if (_food->Consume(*_position)) {
        return Flip(0.5) ? IDLE : WALKING;
      }

      return UNSET;
    }

    if (right) {
      _position->x += 2;
    } else {
      _position->x -= 2;
    }

    return UNSET;
  }

  virtual void Draw() {
    // right is the direction the juniper is facing
    bool right = IsFacingRight();
    bool isEating = IsEating();

    Texture2D tama;

    if (isEating) {
      if (right) {
        tama = _texturesRight[_counter % _texturesRight.size()];
      } else {
        tama = _texturesLeft[_counter % _texturesRight.size()];
      }
    } else {
      if (right) {
        tama = _walkingRight[_counter % _walkingRight.size()];
      } else {
        tama = _walakingLeft[_counter % _walakingLeft.size()];
      }
    }

    int xoffset = 0;

    DrawTama(tama, *_position);

    _food->Draw();
  }

private:
  bool IsFacingRight() { return _position->x - _food->Position.x < 0; }

  bool IsEating() {
    bool right = IsFacingRight();
    bool isEating;

    if (right) {
      int distance =
          _food->Position.x - _position->x - (_walkingRight[0].width * 2);
      isEating = std::abs(distance) <= 4;
    } else {
      isEating = std::abs(_food->Position.x - _position->x) <= 4;
    }

    return isEating;
  }

  std::vector<Texture2D> _walkingRight;
  std::vector<Texture2D> _walakingLeft;
  Vector2 *_position;
  int _counter;
  float _velocity;
  std::string _assetsDirectory;
  Food *_food;
};

class Sleeping : public TamaState {
public:
  Sleeping() {}

  Sleeping(std::string assetsDirectory) {
    state = SLEEPING;

    std::string path = assetsDirectory + "sleeping/sleeping.png";
    Image img = LoadImage(path.c_str());

    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _texturesRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _texturesLeft.push_back(LoadTextureFromImage(partImage));
    }

    path = assetsDirectory + "enter_sleep/enter_sleep.png";
    img = LoadImage(path.c_str());

    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _enterSleepRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _enterSleepLeft.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _position = *position;
    _counter = 0;
  };

  void ExitState() {}

  State Update(long frameCounter) {
    if (frameCounter % SPEED == 0) {
      _counter += 1;
    } else {
      return UNSET;
    }

    if (_counter < 128 || Flip(95)) {
      return UNSET;
    }

    return Flip(0.75) ? IDLE : WALKING;
  };

  virtual void Draw() {
    if (_counter < _enterSleepLeft.size() - 1) {
      int idx = _counter % _enterSleepRight.size();
      DrawTama(_enterSleepRight[idx], _position);
    } else {
      int idx = _counter % _texturesRight.size();
      DrawTama(_texturesRight[idx], _position);
    }
  }

private:
  std::vector<Texture2D> _enterSleepRight;
  std::vector<Texture2D> _enterSleepLeft;
  Vector2 _position;
};

class Headpat : public TamaState {
public:
  Headpat() {}

  Headpat(std::string assetsDirectory) {
    state = HEADPAT;

    std::string path = assetsDirectory + "head-pat/headpat.png";
    Image img = LoadImage(path.c_str());

    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _texturesRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _texturesLeft.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _position = *position;
    _counter = 0;
  }

  void ExitState() {}

  State Update(long frameCounter) {
    if (frameCounter % SPEED == 0) {
      _counter += 1;
    } else {
      return UNSET;
    }

    if (_counter >= 32) {
      return Flip(0.20) ? WALKING : IDLE;
    }

    return UNSET;
  }

  void Draw() {
    DrawTama(_texturesRight[_counter % _texturesRight.size()], _position);
  }

private:
  Vector2 _position;
  int _counter;
};

class Hydrate : public TamaState {
public:
  Hydrate() : _bowl("", -1) {}
  Hydrate(std::string assetsDirectory, Water waterBowl) : _bowl(waterBowl) {
    std::string path = assetsDirectory + "walking/walking.png";
    Image img = LoadImage(path.c_str());

    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _texturesRight.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _texturesLeft.push_back(LoadTextureFromImage(partImage));
    }
  }

  virtual void EnterState(Vector2 *position) {
    _isHunting = true;
    _position = position;
    _velocity = (_bowl.Position.x - position->x) > 0 ? 1 : -1;
  };

  void ExitState() {

  };

  // Called each tick, returns UNSET to signal no transition should occur
  virtual State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    int consumesRemaining = 6;
    // if tama is at least four pixels away from the water bowl
    if (std::abs(_bowl.Position.x - _position->x) <= 4) {
      _isHunting = false; // we are no longer hunting, we got it
      consumesRemaining = _bowl.Consume(*_position);
    } else {
      _position->x += _velocity;
    }

    return consumesRemaining > 0 ? UNSET : IDLE;
  }

  virtual void Draw() = 0;

private:
  TamaState *_walking;
  Vector2 *_position;
  Water _bowl;
  int _velocity;
  bool _isHunting;
};
