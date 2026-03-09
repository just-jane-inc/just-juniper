#pragma once
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "constants.h"
#include "raylib.h"

#include "game_world.h"
#include "rngesus.h"

#define FLOOR_Y 24

const int SPEED = 16;
enum State {
  UNSET,
  IDLE,
  WALKING,
  HUNTING,
  EATING,
  HEADPAT,
  HYDRATE,
  JANKEN,
  ENTER_SLEEPING,
  SLEEPING
};

void DrawTama(Texture2D texture, Vector2 position, bool isRight = true);

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

  virtual void Draw() {
    Texture2D frame = _textures[_animationFrame];

    Rectangle source = Rectangle{
        .x = 0,
        .y = 0,
        .width = float(frame.width * _direction),
        .height = float(frame.height)};

    Rectangle destination = Rectangle{
        .x = _drawPosition.x,
        .y = _drawPosition.y,
        .width = float(frame.width) * 2,
        .height = float(frame.height) * 2};

    DrawTexturePro(frame, source, destination, {0, 0}, 0.0, WHITE);
  }

protected:
  std::vector<Texture2D> _textures;
  int _animationFrame;
  int _direction = 1;
  Vector2 _drawPosition;
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
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _drawPosition = *position;
    _animationFrame = 0;
    _direction = Flip(0.5) ? -1 : 1;
    _stateCounter = 0;
  };

  bool TryExitState(State next) { return true; }

  State Update(long frameCounter) {
    _stateCounter += 1;

    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _animationFrame = (_animationFrame + 1) % _textures.size();
    if (Flip(0.01)) {
      _direction *= -1;
    }

    if (_stateCounter < 300 || Flip(.80)) {
      return UNSET;
    }

    return Flip(0.75) ? WALKING : ENTER_SLEEPING;
  }

private:
  // the number of frames that we have been in this state.
  int _stateCounter;
};

class Walking : public TamaState {
public:
  Walking() { _animationFrame = 0; }

  Walking(std::string assetsDirectory) {
    state = WALKING;

    std::string path = assetsDirectory + "walking/walking.png";
    Image img = LoadImage(path.c_str());

    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _direction = Flip(0.5) ? 1 : -1;
    _stateCount = 0;

    _position = position;
    _drawPosition = *_position;
    _animationFrame = 0;
  }

  bool TryExitState(State next) { return true; }

  State Update(long frameCounter) {
    _stateCount += 1;

    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _animationFrame = (_animationFrame + 1) % _textures.size();

    if (Flip(0.01)) {
      _direction *= -1;
    }

    _position->x += _direction;

    if (IsColliding()) {
      _direction = _direction * -1;
      _position->x += 2 * _direction;
    }

    _drawPosition = *_position;

    if (_stateCount < 300 || Flip(0.80)) {
      return UNSET;
    }

    return Flip(0.75) ? IDLE : ENTER_SLEEPING;
  }

private:
  Vector2 *_position;
  int _stateCount;

  bool IsColliding() {
    return _position->x <= window.x
           || (_position->x + 48) >= window.x + window.width;
  }
};

class Hunting : public TamaState {
public:
  Hunting() {}

  Hunting(std::string assetsDirectory) {
    state = HUNTING;
    _assetsDirectory = assetsDirectory;
    std::string path = assetsDirectory + "walking/walking.png";
    Image img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _position = position;
    _direction = _position->x > (window.x + (window.width / 2.0f)) ? -1 : 1;
    _velocity = _direction * 2;
    if (_direction > 0) {
      _terminalPosition = window.x + window.width - 40;
      _terminalPosition -= TamaConstant::TAMA_SIZE.x;
    } else {
      _terminalPosition = window.x + 20;
    }
  }

  bool TryExitState(State next) {
    // after a hunting we shall only goto eating.
    return next == EATING;
  }

  State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    int distance = abs(int(_position->x) - _terminalPosition);

    if (distance < 4) {
      return EATING;
    }

    _animationFrame += 1;
    _position->x += _velocity;
    return UNSET;
  }

  virtual void Draw() {
    DrawTama(
        _textures[_animationFrame % _textures.size()],
        *_position,
        _direction > 0);
  }

