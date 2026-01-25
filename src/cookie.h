#include "raylib.h"

#include <vector>

#ifndef COOKIE
#define COOKIE

class Cookie {
public:
  Vector2 position;

  Cookie(int height, int width, Image cookie) {
    for (int x = 0; x < cookie.width; x += 8) {
      Rectangle frame =
          Rectangle{.x = (float)x, .y = 0, .width = 8, .height = 8};

      Image partImage = ImageFromImage(cookie, frame);
      _right.push_back(LoadTextureFromImage(partImage));

      ImageFlipHorizontal(&partImage);
      _left.push_back(LoadTextureFromImage(partImage));
    }

    position = Vector2{.x = (float)width - 20, .y = (float)height / 2 + 30};
  }

  bool TakeBite(bool fromRight) {
    _idx += 1;
    return _idx < _right.size();
  }

  Texture2D GetFrame(bool isRight) {
    if (isRight) {
      return _right[_idx];
    }

    return _left[_idx];
  }

private:
  std::vector<Texture2D> _right;
  std::vector<Texture2D> _left;
  int _idx;
};

#endif
