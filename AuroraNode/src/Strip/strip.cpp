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
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, LED_BUFFER);

  FastLED.showColor(CRGB::Black);

  FastLED.show();
  FastLED.setBrightness(50);

  delay(1000);
  printf("Strip initialized with length: %d \n", numLeds);
}

Strip* Strip::getInstance(uint16_t numLeds) {

  if (Strip::instance == nullptr) {
    Strip::instance = new Strip(numLeds);
    Strip::animator = new Animator();
  }

  return Strip::instance;
}

void Strip::off() {
  this->state = STATE::OFF;
}

void Strip::play() {
  this->state = STATE::PLAYING;
}

void Strip::pause() {
  this->state = STATE::PAUSED;
}

Strip::STATE Strip::getState() {
  return this->state;
}

void Strip::setMaxBrightness(float br) {
  this->maxBrightness = br;
}

float Strip::getMaxBrightness() {
  return this->maxBrightness;
}

void Strip::setAnimationSpeed(uint8_t spd) {
  this->animationSpeed = spd;
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
  // FastLED.showColor(CHSV(h, s, v));
};

void Strip::clearToRGB(uint8_t r, uint8_t g, uint8_t b) {
  // FastLED.showColor(CRGB(r, g, b));
};

void Strip::setPixelColor(uint16_t pixel, uint8_t h, uint8_t s, uint8_t v) {
  FastLED.leds()[pixel] = CHSV(h, s, v);
  FastLED.show();
}

void Strip::test() {
  printf("test \n");

  for (int i = 0; i < this->length; i++) {
    FastLED.leds()[i] = CRGB(50, 0, 0);
    FastLED.show();
    FastLED.delay(25);

    FastLED.leds()[i] = CRGB(0, 50, 0);
    FastLED.show();
    FastLED.delay(25);

    FastLED.leds()[i] = CRGB(0, 0, 50);
    FastLED.show();
    FastLED.delay(25);

    FastLED.leds()[i] = CRGB::Black;
    FastLED.show();
    FastLED.delay(50);
  }
};

uint16_t Strip::getLength() {
  return this->length;
}

void Strip::update() {
  static unsigned long lastUpdate = millis();
  //update every 50ms
  if (millis() - lastUpdate < 30) {
    return;
  }

  digitalWrite(2, !digitalRead(2));
  lastUpdate = millis();

  switch (this->state) {
    case STATE::OFF:
      FastLED.showColor(CRGB::Black);
      break;
    case STATE::PLAYING:
      this->animator->animate(this->frameIndex, (Animator::FX)this->fx);
      break;
    case STATE::PAUSED:
      break;
  }
};