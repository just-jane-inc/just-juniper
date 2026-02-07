#pragma once
#include <string>
#include <vector>

#include "raylib.h"

#include "game_world.h"
#include "rngesus.h"

#define FLOOR_Y 24

const int SPEED = 16;
enum State { UNSET, IDLE, WALKING, SLEEPING, EATING, HEADPAT, HYDRATE };

void DrawTama(Texture2D texture, Vector2 position);

class TamaState {
public:
  State state;
  Rectangle window;
  virtual void EnterState(Vector2 *position) = 0;
  virtual void ExitState() = 0;

  // Called each tick, returns UNSET to signal no transition should occur
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
  }

  void EnterState(Vector2 *position) {
    _counter = 0;
    _position = position;

    Vector2 food_position = {
        _position->x + _texturesRight[0].width,
        FLOOR_Y - 8};
    _food = new Food(
        _assetsDirectory,
        _position->x + (_texturesRight[0].width - 2) * 2);
  }

  void ExitState() {
    _counter = 0;
    _velocity = 0;
    delete _food;
  }

  State Update(long frameCounter) {
    if (frameCounter % SPEED == 0) {
      _counter += 1;
    } else {
      return UNSET;
    }

    if (_counter > 7 && _food->ConsumeFromLeft() <= 0) {
      return Flip(0.5) ? IDLE : WALKING;
    }

    return UNSET;
  }

  virtual void Draw() {
    Texture2D tama;
    tama = _texturesRight[_counter % _texturesRight.size()];

    int xoffset = 0;

    DrawTama(tama, *_position);

    _food->Draw();
  }

private:
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
    if (std::abs(_bowl.Position.x - _position->x) <= 4) {
      _isHunting = false;
      if (_velocity > 0) {
        consumesRemaining = _bowl.ConsumeFromLeft();
      } else {
        consumesRemaining = _bowl.ConsumeFromRight();
      }
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
