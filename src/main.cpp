#include "raylib.h"

#include "cookie.h"
#include "tama.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <string>
#include <sw/redis++/redis++.h>
#include <thread>
#include <unordered_map>

using namespace sw::redis;

std::unordered_map<std::string, TamaEvent> EVENT_MAP = {
    {"juniper/redeems/headpat", EVENT_HEADPAT},
    {"juniper/redeems/hydrate", EVENT_HYDRATE},
};

#define TRANSPARENT CLITERAL(Color){0x00, 0x00, 0x00, 0x00}

#define WINDOW_HEIGHT 384.0
#define WINDOW_WIDTH 216.0

void headpat_listener(std::queue<TamaEvent> eventQueue) {
  try {
    char *uri = std::getenv("REDIS_CONNECTION_STRING");
    sw::redis::Redis redis(uri);
    auto sub = redis.subscriber();

    sub.on_message([&eventQueue](std::string channel, std::string msg) {
      eventQueue.push(EVENT_MAP[channel]);
    });

    sub.subscribe("juniper/redeems/headpat");
    sub.subscribe("juniper/redeems/hydrate");

    bool wtf = false;
    while (true) {
      try {
        sub.consume();
      } catch (const Error &err) {
        std::cerr << "Error in subscriber consume loop: " << err.what()
                  << std::endl;
        // Handle exceptions, possibly with a reconnection strategy
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    }
  } catch (const Error &err) {
    std::cerr << "Redis connection error in subscriber thread: " << err.what()
              << std::endl;
  }
}

int main() {
  Color screen = {0xad, 0xe0, 0xcf, 0xff};
  SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_ALWAYS_RUN);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "just-tamagotchi");
  SetTargetFPS(60);

  Rectangle gameZone =
      Rectangle{.x = 48, .y = 121, .width = 120, .height = 142};
  Tama tama = Tama(gameZone, "juniper");
  Image cookieImage = LoadImage("/home/jane/just-stream/just-ray-bahms/"
                                "just-juniper/assets/cookie.png");
  int animationStep = 0;

  Image egg = LoadImage(
      "/home/jane/just-stream/just-ray-bahms/just-juniper/assets/egg.png");

  Image bottleImg =
      LoadImage("/home/jane/just-stream/just-ray-bahms/just-juniper/"
                "assets/water-bottle.png");
  Texture2D bg = LoadTextureFromImage(egg);
  Texture2D bottle = LoadTextureFromImage(bottleImg);

  Lightning bolt = Lightning(gameZone);

  int count = 0;

  int blah;
  std::thread sub_thread(headpat_listener, tama.eventQueue);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  UserInput uinput = UserInput();

  while (!WindowShouldClose()) {
    count += 1;
    tama.Update();
    if (uinput.CheckForHeadpat()) {
      tama.eventQueue.push(EVENT_HEADPAT);
    }

    BeginDrawing();
    ClearBackground(TRANSPARENT);

    DrawTexture(bg, 0, 0, WHITE);
    DrawTexture(bottle, gameZone.x + gameZone.width - bottle.width, gameZone.y,
                WHITE);

    tama.Draw();
    EndDrawing();
  }

  CloseWindow();

  if (sub_thread.joinable()) {
    sub_thread.join();
  }

  return 0;
}
