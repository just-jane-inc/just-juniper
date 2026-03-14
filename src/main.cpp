#include "raylib.h"

#include "constants.h"
#include "tama.h"

#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>

#include <cstdlib>
#include <cstring>
#include <format>
#include <raylib.h>
#include <string>

#include <emscripten/websocket.h>
#include <nlohmann/json.hpp>

EMSCRIPTEN_WEBSOCKET_T socket;

#define TRANSPARENT CLITERAL(Color){0x00, 0x00, 0x00, 0x00}

struct Message {
  int id;
};

void from_json(const nlohmann::json &j, Message &d) {
  // for the love of god stop havaing so many opinions format thing
  j.at("id").get_to(d.id);
}

bool on_ws_open(
    int eventType,
    const EmscriptenWebSocketOpenEvent *event,
    void *userData) {
  return true;
}

bool on_ws_msg(
    int eventType,
    const EmscriptenWebSocketMessageEvent *event,
    void *userData) {
  std::string jsonStr((char *)event->data, event->numBytes);
  auto j = nlohmann::json::parse(jsonStr);
  Message msg = j.get<Message>();
  TraceLog(LOG_INFO, std::format("hello juniper: {}", msg.id).c_str());
  return true;
}

bool on_ws_closed(
    int eventType,
    const EmscriptenWebSocketCloseEvent *event,
    void *userData) {
  return true;
}

void stdin_listener(std::queue<TamaEvent> *eventQueue) {
  // map of strings expected over stdin to the enum values associated to those
  // strings
  std::unordered_map<std::string, TamaEvent> eventMap = {
      {"headpat", EVENT_HEADPAT},
      {"hydrate", EVENT_HYDRATE},
      {"food", EVENT_FOOD},
      {"game", EVENT_GAME},
  };

  // we are just going to block on stdin in this function which is called in a
  // bg thread.
  while (true) {
    std::string line;
    std::getline(std::cin, line);

    if (eventMap.find(line) == eventMap.end()) {
      // whatever came over stdin as not valid
      continue;
    }

    // push the mapped event onto the queue so that it can be accessed
    // from tama
    eventQueue->push(eventMap[line]);
  }
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

  Tama tama = Tama(gameZone, "juniper");
  int animationStep = 0;

  std::string path = "resources/juniper/egg.png";
  Image egg = LoadImage(path.c_str());
  Texture2D bg = LoadTextureFromImage(egg);

  int count = 0;

  int blah;
  // std::thread stdin_event_thread(stdin_listener, &tama.eventQueue);
  // std::this_thread::sleep_for(std::chrono::seconds(1));
  UserInput uinput = UserInput();

  DisplayClock clock;

  while (!WindowShouldClose()) {
    count += 1;
    tama.Update();
    clock.Update(count);

    // check if the application has received
    // input from a button press, add events
    // to the event queue.
    TamaEvent e = uinput.CheckForInput();
    if (e != EVENT_UNSET) {
      tama.eventQueue.push(e);
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
  std::string wsURL = "";
  wsURL = "ws://localhost:42075/ws";

  SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_ALWAYS_RUN);

  InitWindow(
      TamaConstant::WINDOW_WIDTH,
      TamaConstant::WINDOW_HEIGHT,
      "just-tamagotchi");

  SetTargetFPS(60);

  TraceLog(
      LOG_INFO,
      std::format("attempting to connect to: {}", wsURL).c_str());

  EmscriptenWebSocketCreateAttributes attrs = {wsURL.c_str(), NULL, EM_TRUE};

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
