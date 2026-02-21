#pragma once
#include "constants.h"
#include "state.h"
#include <queue>
#include <raylib.h>
#include <string>

class Tama {
public:
  std::queue<TamaEvent> eventQueue;
  std::string name;
  TamaState *currentState;

  /**
   * @brief Instatiate tama.
   * @param gameArea the part of the screen that is valid for the character
   * @param name the name of the character (used to navigatae assets directory)
   */
  Tama(Rectangle gameArea, std::string name) {
    _gameArea = gameArea;

    _position = Vector2{.x = gameArea.x, .y = TamaConstant::SCREEN_FLOOR - 48};
    this->name = name;

    std::string rootPath = std::string(ASSETS_PATH) + name + "/";

    _idleState = Idle(rootPath);
    _walkingState = Walking(rootPath);
    _walkingState.window = gameArea;
    _sleepState = Sleeping(rootPath);
    _eating = Eating(rootPath);
    _headPatState = Headpat(rootPath);

    currentState = &_idleState;
    currentState->EnterState(&_position);
  }

  void Update() {
    _frameCounter += 1;
    State nextState = currentState->Update(_frameCounter);

    if (nextState == UNSET && eventQueue.size() > 0 && !currentState->BlockTransition()) {
      TamaEvent e = eventQueue.front();
      eventQueue.pop();
      switch (e) {
      case EVENT_UNSET:
      case EVENT_HYDRATE:
        break;
      case EVENT_HEADPAT:
        nextState = HEADPAT;
        break;
      case EVENT_FOOD:
        nextState = EATING;
        break;
      }
    }

    // if the next state is unset we do not transition this frame
    if (nextState != UNSET) {
      this->Transition(nextState);
    }
  }

  void Draw() { currentState->Draw(); }

  void Transition(State nextState) {
    currentState->ExitState();
    switch (nextState) {
    case UNSET:
      break;
    case IDLE:
      currentState = &this->_idleState;
      break;
    case WALKING:
      currentState = &this->_walkingState;
      break;
    case SLEEPING:
      currentState = &this->_sleepState;
      break;
    case EATING:
      currentState = &this->_eating;
      break;
    case HEADPAT:
      currentState = &this->_headPatState;
      break;
    case HYDRATE:
      break;
    }

    currentState->EnterState(&_position);
  }

private:
  Idle _idleState;
  Sleeping _sleepState;
  Eating _eating;
  Walking _walkingState;
  Headpat _headPatState;

  Rectangle _gameArea;
  Vector2 _position;
  long _frameCounter;
};
