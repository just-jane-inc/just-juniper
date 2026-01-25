#include "raylib.h"

#include "cookie.h"
#include "tama.h"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <sw/redis++/redis++.h>
#include <thread>

using namespace sw::redis;

#define TRANSPARENT CLITERAL(Color){0x00, 0x00, 0x00, 0x00}

#define WINDOW_HEIGHT 250.0
#define WINDOW_WIDTH 180.0

void headpat_listener(int *headpat_counter) {
  try {
    char *uri = std::getenv("REDIS_CONNECTION_STRING");
    sw::redis::Redis redis(uri);
    auto sub = redis.subscriber();

    sub.on_message([](std::string channel, std::string msg) {});

    sub.subscribe("juniper/redeems/headpat");

    bool wtf = false;
    while (true) {
      try {
        sub.consume();
        if (!wtf) {
          wtf = true;
          continue;
        }

        *headpat_counter += 1;

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
  Color screen = {200, 200, 200, 128};
  SetConfigFlags(FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_ALWAYS_RUN);
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "just-tamagotchi");
  SetTargetFPS(60);

  Tama tama = Tama(WINDOW_WIDTH, WINDOW_HEIGHT, "juniper");
  Image cookieImage = LoadImage("/home/jane/just-stream/just-ray-bahms/"
                                "just-tamagotchi/assets/cookie.png");
  int animationStep = 0;

  Image egg = LoadImage(
      "/home/jane/just-stream/just-ray-bahms/just-tamagotchi/assets/egg.png");
  Texture2D bg = LoadTextureFromImage(egg);

  Rectangle gameZone = Rectangle{.x = 28, .y = 64, .width = 62, .height = 60};
  Rectangle gameZone2 =
      Rectangle{.x = 27, .y = 64, .width = 124, .height = 120};
  Lightning bolt = Lightning(gameZone);

  int count = 0;

  int blah;
  std::thread sub_thread(headpat_listener, &tama.headpat);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  while (!WindowShouldClose()) {
    count += 1;
    tama.Update();

    BeginDrawing();
    ClearBackground(TRANSPARENT);

    DrawTexture(bg, 0, 0, WHITE);
    if (bolt.Update()) {
      bolt.Draw();
      DrawRectangleRec(gameZone2, screen);
    }

    tama.Draw();
    EndDrawing();
  }

  CloseWindow();

  if (sub_thread.joinable()) {
    sub_thread.join();
  }

  return 0;
}
