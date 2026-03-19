#include "raylib.h"

void DrawTama(Texture2D frame, Vector2 position, bool isRight = true) {
  float width = float(frame.width * (isRight ? 1 : -1));
  Rectangle source =
      Rectangle{.x = 0, .y = 0, .width = width, .height = float(frame.height)};

  Rectangle destination = Rectangle{
      .x = position.x,
      .y = position.y,
      .width = float(frame.width) * 2,
      .height = float(frame.height) * 2};

  DrawTexturePro(frame, source, destination, {0, 0}, 2.0, WHITE);
}
