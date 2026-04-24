#pragma once
#include "Arduino.h"
#include <FastLED.h>


template <uint8_t s>
void animationOneNExtFame(CRGBArray<s> leds, const uint8_t max_br, uint8_t speed) {
  static uint8_t hue = 120;
  for (int i = 0; i < length / 2; i++) {

    leds.fadeToBlackBy(10);

    leds[i] = CHSV(hue, 255, 50);
    leds(length / 2, length - 1) = leds(length / 2 - 1, 0);

    FastLED.delay(33);
    // FastLED.show();
  };
  hue = (hue < 180) ? hue + 1 : 120;
}




template <uint16_t size>
class Strip {
  enum FX {
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

  enum MODE {
    OFF,
    PLAYING,
    PAUSED
  };


  private:
  uint16_t length = size;
  uint8_t state;
  uint8_t animations;
  uint8_t maxBrightness = 50;
  uint8_t animationSpeed = 10;
  uint8_t fx = 0;
  CRGBArray<size> leds;
  float maxBrightness;
  void nextFrame();

  public:
  Strip() {};

  void begin(uint8_t len) {
    FastLED.addLeds<NEOPIXEL, 2>(leds, length);
  };

  uint8_t getState() {
    return this->state;
  }

  uint8_t setState(uint8_t state) {
    this->state = state;
  }

  void off() {
    this->state = OFF;
  }

  void play() {
    this->state = PLAYING;
  }

  void pause() {
    this->state = PAUSED;
  }

  void setMaxBrightness(uint8_t br) {
    this->maxBrightness = br;
  }

  void setAnimationSpeed(uint8_t spd) {
    this->animationSpeed = spd;
  }

  void clearToRGB(uint8_t r, uint8_t g, uint8_t b) {
    leds.fill_solid(CRGB(r, g, b);)
  };

  void clearToHSV(uint8_t h, uint8_t s, uint8_t v) {
    leds.fill_solid(CHSV(h, s, v));
  }

  void setFx(uin8_t fx) {
    this->fx = fx;
  }

  uint8_t getFx() {
    return this->fx;
  }

  void test() {
    this->state = PLAYING;

    leds.fill_solid(CHSV(0, 0, 0));
    int i = 0;
    for (i = 0; i < length; i++) {
      leds[i] = CHSV((255 / i) * i, 255, this->maxBrightness);
      FastLED.delay(30);
    };

    for (i; i <= 0; i--) {
      leds[i] = CHSV(0, 0, 0);
      FastLED.delay(30);
    }

    this->state = OFF;
  };

  void getLength();

  void update() {
    switch (state) {
      case OFF:
        FastLED.clear(true);
        break;


      case PLAYING:
        this->nextFrame();
        break;

    }
  }
};