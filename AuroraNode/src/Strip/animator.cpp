#include "animator.h"

void Animator::fx_rainbow() {
  this->frameIndex++;
  FastLED.showColor(CHSV(frameIndex % 255, 255, 255));
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
    default:
      break;
  }
}