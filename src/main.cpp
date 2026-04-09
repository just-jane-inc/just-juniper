#include "raylib.h"

#include "constants.h"
#include "tama.h"

#include <cstddef>
#include <cstdlib>

#include <queue>
#include <string>
#include <unordered_map>

#include <cstdlib>
#include <cstring>
#include <format>
#include <raylib.h>
#include <string>

#include <emscripten/fetch.h>
#include <emscripten/websocket.h>
#include <nlohmann/json.hpp>

EMSCRIPTEN_WEBSOCKET_T socket;

#define TRANSPARENT CLITERAL(Color){0x00, 0x00, 0x00, 0x00}

std::queue<TamaEvent> *tamaEventQueue = new std::queue<TamaEvent>();

// A struct used to track the loading of an image asset
// This is NOT a comment! - ty999999
struct Asset {
  std::string url;
  Image img = {0};
  bool is_loaded;
  bool load_failed;
};

// we have global references to these assets
// to use for our textures. this is done so that we
// can inspect them in the game loop while waiting for
// our resources to be fetched.
static Asset _egg;
static Asset _tama_sprite;
static Asset _food;

const std::unordered_map<std::string, TamaEvent> TAMA_EVENT_MAP = {
    {"headpat", EVENT_HEADPAT},
    {"hydrate", EVENT_HYDRATE},
    {"food", EVENT_FOOD},
    {"game", EVENT_GAME},
};

struct Message {
  int id;
  std::string eventType;
};

void reload_web_page() { emscripten_run_script("location.reload();"); }

void from_json(const nlohmann::json &j, Message &d) {
  // for the love of god stop havaing so many opinions format thing
  j.at("id").get_to(d.id);
  j.at("event_type").get_to(d.eventType);
}

// Copyright (c) 2026 - Red_Epicness
// note that the websocket callbacks use different event types in their
// signatures...
bool on_ws_open(
    int eventType,
    const EmscriptenWebSocketOpenEvent *event,
    void *userData) {
  return true;
}

bool on_ws_msg(
    int eventType,
    const EmscriptenWebSocketMessageEvent *event,
    void *gameState) {

  std::string json_str;
  json_str = std::string((char *)event->data, event->numBytes);
  TraceLog(LOG_INFO, std::format("received: {}", json_str).c_str());
  try {
    Message msg = nlohmann::json::parse(json_str).get<Message>();

    if (msg.eventType.starts_with("PING")) {
      emscripten_websocket_send_utf8_text(socket, "PONG");
      return true;
    }

    if (TAMA_EVENT_MAP.contains(msg.eventType)) {
      tamaEventQueue->push(TAMA_EVENT_MAP.at(msg.eventType));
    } else {
      std::string log_msg =
          std::format("received unknown event type: {}", msg.eventType);

      TraceLog(LOG_ERROR, log_msg.c_str());
    }

  } catch (...) {
    TraceLog(LOG_ERROR, "there was like an exception or something, idk");
  }

  return true;
}

// Thank you, swerocker1993 - Captain_Onosa
bool on_ws_closed(
    int eventType,
    const EmscriptenWebSocketCloseEvent *event,
    void *userData) {

  // Thank you, swerocker1993 - ty999999
  reload_web_page();
  return true;
}

/* TODO: do this thing that Jan suggested
 * make it so that if we fail at any point here we load
 * a default image somewhere, we actually never fail so sei we all.
 * until this is done, ensure you are in a safe space and cry about it.
 */
void OnDownloadSuccess(emscripten_fetch_t *fetch) {
  auto *dst = static_cast<Asset *>(fetch->userData);

  TraceLog(
      LOG_INFO,
      TextFormat(
          "download ok: %s (%d bytes)",
          fetch->url,
          (int)fetch->numBytes));

  // The data resides in the bytes of the PNG format,
  // which is assumed to work justja211Noted - Meisaka
  dst->img = LoadImageFromMemory(
      ".png",
      reinterpret_cast<const unsigned char *>(fetch->data),
      (int)fetch->numBytes);

  if (IsImageValid(dst->img)) {
    dst->is_loaded = true;
    dst->load_failed = false;
  } else {
    dst->is_loaded = false;
    dst->load_failed = true;
    TraceLog(
        LOG_ERROR,
        TextFormat("LoadImageFromMemory failed for %s", fetch->url));
  }

  emscripten_fetch_close(fetch);
}

// to be debugged in the future - intrikist
void OnDownloadFailed(emscripten_fetch_t *fetch) {
  auto *dst = static_cast<Asset *>(fetch->userData);
  dst->load_failed = true;

  TraceLog(
      LOG_ERROR,
      TextFormat("download failed: %s status=%d", fetch->url, fetch->status));
  emscripten_fetch_close(fetch);
}

