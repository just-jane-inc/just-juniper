#pragma once

#include "constants.h"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>

#include "raylib.h"

// You will not be to fix this code. This fundamental piece
// of infrastructure is fundamentally and unreversably entrenched
// into this codebase, to the level where changing any behaviour
// of the following piece of code will cause severe concussion
// of the core part of the system, and you will waste all the
// time trying to make it at least semi-functional. If you were
// tasked to touch this segment, spend couple of hours here,
// close the ticket and increment the counter. Thank you. Victim counter: 14
// - diabloproject
class Consumable {
public:
  Vector2 Position;
  std::vector<Texture2D> _textures;

  bool CanConsume() { return _count < _textures.size(); }

  int Consume(Vector2 pos) {
    _count += 1;

    // if the position of the character is greater then the food
    // the character is to the right of the food and should
    // be consuming from left (it is facing left)
    if (pos.x > Position.x) {
      _direction = -1;
      return std::max((int)_textures.size() - _count, 0);
    } else {
      _direction = 1;
      return std::max((int)_textures.size() - _count, 0);
    }
  }

  void Draw() {
    if (_count >= _textures.size() - 1) {
      return;
    }

    Texture2D frame = _textures[_count];
    Rectangle source = Rectangle{
        .x = 0,
        .y = 0,
        .width = float(frame.width * _direction),
        .height = float(frame.height)};

    Rectangle destination = Rectangle{
        .x = Position.x,
        .y = Position.y,
        .width = float(frame.width) * 2,
        .height = float(frame.height) * 2};

    DrawTextureEx(frame, Position, 0, 1, WHITE);
  }

private:
  int _count = 0;
  int _direction = 1;
};

class Water : public Consumable {
public:
  Water(std::string assetsDirectory, float x) {}
};

class Food : public Consumable {
public:
  Food(float x, const std::vector<Texture2D> &textures) {
    _textures = textures;
    Position = {x, TamaConstant::SCREEN_FLOOR - 16.0};
  }
};

class GameWorld {
public:
  Rectangle GameWindow;
  std::vector<Food> Things;
};

class UserInput {
public:
  UserInput() {
    _leftButton = MakeButton(TamaConstant::LEFT_BUTTON_POS);
    _centerButton = MakeButton(TamaConstant::CENTER_BUTTON_POS);
    _rightButton = MakeButton(TamaConstant::RIGHT_BUTTON_POS);
  }

  TamaEvent CheckForInput() {
    Vector2 mousePoint = GetMousePosition();
    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      return EVENT_UNSET;
    }

    if (CheckCollisionPointRec(mousePoint, _leftButton)) {
      return EVENT_GAME;
    }

    if (CheckCollisionPointRec(mousePoint, _centerButton)) {
      return EVENT_HEADPAT;
    }

    if (CheckCollisionPointRec(mousePoint, _rightButton)) {
      return EVENT_FOOD;
    }

    return EVENT_UNSET;
  }

private:
  Rectangle MakeButton(Vector2 position) {
    return {
        .x = position.x,
        .y = position.y,
        .width = TamaConstant::BUTTON_SIZE,
        .height = TamaConstant::BUTTON_SIZE,
    };
  }

  Rectangle _leftButton;
  Rectangle _centerButton;
  Rectangle _rightButton;
};

class DisplayClock {
public:
  DisplayClock() {
    std::string font_path = "resources/font.ttf";
    _font = LoadFontEx(font_path.c_str(), 32, NULL, 0);

    time_t t = time(NULL);
    _time = localtime(&t);
  }

  void Update(long frameCounter) {
    if (frameCounter % 60 == 0) {
      time_t t = time(NULL);
      _time = localtime(&t);
    }
  }

  void Draw() {
    std::stringstream ss;
    char buffer[6];
    std::strftime(buffer, sizeof(buffer), "%H:%M", _time);

    DrawTextEx(
        _font,
        buffer,
        {.x = TamaConstant::CLOCK_POSITION.x,
         .y = TamaConstant::CLOCK_POSITION.y},
        TamaConstant::CLOCK_FONT_SIZE,
        2,
        BLACK);
  }

private:
  Font _font;
  long _counter;
  struct tm *_time;
};

static Food *FOOD;