private:
  Vector2 *_position;
  int _velocity;
  int _terminalPosition;
  std::string _assetsDirectory;
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
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _animationFrame = 0;
    _drawPosition = *position;

    // figure out if juniper is on left or right side of screen,
    // spawn food on opposite side, walk toward it then eat
    bool right = position->x > (window.x + (window.width / 2.0f));

    float foodx;
    if (right) {
      foodx = window.x + window.width - 24;
      _direction = 1;
    } else {
      foodx = window.x + 6;
      _direction = -1;
    }

    _food = new Food(_assetsDirectory, foodx);
  }

  bool TryExitState(State next) {
    if (_food->CanConsume()) {
      return false;
    }

    delete _food;
    return true;
  }

  State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _animationFrame += 1;
    if (_animationFrame > 0 && _food->Consume(_drawPosition) <= 0) {
      return Flip(0.5) ? IDLE : WALKING;
    }

    return UNSET;
  }

  virtual void Draw() {
    Texture2D tama;
    tama = _textures[_animationFrame % _textures.size()];
    DrawTama(tama, _drawPosition, _direction > 0);
    _food->Draw();
  }

private:
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
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _drawPosition = *position;
    _animationFrame = 0;
    _stateCounter = 0;
  };

  bool TryExitState(State next) { return true; }

  State Update(long frameCounter) {
    _stateCounter += 1;
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _animationFrame = (_animationFrame + 1) % _textures.size();
    if (_stateCounter < 128 || Flip(0.95)) {
      return UNSET;
    }

    return Flip(0.75) ? IDLE : WALKING;
  };

private:
  int _stateCounter;
};

class EnterSleeping : public TamaState {
public:
  EnterSleeping() {}

  EnterSleeping(std::string assetsDirectory) {
    state = ENTER_SLEEPING;

    std::string path = assetsDirectory + "enter_sleep/enter_sleep.png";
    Image img = LoadImage(path.c_str());

    for (int x = 0; x < img.width; x += 24.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _drawPosition = *position;
    _animationFrame = 0;
    _stateCounter = 0;
  };

  bool TryExitState(State next) { return SLEEPING == next; }

  State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _animationFrame = _animationFrame + 1;
    if (_animationFrame >= _textures.size()) {
      return SLEEPING;
    }

    _animationFrame = _animationFrame % _textures.size();
    return UNSET;
  };

private:
  int _stateCounter;
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
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  void EnterState(Vector2 *position) {
    _drawPosition = *position;
    _animationFrame = 0;
    _headpatCeremonyComplete = false;
    _stateCounter = 0;
  }

  bool TryExitState(State next) { return _headpatCeremonyComplete; }

  State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _stateCounter += 1;
    _animationFrame = (_animationFrame + 1) % _textures.size();
    if (_stateCounter >= 32) {
      _headpatCeremonyComplete = true;
      return Flip(0.20) ? WALKING : IDLE;
    }

    return UNSET;
  }

private:
  bool _headpatCeremonyComplete;
  int _stateCounter;
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
      _textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  virtual void EnterState(Vector2 *position) {
    _isHunting = true;
    _position = position;
    _velocity = (_bowl.Position.x - position->x) > 0 ? 1 : -1;
  };

  bool TryExitState(State next) { return true; }

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

private:
  TamaState *_walking;
  Vector2 *_position;
  Water _bowl;
  int _velocity;
  bool _isHunting;
};

class Janken : public TamaState {
public:
  Janken() {}

  Janken(std::string assetsDirectory) {
    state = JANKEN;

    // juniper idling
    BufferTextures(_textures, assetsDirectory + "idle/idle.png", 24, 24);
    // Could be any croco 🐊🕶️
    BufferTextures(
        _friend,
        assetsDirectory + "janken/friend.png",
        24,
        24,
        true);
    // dots
    BufferTextures(_count, assetsDirectory + "janken/count.png", 24, 24);
    // throws
    BufferTextures(_throw, assetsDirectory + "janken/rps.png", 24, 24);
    // victory heart
    BufferTextures(
        _victoryHeart,
        assetsDirectory + "janken/victory_heart.png",
        9,
        8);
  }

  void EnterState(Vector2 *position) {
    _tamaThrowChoice = RNG(0, 3);
    _complete = false;
    _idleCounter = 0;
    _countAnimationCounter = 0;
    _throwAnimationCounter = 0;
    _celebratingCounter = 0;
    _friendThrowChoice = RNG(0, 3);
    _outcome = GetOutcome();
  }

  bool TryExitState(State next) { return _complete; }

  State Update(long frameCounter) {
    if (frameCounter % SPEED != 0) {
      return UNSET;
    }

    _idleCounter += 1;

    if (DoneCelebrating() || GetOutcome() == Outcome::DRAW && DoneThrowing()) {
      _complete = true;
      return Flip(0.5) ? IDLE : WALKING;
    }

    if (IsCounting()) {
      _countAnimationCounter += 1;
    } else if (IsThrowing()) {
      _throwAnimationCounter += 1;
    } else {
      _celebratingCounter += 1;
    }

    return UNSET;
  }

