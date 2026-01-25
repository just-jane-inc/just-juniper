#include <string>
#include <vector>

#include "raylib.h"

#include "game_world.h"
#include "rngesus.h"

#ifndef STATE
#define STATE

const int SPEED = 16;
enum State { UNSET, IDLE, WALKING, SLEEPING, EATING, HEADPAT };

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
      if (Flip(0.15)) {
        _velocity *= -1;
      }
    } else {
      return UNSET;
    }

    if (counter < 32 || Flip(.80)) {
      return UNSET;
    }

    return Flip(0.1) ? HEADPAT : WALKING;
  }

  void Draw() {
    int idx = counter % _texturesRight.size();

    if (_velocity > 0) {
      DrawTextureEx(_texturesRight[idx], _position, 0, 2.0f, WHITE);
    } else {
      DrawTextureEx(_texturesLeft[idx], _position, 0, 2.0f, WHITE);
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

      if (Flip(0.15)) {
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

    if (_counter < 64 || Flip(0.80)) {
      return UNSET;
    }

    return Flip(0.1) ? EATING : SLEEPING;
  }

  void Draw() {
    Texture2D frame;
    int idx = _counter % _texturesRight.size();
    if (_velocity > 0) {
      frame = _texturesRight[idx];
    } else {
      frame = _texturesLeft[idx];
    }

    DrawTextureEx(frame, *_position, 0, 2.0f, WHITE);
  }

private:
  Vector2 *_position;
  int _counter;
  int _velocity;

  bool IsColliding() {
    return _position->x < 30 || (_position->x + 24) > window.width;
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
    Vector2 p = Vector2{.x = 0, .y = 0};
    _food = new Food(_assetsDirectory, *position);
    _counter = 0;
    _position = position;
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

    if (_counter > 7 && _food->ChompFromLeft() <= 0) {
      return Flip(0.5) ? IDLE : WALKING;
    }

    return UNSET;
  }

  virtual void Draw() {
    Texture2D tama;
    tama = _texturesRight[_counter % _texturesRight.size()];

    int xoffset = 0;
    if (_velocity > 0) {
      xoffset = (tama.width + 16);
    } else {
      xoffset = -(tama.width + 16);
    }

    Vector2 foodPosition =
        Vector2{.x = _position->x + xoffset, .y = _position->y};
    DrawTextureEx(tama, *_position, 0, 2.0f, WHITE);

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

    if (_counter < 16) {
      return UNSET;
    }

    if (Flip(0.01)) {
      return WALKING;
    }

    return UNSET;
  };

  virtual void Draw() {
    if (_counter < _enterSleepLeft.size() - 1) {
      int idx = _counter % _enterSleepRight.size();
      DrawTextureEx(_enterSleepRight[idx], _position, 0, 2.0f, WHITE);
    } else {
      int idx = _counter % _texturesRight.size();
      DrawTextureEx(_texturesRight[idx], _position, 0, 2.0f, WHITE);
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

    if (_counter >= 16) {
      return Flip(0.20) ? WALKING : IDLE;
    }

    return UNSET;
  }

  void Draw() {
    DrawTextureEx(_texturesRight[_counter % _texturesRight.size()], _position,
                  0, 2.0f, WHITE);
  }

private:
  Vector2 _position;
  int _counter;
};

#endif
