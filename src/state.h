#pragma once
#include <iostream>
#include <string>
#include <vector>

#include "constants.h"
#include "raylib.h"

#include "game_world.h"
#include "rngesus.h"

#define FLOOR_Y 24

const int SPEED = 16;
enum State { UNSET, IDLE, WALKING, SLEEPING, EATING, HEADPAT, HYDRATE, GAME };

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

  virtual bool TryExitState(State next) = 0;

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

  bool TryExitState(State next) {
    return true;
  }

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

  bool TryExitState(State next) {
    return true;
  }

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

    // The eating state maintains two internal states and should likely
    // be reworked slightly. The first internal state is the hunting/seeking
    // state where juniper walks in the direction of food. This is followed
    // by an internal eating state where she actually calls Consume on food.
    //
    // This could, and probably should, be reworked to literally be two
    // distinct states or to just cleanup how the swap occurrs. Note
    // that most of the oddities of this state stem from this choice
    // to internally encode two distinct behaviors in one state.
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
    _dined = false;
    _eatingAnimationCounter = 0;
    _walkingAnimationCounter = 0;
    _position = position;

    // figure out if juniper is on left or right side of screen,
    // spawn food on opposite side, walk toward it then eat
    bool right = _position->x > (TamaConstant::SCREEN_WIDTH / 2.0f);

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

  bool TryExitState(State next) {
    if(!_dined){
      return false;
    } 
    _walkingAnimationCounter = 0;
    _eatingAnimationCounter = 0;
    delete _food;
    return true;
  }

  State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    if (IsEating()) {
      _eatingAnimationCounter += 1;

      // This is using forbidden knowledge about the animation.
      // At the eating animation frame we want to state consuming the food item.
      // this is brittle, will break, and should be done away with.
      if (_eatingAnimationCounter < 7) {
        return UNSET;
      }

      if (_food->Consume(*_position) <= 0) {
        _dined = true;
        return Flip(0.5) ? IDLE : WALKING;
      }

      return UNSET;
    }

    _walkingAnimationCounter += 1;
    if (IsFacingRight()) {
      _position->x += 2;
    } else {
      _position->x -= 2;
    }

    return UNSET;
  }

  virtual void Draw() {
    bool right = IsFacingRight();
    Texture2D tama;

    if (IsEating()) {
      if (IsFacingRight()) {
        tama = _texturesRight[_eatingAnimationCounter % _texturesRight.size()];
      } else {
        tama = _texturesLeft[_eatingAnimationCounter % _texturesLeft.size()];
      }
    } else {
      if (IsFacingRight()) {
        tama = _walkingRight[_walkingAnimationCounter % _walkingRight.size()];
      } else {
        tama = _walakingLeft[_walkingAnimationCounter % _walakingLeft.size()];
      }
    }

    int xoffset = 0;

    DrawTama(tama, *_position);

    _food->Draw();
  }

