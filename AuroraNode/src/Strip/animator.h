#pragma once

#include "Arduino.h"
#include <FastLED.h>
#include "Logger.h"

#define HUE_AURORA_GREEN 30
#define HUE_AURORA_PINK 160
#define HUE_AURORA_BLUE 96

#define FPS 48

// Todo, make abstract and allow differnt animators.
class Animator {
  private:
  uint8_t transitionSpeed = 128;
  uint32_t frameIndex = 0;
  uint16_t numLeds = 0;
  void fx_rainbow();
  void fx_waveBow();
  void fx_aurora();
  void fx_opposites();
  void fx_lightning();
  void fx_chaser();
  void fx_police();
  void fx_albiceleste();

  public:
  Animator(uint16_t numLeds);

  enum class FX {
    NONE,
    RAINBOW, // changes hue on all leds 
    WAVEBOW, //  changes hue from center to edges and bounces back, uses two colors as inputs
    AURORA, // simulates aurora effect
    OPPOSITES, // sets opposite sides of the strip to complementary hues and rotates them inwards or outwards
    LIGHTNING, // flashes the strip white for a single frame emulating lightning storms
    CHASER, // a single bright pixel that chases around the strip
    POLICE, // two pixels of complementary hues chase around the strip
    ALBICELESTE, // celeste y blanca

    COUNT,
  };

  void setTransitionSpeed(uint8_t spd);
  void animate(uint32_t frameIndex, FX fx);
};