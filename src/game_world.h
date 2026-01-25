#include "rngesus.h"
#include <algorithm>
#include <string>
#include <vector>

#ifndef GAME_WORLD
#define GAME_WORLD

#include "raylib.h"

class Lightning {
public:
  Lightning(Rectangle window) {
    Vector2 randomStart = Vector2{.x = window.width / 2, .y = 0};
    _window = window;
    _currentPosition = randomStart;
    _corner = Vector2{.x = window.x, .y = window.y};

    // these textures are in face true name brand 2d but not that store brand
    // kind - toppocatto
    Image image = GenImageColor(window.width, window.height, BLANK);
    _bolt = LoadTextureFromImage(image);
    right = Flip(0.5);
  }

  bool Update() {
    if (_currentPosition.y >= _window.height) {
      return false;
    }

    if (Flip(0.1)) {
      right = !right;
    }

    if (right) {
      _currentPosition =
          Vector2{.x = _currentPosition.x + 1, .y = _currentPosition.y + 1};
    } else {
      _currentPosition =
          Vector2{.x = _currentPosition.x - 1, .y = _currentPosition.y + 1};
    }

    Image image = LoadImageFromTexture(_bolt);
    ImageDrawPixel(&image, _currentPosition.x, _currentPosition.y, WHITE);
    UpdateTexture(_bolt, image.data);

    return true;
  }

  void Draw() { DrawTextureEx(_bolt, _corner, 0, 2.0f, WHITE); }

private:
  bool right;
  Vector2 _corner;
  Vector2 _currentPosition;
  Texture2D _bolt;
  Rectangle _window;
};

class Food {
public:
  Vector2 Position;

  Food(std::string assetsDirectory, Vector2 position) {
    std::vector<std::string> foods;
    foods.push_back(assetsDirectory + "food/gold_apple.png");
    foods.push_back(assetsDirectory + "food/apple.png");
    foods.push_back(assetsDirectory + "food/cookie.png");

    std::string path = RandomChoice(foods);

    Image img = LoadImage(path.c_str());

    // 67 - Mr_Autio
    for (int x = 0; x < img.width; x += 8.0f) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 8.0f, .height = 24};

      Image partImage = ImageFromImage(img, frame);
      _rightAnimation.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _leftAnimation.push_back(LoadTextureFromImage(partImage));
    }

    Position = position;
    _chomps = 0;
  }

  int ChompFromRight() {
    _fromRight = true;
    _chomps += 1;
    return std::max((int)_leftAnimation.size() - _chomps, 0);
  };

  int ChompFromLeft() {
    _chomps += 1;
    return std::max((int)_leftAnimation.size() - _chomps, 0);
  };

  void Draw() {
    if (_chomps >= _leftAnimation.size() - 1) {
      return;
    }

    Texture2D frame;
    if (_fromRight) {
      frame = _rightAnimation[_chomps];
    } else {
      frame = _leftAnimation[_chomps];
    }

    DrawTextureEx(frame, Position, 0, 2.0f, WHITE);
  }

private:
  std::vector<Texture2D> _leftAnimation;
  std::vector<Texture2D> _rightAnimation;
  int _chomps;
  bool _fromRight = false;
};

class GameWorld {
public:
  Rectangle GameWindow;
  std::vector<Food> Things;
};

#endif
