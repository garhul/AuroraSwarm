#include "animator.h"




Animator::Animator(uint16_t numLeds) {
  this->numLeds = numLeds;
}

/**
 * Linearly changes the hue of all pixels on the strip over time, creating a rainbow effect.
 * The speed of the hue change is determined by the frame index and the transition speed.
 */
void Animator::fx_rainbow() {
  static uint8_t hue = 0;
  this->frameIndex++;

  if (this->frameIndex % this->transitionSpeed == 0) {
    hue++;
  }

  FastLED.showColor(CHSV(hue, 255, 255));
}


void Animator::fx_chaser() {
  static uint8_t spd = 0;

  for (uint16_t idx = 0; idx < this->numLeds; idx++) {
    FastLED.leds()[idx] = CHSV(sin8(idx + spd), 255, sin8(spd + (idx * 16))); //sin8(idx / 2));
  }

  if (this->frameIndex % this->transitionSpeed == 0) {
    spd += 4;
  }

  FastLED.show();
  this->frameIndex++;
}


void Animator::fx_aurora() {
  static uint8_t start_hue = 120;
  static uint8_t center_hue = 150;
  static int8_t dir = 1;
  static uint16_t br = 0;
  static uint8_t increment = 4;
  uint16_t midpoint = this->numLeds / 2;


  float hue_increment = (center_hue - start_hue) / ((float)midpoint / 2);
  float hue = start_hue;

  if (this->frameIndex % this->transitionSpeed == 0) {
    for (uint16_t idx = 0; idx < this->numLeds; idx++) {
      if (idx > midpoint) {
        br += 8;
        hue -= hue_increment;
      } else {
        br -= 8;
        hue += hue_increment;
      }
      FastLED.leds()[idx] = CHSV(hue, 255, sin16(br)); //sin8(idx / 2));
    }
  }
  FastLED.show();


  this->frameIndex++;
}

void Animator::fx_albiceleste() {
  uint16_t idx = 0;
  this->frameIndex++;

  uint16_t third = this->numLeds / 3;

  for (idx = 0; idx < this->numLeds; idx++) {
    if (idx < third) {
      FastLED.leds()[idx] = CHSV(131, 255, 200);
    } else if (idx > third && idx < (2 * third)) {
      FastLED.leds()[idx] = CHSV(0, 0, 120);
    } else {
      FastLED.leds()[idx] = CHSV(135, 255, 200);
    }
  }

  FastLED.show();
}

/** sets a scale for updating transitions within animations
 * spd gets inverted ( 0 -> 255, 255->0) higher transition speed it takes more frames for transition to update
 *
  */
void Animator::setTransitionSpeed(uint8_t spd) {
  this->transitionSpeed = map(spd, 0, 255, 255, 0);
  if (this->transitionSpeed == 0) this->transitionSpeed = 1;
  DEBUG("Transition speed set to %d \n", this->transitionSpeed);
}



/**
 * This animation has several stages making it rather complex
 * (0 to 192 frames off) Starts with an off state for a small random amount of time from .5 to 4 seconds (24 to 192 frames)
 * Then a single pixel at 1/4th the brightness spreads towards the beginging and end of of the strip at a fast speed of 2 seconds per full strip (5 frames for a 200 led strip)
 * Then the strip goes black for 0.5 seconds (12 frames)
 * Then the strip turns on on for 0.5 seconds (12 frames)
 * cycle repeats for a maximum of 4 times, with timing ranging from .5 to 1 second of blackness in between and increasing brightness each time until it gets to 255
 * after cycle is complete the random numbers are regenerated and the animation starts again
 */

