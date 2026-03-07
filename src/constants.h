#pragma once
#include "raylib.h"

enum TamaEvent { EVENT_UNSET, EVENT_HEADPAT, EVENT_HYDRATE, EVENT_FOOD, EVENT_GAME };

namespace TamaConstant {

constexpr float WINDOW_HEIGHT = 254;
constexpr float WINDOW_WIDTH = 200;

// screen
constexpr float SCREEN_WIDTH = 144;
constexpr float SCREEN_HEIGHT = 81;
constexpr float SCREEN_X = 28;
constexpr float SCREEN_Y = 74;

// where tama should consider the floor
constexpr float SCREEN_FLOOR = 155;

// buttons
constexpr float BUTTON_SIZE = 20;
constexpr Vector2 LEFT_BUTTON_POS = {.x = 48, .y = 185};
constexpr Vector2 CENTER_BUTTON_POS = {.x = 90, .y = 196};
constexpr Vector2 RIGHT_BUTTON_POS = {.x = 132, .y = 185};

constexpr Vector2 CLOCK_POSITION = {.x = 122, .y = 55};
constexpr int CLOCK_FONT_SIZE = 20;

// tama size
constexpr Vector2 TAMA_SIZE = {.x = 24, .y = 24};

} // namespace TamaConstant
