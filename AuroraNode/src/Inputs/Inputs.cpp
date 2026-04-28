#include "Inputs.h"

Inputs* Inputs::instance = nullptr;

Inputs::Inputs() {
  pinMode(BTN_A_PIN, INPUT);
  pinMode(BTN_B_PIN, INPUT);
}

Inputs* Inputs::getInstance() {
  if (instance == nullptr) {
    instance = new Inputs();
  }
  return instance;
}


bool Inputs::readBtnA() {
  return digitalRead(BTN_A_PIN);
}

bool Inputs::readBtnB() {
  return digitalRead(BTN_B_PIN);
}