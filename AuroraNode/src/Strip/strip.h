#pragma once


#include "Arduino.h"
#include "animator.h"
#include <FastLED.h>

#define LED_BUFFER 300 
#define DATA_PIN 4

class Strip {
  enum class STATE {
    OFF,
    PLAYING,
    PAUSED
  };

  private:
  static Strip* instance;
  static Animator* animator;
  uint16_t length = 0;
  STATE state;
  uint8_t animationSpeed = 10;
  Animator::FX fx = (Animator::FX)0;
  uint32_t frameIndex = 0;

  float maxBrightness = .5;
  void animate();
  void drawFrame();

  Strip(uint16_t numLeds);

  public:
  static Strip* getInstance(uint16_t numLeds);
  STATE getState();
  uint8_t setState(STATE state);
  void off();
  void play();
  void pause();

  void setMaxBrightness(float br);
  float getMaxBrightness();

  void setAnimationSpeed(uint8_t spd);
  uint8_t getAnimationSpeed();
  void setFx(Animator::FX fx);
  Animator::FX getFx();

  void clearToHSV(uint8_t h, uint8_t s, uint8_t v);
  void clearToRGB(uint8_t r, uint8_t g, uint8_t b);

  void test();
  uint16_t getLength();
  void update();
};

