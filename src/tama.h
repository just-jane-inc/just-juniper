#pragma once
#include "constants.h"
#include "state.h"
#include <iostream>
#include <iterator>
#include <queue>
#include <raylib.h>
#include <string>
#include <vector>

class Tama {
public:
  std::queue<TamaEvent> *eventQueue;
  std::string name;
  TamaState *currentState;

  /**
   * @brief Instatiate tama.
   * @param gameArea the part of the screen that is valid for the character
   * @param sprite_sheet the image to use for sprite sheet
   */
  Tama(Rectangle gameArea, Image sprite_sheet) {
    _gameArea = gameArea;
    _position = Vector2{.x = gameArea.x, .y = TamaConstant::SCREEN_FLOOR - 48};

    _headPatState = Headpat("resources/juniper/");
    _jankenState = Janken("resources/juniper/");

    std::vector<Texture2D> idle_texture;
    for (int x = 0; x < 2; x++) {
      Rectangle frame =
          Rectangle{.x = x * 24.0f, .y = 0, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(sprite_sheet, frame);
      idle_texture.push_back(LoadTextureFromImage(partImage));
    }

    std::vector<Texture2D> walking_texture;
    for (int x = 0; x < 6; x++) {
      Rectangle frame =
          Rectangle{.x = x * 24.0f, .y = 24.0f, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(sprite_sheet, frame);
      walking_texture.push_back(LoadTextureFromImage(partImage));
    }

    std::vector<Texture2D> sleeping_texture;
    for (int x = 0; x < 4; x++) {
      Rectangle frame =
          Rectangle{.x = x * 24.0f, .y = 48.0f, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(sprite_sheet, frame);
      sleeping_texture.push_back(LoadTextureFromImage(partImage));
    }

    std::vector<Texture2D> transition_texture;
    for (int x = 0; x < 5; x++) {
      Rectangle frame =
          Rectangle{.x = x * 24.0f, .y = 72.0f, .width = 24.0f, .height = 24};

      Image partImage = ImageFromImage(sprite_sheet, frame);
      transition_texture.push_back(LoadTextureFromImage(partImage));
    }

    std::vector<Texture2D> eating_textures;
    for (Texture2D texture : transition_texture) {
      eating_textures.push_back(texture);
    }

    for (int omg = 0; omg < 9; omg++) {
      eating_textures.push_back(sleeping_texture[0]);
    }

    for (auto rit = transition_texture.rbegin();
         rit != transition_texture.rend();
         ++rit) {
      eating_textures.push_back(*rit);
    }

    _idleState = Idle(idle_texture);
    _sleepState = Sleeping(sleeping_texture);
    _enterSleepingState = EnterSleeping(transition_texture);

    _walkingState = Walking(walking_texture);
    _walkingState.window = _gameArea;

    _eating = Eating(eating_textures);
    _eating.window = _gameArea;

    _huntingState = Hunting(walking_texture);
    _huntingState.window = _gameArea;

    currentState = &_walkingState;
    currentState->EnterState(&_position);
  }

  void Update() {
    _frameCounter += 1;
    bool eventFired = false;
    State nextState = currentState->Update(_frameCounter);

    // if the current state update did not return a transition
    // we can check the event queue for a potential next state
    // to transition to.
    if (nextState == UNSET && !eventQueue->empty()) {
      // we just want to peek the queue here, not pop
      // even if their is an event here we may not actually
      // be able to go to it
      TamaEvent e = eventQueue->front();

      switch (e) {
      case EVENT_HYDRATE:
      case EVENT_UNSET:
        TraceLog(LOG_INFO, "we surely never get here that would be weird");
        break;
      case EVENT_GAME:
        eventFired = true;
        nextState = JANKEN;
        break;
      case EVENT_HEADPAT:
        eventFired = true;
        nextState = HEADPAT;
        TraceLog(LOG_INFO, "headpad event doing thing?");
        break;
      case EVENT_FOOD:
        eventFired = true;
        nextState = HUNTING;
        break;
      default:
        TraceLog(
            LOG_ERROR,
            std::format("received invalid state: {}", (int)e).c_str());
      }
    }

    if (nextState != UNSET && this->Transition(nextState)) {
      if (eventFired) {
        eventQueue->pop();
      }
    }
  }

  void Draw() { currentState->Draw(); }

  bool Transition(State nextState) {
    if (!currentState->TryExitState(nextState)) {
      return false;
    }

    switch (nextState) {
    case UNSET:
      break;
    case IDLE:
      std::cout << "enter idle" << std::endl;
      currentState = &this->_idleState;
      break;
    case WALKING:
      std::cout << "enter walking" << std::endl;
      currentState = &this->_walkingState;
      break;
    case SLEEPING:
      std::cout << "enter sleeping" << std::endl;
      currentState = &this->_sleepState;
      break;
    case EATING:
      std::cout << "enter eating" << std::endl;
      currentState = &this->_eating;
      break;
    case HEADPAT:
      std::cout << "enter headpat" << std::endl;
      currentState = &this->_headPatState;
      break;
    case JANKEN:
      std::cout << "enter janken" << std::endl;
      currentState = &this->_jankenState;
      break;
    case HYDRATE:
      std::cout << "enter hydrate" << std::endl;
      break;
    case ENTER_SLEEPING:
      std::cout << "enter enter_sleeping" << std::endl;
      currentState = &this->_enterSleepingState;
      break;
    case HUNTING:
      std::cout << "enter hunting" << std::endl;
      currentState = &this->_huntingState;
    }

    currentState->EnterState(&_position);
    return true;
  }

private:
  Idle _idleState;
  Sleeping _sleepState;
  Eating _eating;
  Walking _walkingState;
  Headpat _headPatState;
  Janken _jankenState;
  EnterSleeping _enterSleepingState;
  Hunting _huntingState;

  Rectangle _gameArea;
  Vector2 _position;
  long _frameCounter;
};
