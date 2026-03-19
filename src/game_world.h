#pragma once

#include "constants.h"
#include "rngesus.h"
#include <algorithm>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>

#include "raylib.h"

class Consumable {
public:
  Vector2 Position;
  std::vector<Texture2D> _leftAnimation;
  std::vector<Texture2D> _rightAnimation;

  bool CanConsume() { return _count < _leftAnimation.size(); }

  int Consume(Vector2 pos) {
    _count += 1;

    // if the position of the character is greater then the food
    // the character is to the right of the food and should
    // be consuming from left (it is facing left)
    if (pos.x > Position.x) {
      _fromRight = false;
      return std::max((int)_leftAnimation.size() - _count, 0);
    } else {
      _fromRight = true;
      return std::max((int)_rightAnimation.size() - _count, 0);
    }
  }

  void Draw() {
    if (_count >= _leftAnimation.size() - 1) {
      return;
    }

    Texture2D frame;
    if (_fromRight) {
      frame = _rightAnimation[_count];
    } else {
      frame = _leftAnimation[_count];
    }

    DrawTextureEx(frame, Position, 0, 1.0f, WHITE);
  }

private:
  int _count = 0;
  bool _fromRight = false;
};

class Water : public Consumable {
public:
  Water(std::string assetsDirectory, float x) {
    Position = {.x = x, .y = 14};
    std::string path = assetsDirectory + "water-bowl/water-bowl.png";
    Image img = LoadImage(path.c_str());

    float frame_width = (float)img.width / 6;

    for (int x = 0; x < img.width; x += frame_width) {
      Rectangle frame = Rectangle{
          .x = (float)x,
          .y = 0,
          .width = (float)frame_width,
          .height = (float)img.height};

      Image partImage = ImageFromImage(img, frame);
      _rightAnimation.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _leftAnimation.push_back(LoadTextureFromImage(partImage));
    }
  }
};

class Food : public Consumable {
public:
  Food(std::string assetsDirectory, float x) {
    std::vector<std::string> foods;
    foods.push_back(assetsDirectory + "food/apple.png");
    foods.push_back(assetsDirectory + "food/blueberry.png");
    foods.push_back(assetsDirectory + "food/cookie.png");
    foods.push_back(assetsDirectory + "food/carrot.png");

    std::string path = RandomChoice(foods);

    Image img = LoadImage(path.c_str());

    // we require that food have 8 animation frames that represent
    // being eaten from the left.
    float frame_width = (float)img.width / 8;

    // 67 - Mr_Autio
    for (int x = 0; x < img.width; x += frame_width) {
      Rectangle frame = Rectangle{
          .x = (float)x,
          .y = 0,
          .width = (float)frame_width,
          .height = (float)img.height};

      Image partImage = ImageFromImage(img, frame);
      _rightAnimation.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _leftAnimation.push_back(LoadTextureFromImage(partImage));
    }

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
    std::string font_path = std::string(ASSETS_PATH) + "font.ttf";
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
