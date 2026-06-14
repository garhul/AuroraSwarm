#include "strip.h"

Strip* Strip::instance = nullptr;
Animator* Strip::animator = nullptr;

Strip::Strip(uint16_t numLeds) {
  this->state = STATE::OFF;

  if (numLeds > LED_BUFFER) {
    printf("Warning: numLeds (%d) is greater than LED_BUFFER (%d), capping to LED_BUFFER \n", numLeds, LED_BUFFER);
    numLeds = LED_BUFFER;
  }
  this->length = numLeds;

  CRGB* leds = new CRGB[LED_BUFFER];
  FastLED.addLeds<WS2811, DATA_PIN, GRB>(leds, numLeds);
  FastLED.setMaxRefreshRate(0.02);

  FastLED.showColor(CRGB::Black);

  FastLED.show();
  FastLED.setBrightness(255);

  delay(1000);
  printf("Strip initialized with length: %d \n", numLeds);
}

Strip* Strip::getInstance(uint16_t numLeds) {

  if (Strip::instance == nullptr) {
    Strip::instance = new Strip(numLeds);
    Strip::animator = new Animator(numLeds);
    instance->setAnimationSpeed(2);
  }

  return Strip::instance;
}


void Strip::off() {
  this->state = STATE::OFF;
}



void Strip::play() {
  this->state = STATE::PLAYING;
}

void Strip::pause(bool toggle) {
  if (toggle) {
    this->state = (this->state == STATE::PAUSED) ? STATE::PLAYING : STATE::PAUSED;
  } else {
    this->state = STATE::PAUSED;
  }
}

Strip::STATE Strip::getState() {
  return this->state;
}

void Strip::setMaxBrightness(uint8_t br) {
  this->maxBrightness = br;
  FastLED.setBrightness(br);
}

uint8_t Strip::getMaxBrightness() {
  return this->maxBrightness;
}

void Strip::setAnimationSpeed(uint8_t spd) {
  this->animationSpeed = spd;
  this->animator->setTransitionSpeed(spd);
}

uint8_t Strip::getAnimationSpeed() {
  return this->animationSpeed;
};

void Strip::setFx(Animator::FX fx) {
  this->fx = fx;
};

Animator::FX Strip::getFx() {
  return (Animator::FX)this->fx;
};

void Strip::clearToHSV(uint8_t h, uint8_t s, uint8_t v) {
  instance->state = STATE::FIXED_COLOR;
  for (uint16_t idx = 0; idx < instance->length; idx++)
    FastLED.leds()[idx] = CHSV(h, s, v);
};

void Strip::clearToRGB(uint8_t r, uint8_t g, uint8_t b) {
  instance->state = STATE::FIXED_COLOR;
  for (uint16_t idx = 0; idx < instance->length; idx++)
    FastLED.leds()[idx] = CRGB(r, g, b);
};

void Strip::setPixelColor(uint16_t pixel, uint8_t h, uint8_t s, uint8_t v) {
  instance->state = STATE::FIXED_COLOR;
  FastLED.leds()[pixel] = CHSV(h, s, v);
  FastLED.show();
}

void Strip::test() {
  DEBUG("Running strip test \n");

  for (int i = 0; i < this->length; i++) {
    FastLED.leds()[i] = CRGB(50, 0, 0);
    FastLED.show();
    FastLED.delay(5);

    FastLED.leds()[i] = CRGB(0, 50, 0);
    FastLED.show();
    FastLED.delay(5);

    FastLED.leds()[i] = CRGB(0, 0, 50);
    FastLED.show();
    FastLED.delay(5);

    FastLED.leds()[i] = CRGB::Black;
    FastLED.show();
    FastLED.delay(5);
  }

};

uint16_t Strip::getLength() {
  return this->length;
}

void Strip::update() {
  static unsigned long lastUpdate = millis();
  //update at ~48 fps -> 0.0208 s per update
  if (millis() - lastUpdate < 20) {
    return;
  }

  lastUpdate = millis();

  switch (this->state) {
    case STATE::OFF:
      FastLED.clear();
      FastLED.showColor(CRGB::Black);
      break;
    case STATE::PLAYING:
      this->animator->animate(this->frameIndex, (Animator::FX)this->fx);
      break;
    case STATE::PAUSED:
      FastLED.show();
      break;
    case STATE::FIXED_COLOR:
      FastLED.show();
      break;
  }
};