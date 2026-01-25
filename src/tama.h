#include "state.h"
#include <iostream>
#include <raylib.h>
#include <string>

#ifndef TAMA
#define TAMA

class Tama {
public:
  int headpat = 0;
  std::string name;
  TamaState *currentState;

  Tama(int width, int height, std::string name) {
    _width = width;
    _height = height;
    _position = Vector2{.x = 32, .y = 130};
    this->name = name;

    std::string rootPath =
        "/home/jane/just-stream/just-ray-bahms/just-tamagotchi/assets/" + name +
        "/";

    _idleState = Idle(rootPath);
    _walkingState = Walking(rootPath);
    _walkingState.window =
        Rectangle{.x = 0, .y = 0, .width = 130, .height = 150};
    _sleepState = Sleeping(rootPath);
    _eating = Eating(rootPath);
    _headPatState = Headpat(rootPath);

    currentState = &_headPatState;
    currentState->EnterState(&_position);
  }

  void Update() {
    State nextState = currentState->Update(_frameCounter);
    if (nextState != UNSET) {
      std::cout << "transitioning state!" << currentState->state << "->"
                << nextState << std::endl;
      this->Transition(nextState);
    }

    _frameCounter += 1;
    currentState->Update(_frameCounter);
  }

  void Draw() { currentState->Draw(); }

  void Transition(State nextState) {
    if (headpat > 0) {
      headpat -= 1;
      nextState = HEADPAT;
    }

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

  Vector2 _position;
  int _width;
  int _height;
  long _frameCounter;
};

#endif
