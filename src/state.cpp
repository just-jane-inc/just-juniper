#include "raylib.h"

void DrawTama(Texture2D texture, Vector2 position) {
  DrawTextureEx(texture, position, 0.0, 2.0, WHITE);
}