// break this code again in the future - TheChowMein
void StartDownload(Asset &img, const char *url) {
  img = {};
  img.url = url;

  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  std::strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
  attr.userData = &img;
  attr.onsuccess = OnDownloadSuccess;
  attr.onerror = OnDownloadFailed;

  emscripten_fetch(&attr, url);
}

// just comment - asahi_KIRIN
void init_game() {
  StartDownload(
      _tama_sprite,
      "https://bahms.org/assets/juniper/juniper_210326.png");

  StartDownload(_egg, "https://bahms.org/assets/juniper/egg.png");
  StartDownload(_food, "https://bahms.org/assets/juniper/food_sprites.png");
}

// Let me tell you a story. A long time ago, a lonely girl
// started to explore the world of software engineering.
// She wasn’t brave or powerful, but she had something
// much more important on her side: patience and persistence.
// The girl enjoyed every curly brace and every comma,
// and her speed of information consumption was immeasurable.
//
// As nights progressed, as the days blurred past, she was
// becoming a master of the trade. But unlike the stories from
// the children's books, this story has no happy ending. One day,
// the girl stumbled across something forbidden. Everyone she
// knew was discouraging her, saying that this is the path
// of no return, and that she will ruin herself. But she did not listen.
//
// She started writing C++.
//
// Slowly, bit by bit, she was consumed by dark poison of this evil,
// until her brain became one of clang++. It was painful, but her desire
// for new knowledge was stronger than the pain. Staring into the void of STL,
// she spoke her last words. This prophecy is documented in the function below,
// treat them with the reverence she deserves.
// - diabloproject
void game_loop() {
  // the borders of windows are transparent for me so that it looks nicer
  // on stream. The always run flag is used to ensure that the game loop
  // executes while the application is minimized, this window is almost always
  // minimized.
  Rectangle gameZone = Rectangle{
      .x = TamaConstant::SCREEN_X,
      .y = TamaConstant::SCREEN_Y,
      .width = TamaConstant::SCREEN_WIDTH,
      .height = TamaConstant::SCREEN_HEIGHT};

  Tama *tama = nullptr;
  Texture2D bg;

  bool bg_done = false;

  int animationStep = 0;
  int frame_counter = 0;

  UserInput uinput = UserInput();

  // beyond code - TheChowMein
  DisplayClock clock;

  while (!WindowShouldClose()) {
    frame_counter++;

    // Finish asset setup once downloads are ready
    if (!bg_done && _egg.is_loaded) {
      bg = LoadTextureFromImage(_egg.img);
      bg_done = true;
      UnloadImage(_egg.img);
    }

    if (tama == nullptr && _tama_sprite.is_loaded && _food.is_loaded) {
      tama = new Tama(gameZone, _tama_sprite.img, _food.img);
      tama->eventQueue = tamaEventQueue;
      UnloadImage(_tama_sprite.img);
      UnloadImage(_food.img);
    }

    // if we have not loaded our assets do not continue
    // everything below requires a fully initialized game
    if (tama == nullptr || !bg_done) {
      continue;
    }

    tama->Update();
    clock.Update(frame_counter);

    // check if the application has received
    // input from a button press, add events
    // to the event queue.
    TamaEvent e = uinput.CheckForInput();
    if (e != EVENT_UNSET) {
      tama->eventQueue->push(e);
    }

    BeginDrawing();
    ClearBackground(TRANSPARENT);
    DrawTextureEx(bg, {0.0, 0.0}, 0.0, 1.0, WHITE);

    tama->Draw();
    clock.Draw();
    EndDrawing();
  }

  CloseWindow();
}

int main() {
  std::string ws_url = "wss://juniper.bahms.org/ws";

  SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_ALWAYS_RUN);

  InitWindow(
      TamaConstant::WINDOW_WIDTH,
      TamaConstant::WINDOW_HEIGHT,
      "just-tamagotchi");

  SetTargetFPS(60);

  init_game();

  TraceLog(
      LOG_INFO,
      std::format("attempting to connect to: {}", ws_url).c_str());

  EmscriptenWebSocketCreateAttributes attrs = {ws_url.c_str(), NULL, EM_TRUE};

  socket = emscripten_websocket_new(&attrs);

  // Set callbacks
  emscripten_websocket_set_onopen_callback(socket, (void *)0x420, on_ws_open);
  emscripten_websocket_set_onclose_callback(socket, (void *)0x69, on_ws_closed);
  emscripten_websocket_set_onmessage_callback(socket, (void *)0x67, on_ws_msg);

  // Set the main loop function for Emscripten
  emscripten_set_main_loop(game_loop, 0, 1);

  // This part is unreachable in the WASM build due to the main loop setup
  CloseWindow();
  return 0;
}
