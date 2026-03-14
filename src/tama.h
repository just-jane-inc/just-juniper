#pragma once
#include "constants.h"
#include "state.h"
#include <iostream>
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

    std::string assetsDirectory = "resources/" + name + "/";

    _idleState = Idle(assetsDirectory);
    _sleepState = Sleeping(assetsDirectory);
    _headPatState = Headpat(assetsDirectory);
    _jankenState = Janken(assetsDirectory);
    _enterSleepingState = EnterSleeping(assetsDirectory);

    _walkingState = Walking(assetsDirectory);
    _walkingState.window = _gameArea;
    _eating = Eating(assetsDirectory);
    _eating.window = _gameArea;
    _huntingState = Hunting(assetsDirectory);
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
    if (nextState == UNSET && eventQueue.size() > 0) {
      // we just want to peek the queue here, not pop
      // even if their is an event here we may not actually
      // be able to go to it
      TamaEvent e = eventQueue.front();
      switch (e) {
      case EVENT_HYDRATE:
      case EVENT_UNSET:
        break;
      case EVENT_GAME:
        eventFired = true;
        nextState = JANKEN;
        break;
      case EVENT_HEADPAT:
        eventFired = true;
        nextState = HEADPAT;
        break;
      case EVENT_FOOD:
        eventFired = true;
        nextState = HUNTING;
        break;
      }
    }

    if (nextState != UNSET && this->Transition(nextState)) {
      if (eventFired) {
        eventQueue.pop();
      }
    }
  }

  void Draw() { currentState->Draw(); }

  bool Transition(State nextState) {
    if (currentState->TryExitState(nextState)) {
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

    return false;
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