private:
  bool IsFacingRight() { return _position->x - _food->Position.x < 0; }

  bool IsEating() {
    int distance;

    if (IsFacingRight()) {
      distance =
          _food->Position.x - (_position->x + (_walkingRight[0].width * 2));
    } else {
      distance = _position->x - _food->Position.x;
    }

    return distance <= 8;
  }

  std::string _assetsDirectory;
  std::vector<Texture2D> _walkingRight;
  std::vector<Texture2D> _walakingLeft;

  Vector2 *_position;
  float _velocity;

  Food *_food;
  int _walkingAnimationCounter;
  int _eatingAnimationCounter;
  bool _dined;
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

  bool TryExitState(State next) {
    return true;
  }

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
    _headpatCeremonyComplete = false;
  }  

  bool TryExitState(State next) {
    return _headpatCeremonyComplete;
  }

  State Update(long frameCounter) {
    if (frameCounter % SPEED == 0) {
      _counter += 1;
    } else {
      return UNSET;
    }

    if (_counter >= 32) {
      _headpatCeremonyComplete = true;
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
  bool _headpatCeremonyComplete;
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

  bool TryExitState(State next) {
    return true;
  }

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

class Game : public TamaState {
public:
  Game() {}

  Game(std::string assetsDirectory) {
    state = GAME;

    std::string path = assetsDirectory + "idle/idle.png";
    Image img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _texturesRight.push_back(LoadTextureFromImage(partImage));
    }

    path = assetsDirectory + "game/friend.png";
    img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      ImageFlipHorizontal(&partImage);
      _friend.push_back(LoadTextureFromImage(partImage));
    }

    path = assetsDirectory + "game/count.png";
    img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _count.push_back(LoadTextureFromImage(partImage));
    }

    path = assetsDirectory + "game/rps.png";
    img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _rps.push_back(LoadTextureFromImage(partImage));
    }

    path = assetsDirectory + "game/victory_heart.png";
    img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 8) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 8, .height = 8};

      Image partImage = ImageFromImage(img, frame);
      _victoryHeart.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _complete = false;
    _idleCounter = 0;
    _countAnimationCounter = 0;
    _throwAnimationCounter = 0;
    _position = position;
    _tamaChoice = RNG(0,3);
    _friendChoice = RNG(0,3);
  }

  bool TryExitState(State next) {
    if(!_complete){
      return false;
    } 
    _countAnimationCounter = 0;
    _throwAnimationCounter = 0;
    return true;
  }

  State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _idleCounter += 1;

    if (_throwAnimationCounter > 12) {
      _complete = true;
      return Flip(0.5) ? IDLE : WALKING;
    }

    if (IsCounting()) {
      _countAnimationCounter += 1;
    } else {
      _throwAnimationCounter += 1;
    }

    return UNSET;
  }

  virtual void Draw() {
    Texture2D tama;
    Vector2 tamaPositionDuringGame = Vector2 {
      .x = TamaConstant::SCREEN_X + 10.0,
      .y = TamaConstant::SCREEN_Y + 10.0
    };

    tama = _texturesRight[_idleCounter % _texturesRight.size()];
    DrawTama(tama, tamaPositionDuringGame);    

    Texture2D playmate = _friend[_idleCounter % _friend.size()];
    Vector2 friendPostion = tamaPositionDuringGame;
    friendPostion.x += 72.0;
    DrawTama(playmate, friendPostion);


    if (IsCounting()){      
      Texture2D count = _count[GetCountFrame() % _count.size()];     
      DrawTama(count, tamaPositionDuringGame);
      DrawTama(count, friendPostion);
    } else {    
      const float throwMargin = 6.0;
      Vector2 throwPosition = tamaPositionDuringGame;
      throwPosition.y -= throwMargin;
      Texture2D tamaChoice = _rps[_tamaChoice % _rps.size()];     
      DrawTama(tamaChoice, throwPosition);

      Outcome outcome = GetOutcome();
      if (_throwAnimationCounter > 5 && outcome == Outcome::WIN) {
        Texture2D heart = _victoryHeart[0];
        Vector2 heartPosition = tamaPositionDuringGame;
        heartPosition.x += 30;
        heartPosition.y -= _throwAnimationCounter - 6;
        DrawTama(heart, heartPosition);        
      }

      throwPosition = friendPostion;
      throwPosition.y -= throwMargin;
      Texture2D friendChoice = _rps[_friendChoice % _rps.size()];
      DrawTama(friendChoice, throwPosition);

      if (_throwAnimationCounter > 5 && outcome == Outcome::LOSE) {
        Texture2D heart = _victoryHeart[0];
        Vector2 heartPosition = friendPostion;
        heartPosition.y -= _throwAnimationCounter - 6;
        DrawTama(heart, heartPosition);        
      }
    }
  }

private:
  enum class Outcome {
      DRAW = 0,
      WIN  = 1,
      LOSE = 2
  };

  bool IsCounting() {
    return _countAnimationCounter < 6;
  }

  int GetCountFrame() {
    // I wanted to increase the number of dots
    // every 2 game ticks
    return _countAnimationCounter / 2;
  }

  Outcome GetOutcome() {
    // thank you jan!
    return static_cast<Outcome>((3 + _tamaChoice - _friendChoice) % 3);
  }

  std::vector<Texture2D> _walkingRight;
  std::vector<Texture2D> _walakingLeft;

  std::vector<Texture2D> _friend;
  std::vector<Texture2D> _count;
  std::vector<Texture2D> _rps;

  std::vector<Texture2D> _victoryHeart;

  Vector2 *_position;
  float _velocity;

  int _idleCounter;
  int _countAnimationCounter;
  int _throwAnimationCounter;
  int _tamaChoice;
  int _friendChoice;
  bool _complete;
  
};