#pragma once

#include "Arduino.h"
#include <FastLED.h>

// Todo, make abstract and allow differnt animators.
class Animator {
  private:
  uint32_t frameIndex = 0;
  // void fx_aurora();
  // void fx_white_aurora();
  void fx_rainbow();
  // void fx_wavebow();
  // void fx_opposites();
  // void fx_hue_split();
  // void fx_chaser();
  // void fx_white_chaser();
  // void fx_trip();
  // void fx_albiCeleste();

  public:

  enum class FX {
    NONE,
    RAINBOW,
    WAVEBOW,
    AURORA,
    OPPOSITES,
    HUE_SPLIT,
    CHASER,
    WHITE_AURORA,
    WHITE_CHASER,
    TRIP,
    ALBI,
    COUNT,
  };

  void animate(uint32_t frameIndex, FX fx);

};