void Animator::fx_lightning() {
  static uint8_t step = 0;
  static uint8_t firstStepDuration = 0;
  static uint8_t secondStepDuration = 0;
  static uint16_t seedPosition = 0;
  static uint8_t seedGrowRate = 0;
  static uint8_t fullLightningDuration = 0;
  static uint8_t noLightningDuration = 0;
  static uint8_t cycles = 0;
  static bool refreshRandomValues = true;

  if (refreshRandomValues) {
    refreshRandomValues = false;
    this->frameIndex = 0;
    firstStepDuration = random(24, 192);
    secondStepDuration = 0;
    seedPosition = random((this->numLeds / 4), (this->numLeds / 4) * 3);
    //seed grow rate goes from 2 to 5 seconds for the whole strip
    // FPS -> 48  
    // @ 1px per frame -> 300 leds -> 6.5 seconds
    // if I want 1 second  for the whole strip that's ~6.25 leds per frame (300 / 48 = 6.25)
    // if I want 5 seconds for the whole strip that's 1.2 leds per frame (300 / 240  = 1.25 )
    // bottom -> stripLen / 
    seedGrowRate = random(ceil(this->numLeds / 48), ceil(this->numLeds / 240));
    fullLightningDuration = random(4, 12);
    noLightningDuration = random(6, 24);
    cycles = random(1, 4);
  }



  switch (step) {
    case 0:
      FastLED.showColor(CRGB::Black);
      if (frameIndex == firstStepDuration) {
        step = 1;
        refreshRandomValues = true;
      }
      break;
    case 1:
      if (this->frameIndex == 0) {
        FastLED.clear(true);
      } else {
        int start = seedPosition - (this->frameIndex * seedGrowRate);
        int end = seedPosition + (this->frameIndex * seedGrowRate);

        if (start < 0) start = 0;
        if (end > this->numLeds) end = this->numLeds;

        for (uint16_t idx = start; idx <= end; idx++) {
          FastLED.leds()[idx] = CHSV(90, 1, 60);
        }
        if (start == 0 && end == this->numLeds) {
          step = 2;
          refreshRandomValues = true;
        }
        FastLED.show();
      }
      break;
    case 2:
      if (frameIndex == 0) {
        FastLED.showColor(CHSV(0, 0, 255)); // all on
      } else if (frameIndex == fullLightningDuration) {
        FastLED.showColor(CHSV(0, 0, 32)); //dim it
        DEBUG("dimming \n");
      } else if (frameIndex == fullLightningDuration + noLightningDuration) {
        DEBUG("looping cycles \n");
        refreshRandomValues = true;
        cycles--;
        if (cycles == 0) {
          DEBUG("RESETTING cycles \n");
          step = 0;
        }
      }
      break;

    default:
      break;
  }


  this->frameIndex++;

}


/*
  sets end and begining of strip to the same hue, middle of the strip to a different hue and fades the hue towards the edges
  Start and end hue will be parametrized but currently remain at dark blue / purple
*/

#define ST_H 140
#define CT_H 180
inline void Animator::fx_waveBow() {
  static uint8_t start_hue = ST_H;
  static uint8_t center_hue = CT_H;
  static int8_t dir = 1;
  uint16_t midpoint = this->numLeds / 2;


  float hue_increment = (center_hue - start_hue) / ((float)midpoint / 2);
  float hue = start_hue;

  for (uint16_t idx = 0; idx < this->numLeds; idx++) {
    if (idx > midpoint) {
      hue -= hue_increment;
    } else {
      hue += hue_increment;
    }

    FastLED.leds()[idx] = CHSV(hue, 255, 255); //sin8(idx / 2));
  }

  FastLED.show();

  if (this->frameIndex % this->transitionSpeed == 0) {
    if (start_hue == CT_H) {
      dir = -1;
    } else if (start_hue < ST_H) {
      dir = 1;
    }

    start_hue += dir;
  }

  this->frameIndex++;
}

inline void Animator::fx_opposites() {
  static uint8_t hue = 0;
  for (uint16_t idx; idx < this->numLeds; idx++) {
    FastLED.leds()[idx] = (idx > (this->numLeds / 2)) ? CHSV(hue + 128, 255, 255) : CHSV(hue, 255, 255);
  }
  FastLED.show();

  if (this->frameIndex % this->transitionSpeed == 0) {
    hue++;
  }

  this->frameIndex++;
}