  virtual void Draw() {
    Vector2 tamaPositionDuringGame = Vector2{
        .x = TamaConstant::SCREEN_X + 10.0,
        .y = TamaConstant::SCREEN_Y + 10.0};

    Texture2D tama;
    if (!DoneThrowing() || _outcome == Outcome::WIN) {
      tama = _textures[_idleCounter % _textures.size()];
    } else {
      // Stay still, you'll get 'em next time
      tama = _textures[0];
    }
    DrawTama(tama, tamaPositionDuringGame);

    Texture2D playmate;
    if (!DoneThrowing() || _outcome == Outcome::LOSE) {
      playmate = _friend[_idleCounter % _friend.size()];
    } else {
      // Stay still loser
      playmate = _friend[0];
    }
    Vector2 friendPostion = tamaPositionDuringGame;
    friendPostion.x += 72.0;
    DrawTama(playmate, friendPostion);

    if (IsCounting()) {
      Texture2D count = _count[GetCountFrame() % _count.size()];
      // Drawing dots over players
      DrawTama(count, tamaPositionDuringGame);
      DrawTama(count, friendPostion);
    } else {
      float throwMargin = 6.0;
      if (IsThrowing()) {
        // Do a little dance with the objects while they are throwing
        throwMargin += (_throwAnimationCounter % 2 ? -1.0 : 1.0);
      }
      Vector2 throwPosition = tamaPositionDuringGame;
      throwPosition.y -= throwMargin;
      Texture2D tamaChoice = _throw[_tamaThrowChoice % _throw.size()];
      DrawTama(tamaChoice, throwPosition);
      if (DoneThrowing() && _outcome == Outcome::WIN) {
        Texture2D heart = _victoryHeart[0];
        Vector2 heartPosition = tamaPositionDuringGame;
        heartPosition.x += 30; // Scooch it over the tama's head
        heartPosition.y -= _celebratingCounter - 6; // Over time, it rises
        DrawTama(heart, heartPosition);
      }

      throwPosition = friendPostion;
      throwPosition.y -= throwMargin;
      Texture2D friendChoice = _throw[_friendThrowChoice % _throw.size()];
      DrawTama(friendChoice, throwPosition);

      if (DoneThrowing() && _outcome == Outcome::LOSE) {
        Texture2D heart = _victoryHeart[0];
        Vector2 heartPosition = friendPostion;
        heartPosition.y -= _celebratingCounter - 6;
        DrawTama(heart, heartPosition);
      }
    }
  }

private:
  enum class Outcome { DRAW = 0, WIN = 1, LOSE = 2 };

  int GetCountFrame() {
    // I wanted to increase the number of dots
    // every 2 game ticks
    return _countAnimationCounter / 2;
  }

  bool LessThanThree(int i) { return i < 3 /*🦔*/; }

  bool IsCounting() { return LessThanThree(GetCountFrame()); }

  bool IsThrowing() { return _throwAnimationCounter < 6; }

  bool DoneThrowing() { return _throwAnimationCounter >= 6; }

  bool IsCelebrating() { return _celebratingCounter > 0; }

  bool DoneCelebrating() { return _celebratingCounter > 6; }

  Outcome GetOutcome() {
    // -ˋˏ ༻❁ thank you jan! ❀༺ ˎˊ-
    // Magic that resolves to the correct outcome based on the player's picks
    return static_cast<Outcome>(
        (3 + _tamaThrowChoice - _friendThrowChoice) % 3);
  }

  /* Fill the texture (passed by reference) based on the sprite sheet in the
   * path. */
  void BufferTextures(
      std::vector<Texture2D> &textures,
      std::string path,
      float width,
      float height,
      bool flip_horizonal = false) {
    Image img = LoadImage(path.c_str());
    for (int x = 0; x < img.width; x += width) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0.0f, .width = width, .height = height};

      Image partImage = ImageFromImage(img, frame);
      if (flip_horizonal) {
        ImageFlipHorizontal(&partImage);
      }
      textures.push_back(LoadTextureFromImage(partImage));
    }
  }

  std::vector<Texture2D> _friend; // 💚
  std::vector<Texture2D> _count;  // dot dot dot
  std::vector<Texture2D> _throw;  // rock paper scissors, in that order

  std::vector<Texture2D> _victoryHeart; // 💜

  int _idleCounter; // Sunshine sunshine, ladybugs awake and do a little shake
  int _countAnimationCounter; // to count to three with dots
  int _throwAnimationCounter; // to show what the players pick
  int _celebratingCounter;    // if winner, show victory animation
  int _tamaThrowChoice;
  int _friendThrowChoice;
  bool _complete;
  Outcome _outcome;
};
