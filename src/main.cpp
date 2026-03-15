#include "raylib.h"

#include "constants.h"
#include "tama.h"

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

  if (event->isText) {
    json_str = std::string((char *)event->data, event->numBytes);
  }

  if (json_str == "PING") {
    emscripten_websocket_send_utf8_text(socket, "PONG");
  }

  try {
    Message msg = nlohmann::json::parse(json_str).get<Message>();
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

bool on_ws_closed(
    int eventType,
    const EmscriptenWebSocketCloseEvent *event,
    void *userData) {
  reload_web_page();
  return true;
}

/* TODO: do this thing that Jan suggested
 * make it so that if we fail at any point here we load
 * a default image somewhere, we actually never fail so sei we all.
 * until this is done, ensure you are in a safe space and cry about it.
 */
void OnDownloadSuccess(emscripten_fetch_t *fetch) {
  TraceLog(
      LOG_INFO,
      std::format("image download success: {}", fetch->url).c_str());

  // this is perfectly safe.
  Image *img = static_cast<Image *>(fetch->userData);

  // The data resides in the bytes of the PNG format,
  // which is assumed to work justja211Noted - Meisaka
  *img = LoadImageFromMemory(
      ".png",
      reinterpret_cast<const unsigned char *>(fetch->data),
      fetch->numBytes);

  TraceLog(
      LOG_INFO,
      std::format("the image bytes are here: {}", fetch->numBytes).c_str());

  emscripten_fetch_close(fetch);
}

void OnDownloadFailed(emscripten_fetch_t *fetch) {
  TraceLog(LOG_ERROR, "panic...");
  emscripten_fetch_close(fetch);
}

/*
 *
 int main() {
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  strcpy(attr.requestMethod, "GET");
  attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY |
EMSCRIPTEN_FETCH_SYNCHRONOUS; emscripten_fetch_t *fetch =
emscripten_fetch(&attr, "file.dat"); // Blocks here until the operation is
complete. if (fetch->status == 200) { printf("Finished downloading %llu bytes
from URL %s.\n", fetch->numBytes, fetch->url);
    // The data is now available at fetch->data[0] through
fetch->data[fetch->numBytes-1]; } else { printf("Downloading %s failed, HTTP
failure status code: %d.\n", fetch->url, fetch->status);
  }
  emscripten_fetch_close(fetch);
}
 */
Image Download(std::string url) {
  // TODO: what the actual frick is this?
  emscripten_fetch_attr_t attr;
  emscripten_fetch_attr_init(&attr);
  std::strcpy(attr.requestMethod, "GET");

  attr.attributes =
      EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
  //  attr.userData = &kill_me;
  //  attr.onsuccess = OnDownloadSuccess;
  //  attr.onerror = OnDownloadFailed;

  emscripten_fetch_t *fetch = emscripten_fetch(&attr, url.c_str());

  if (fetch->status == 200) {
    TraceLog(LOG_INFO, "yippie");

    Image kill_me = LoadImageFromMemory(
        ".png",
        reinterpret_cast<const unsigned char *>(fetch->data),
        fetch->numBytes);

    TraceLog(
        LOG_INFO,
        std::format("the image bytes: {}", fetch->numBytes).c_str());
    TraceLog(LOG_INFO, std::format("the image: {}", kill_me.height).c_str());

    emscripten_fetch_close(fetch);
    return kill_me;
  } else {
    TraceLog(LOG_ERROR, std::format("frick {}", fetch->status).c_str());
  }

  return Image{};
}

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

  Image juniper_sprite_sheet =
      Download("https://bahms.org/assets/juniper/hedgehog.png");
  Tama tama = Tama(gameZone, juniper_sprite_sheet);
  tama.eventQueue = tamaEventQueue;
  int animationStep = 0;

  std::string egg_url = "https://bahms.org/assets/juniper/egg.png";
  Image egg = Download(egg_url);
  Texture2D bg = LoadTextureFromImage(egg);

  int frame_counter = 0;

  UserInput uinput = UserInput();

  DisplayClock clock;

  while (!WindowShouldClose()) {
    frame_counter += 1;
    tama.Update();
    clock.Update(frame_counter);

    // check if the application has received
    // input from a button press, add events
    // to the event queue.
    TamaEvent e = uinput.CheckForInput();
    if (e != EVENT_UNSET) {
      tama.eventQueue->push(e);
    }

    BeginDrawing();
    ClearBackground(TRANSPARENT);
    DrawTextureEx(bg, {0.0, 0.0}, 0.0, 1.0, WHITE);

    tama.Draw();
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
