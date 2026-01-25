#include "state.h"
#include <iostream>
#include <queue>
#include <raylib.h>
#include <string>

#ifndef TAMA
#define TAMA

enum TamaEvent { EVENT_UNSET, EVENT_HEADPAT, EVENT_HYDRATE };

class Tama {
public:
  std::queue<TamaEvent> eventQueue;
  std::string name;
  TamaState *currentState;

  Tama(Rectangle gameArea, std::string name) {
    _gameArea = gameArea;

    // we are subtracting 40 pixels here because the gameArea.y + height would
    // be top left corner
    _position =
        Vector2{.x = gameArea.x, .y = gameArea.y + gameArea.height - 64};
    this->name = name;

    std::string rootPath =
        "/home/jane/just-stream/just-ray-bahms/just-juniper/assets/" + name +
        "/";

    _idleState = Idle(rootPath);
    _walkingState = Walking(rootPath);
    _walkingState.window = Rectangle{
        .x = 0, .y = 0, .width = gameArea.width, .height = gameArea.height};
    _sleepState = Sleeping(rootPath);
    _eating = Eating(rootPath);
    _headPatState = Headpat(rootPath);

    currentState = &_headPatState;
    currentState->EnterState(&_position);
  }

  void Update() {
    State nextState = currentState->Update(_frameCounter);

    if (eventQueue.size() > 0) {
      TamaEvent e = eventQueue.front();
      eventQueue.pop();
      switch (e) {
      case EVENT_UNSET:
        break;
      case EVENT_HYDRATE:
        std::cout << "hydration!" << std::endl;
        break;
      case EVENT_HEADPAT:
        nextState = HEADPAT;
        break;
      }
    }

    // if the next state is unset we do not transition this frame
    if (nextState != UNSET) {
      this->Transition(nextState);
    }

    _frameCounter += 1;
    currentState->Update(_frameCounter);
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

#endif