// inline void Animator::fx_() {
// }

inline void Animator::fx_police() {
  static uint8_t mode = 0;
  static uint8_t cycles = 3;
  uint16_t midpoint = this->numLeds / 2;
  static bool resetFrameCount = false;
  // 2 blink rates, slow and fast
  // 3 cycles per blink rate

  if (resetFrameCount) {
    this->frameIndex = 0;
    resetFrameCount = false;
  }

  switch (mode) {
    case 0:
      if (this->frameIndex == 0) {
        FastLED.showColor(CRGB::Black);
      } else if (this->frameIndex == 3) {
        // flash red 3 times
        for (uint16_t idx = 0; idx < this->numLeds; idx++) {
          FastLED.leds()[idx] = (idx > midpoint) ? CRGB::Black : CRGB::Red1;
        }

        FastLED.show();
      } else if (this->frameIndex == 6) {
        FastLED.showColor(CRGB::Black);

        cycles--;
        resetFrameCount = true;

        if (cycles == 0) {
          mode = 1;
          cycles = 3;
        }
      }
      break;
    case 1:
      if (this->frameIndex == 0) {
        FastLED.showColor(CRGB::Black);

      } else if (this->frameIndex == 3) {
        // flash blue 3 times
        for (uint16_t idx = 0; idx < this->numLeds; idx++) {
          FastLED.leds()[idx] = (idx > midpoint) ? CRGB::Blue1 : CRGB::Black;
        }
        FastLED.show();
      } else if (this->frameIndex == 6) {
        FastLED.showColor(CRGB::Black);

        cycles--;
        resetFrameCount = true;

        if (cycles == 0) {
          mode = 2;
          cycles = 3;
        }
      }
      break;

    case 2:
      if (this->frameIndex == 0) {
        for (uint16_t idx = 0; idx < this->numLeds; idx++) {
          FastLED.leds()[idx] = (idx > midpoint) ? CRGB::Black : CRGB::Blue1;
        }
        FastLED.show();

      } else if (this->frameIndex == 24) {
        for (uint16_t idx = 0; idx < this->numLeds; idx++) {
          FastLED.leds()[idx] = (idx > midpoint) ? CRGB::Red1 : CRGB::Black;
        }
        FastLED.show();

      } else if (this->frameIndex == 48) {
        cycles--;
        resetFrameCount = true;

        if (cycles == 0) {
          cycles = 3;
          mode = 0;
        }
      }
      break;

  }

  this->frameIndex++;
}

inline bool animationChanged(Animator::FX newFx) {
  static Animator::FX prevFx = Animator::FX::NONE;

  if (newFx != prevFx) {
    prevFx = newFx;
    return true;
  }
  return false;
}

void Animator::animate(uint32_t frameIndex, Animator::FX fx) {
  static uint8_t randomSeeds[4] = { 0, 0, 0, 0 };
  // reset animation values if the animation has changed

  if (animationChanged(fx)) {
    this->frameIndex = 0;

    randomSeeds[0] = random8();
    randomSeeds[1] = random8();
    randomSeeds[2] = random8();
    randomSeeds[3] = random8();
  }


  switch (fx) {
    case Animator::FX::RAINBOW:
      this->fx_rainbow();
      break;
    case Animator::FX::AURORA:
      this->fx_aurora();
      break;
    case Animator::FX::CHASER:
      this->fx_chaser();
      break;
    case Animator::FX::OPPOSITES:
      this->fx_opposites();
      break;
    case Animator::FX::WAVEBOW:
      this->fx_waveBow();
      break;
    case Animator::FX::LIGHTNING:
      this->fx_lightning();
      break;
    case Animator::FX::ALBICELESTE:
      this->fx_albiceleste();
      break;
    case Animator::FX::POLICE:
      this->fx_police();
      break;
    default:
      break;
  }
}