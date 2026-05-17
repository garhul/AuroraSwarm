#include "animator.h"

void Animator::fx_rainbow() {
  this->frameIndex++;
  FastLED.showColor(CHSV(frameIndex % 255, 255, 255));
}

void Animator::fx_wave() {
  static uint8_t idx = 0;
  this->frameIndex++;

  FastLED.leds()[idx] = CHSV(frameIndex * 4, 255, 255);
  idx = ++idx % FastLED.size();
  // INFO("idx = %d \n", idx);
}

void Animator::animate(uint32_t frameIndex, Animator::FX fx) {
  static Animator::FX prev_fx = Animator::FX::NONE;

  if (prev_fx != fx) {
    prev_fx = fx;
    this->frameIndex = 0;
  }

  switch (fx) {
    case Animator::FX::RAINBOW:
      this->fx_rainbow();
      break;
    case Animator::FX::WAVEBOW:
      this->fx_wave();
      FastLED.show();
      break;

    default:
      break;
  }
}