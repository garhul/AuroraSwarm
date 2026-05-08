
#include <Arduino.h>
#include "Logger.h"
#include "defaults.h"
#include "Strip/strip.h"
#include "EspNowWrapper.h"
// #include "Settings/settings.h"
// #include "Inputs/Inputs.h"

ESPNowWrapper* espNow = nullptr;
Strip* strip = nullptr;

void auroraHndlr(uint8_t* macAddr, uint8_t* message, uint8_t len) {

  switch ((AURORA_COMMANDS)message[1]) {
    case AURORA_COMMANDS::CMD_SET_PX:
      strip->pause();
      strip->setPixelColor(message[2], message[3], message[4], message[5]);
      break;

    case AURORA_COMMANDS::CMD_SET_HSV:
      strip->pause();
      strip->clearToHSV(message[2], message[3], message[4]);
      break;

    case AURORA_COMMANDS::CMD_SET_BR:
      strip->setMaxBrightness(message[2]);
      break;

    case AURORA_COMMANDS::CMD_FX_SPEED:
      strip->setAnimationSpeed(message[2]);
      break;

    case AURORA_COMMANDS::CMD_PLAY:
      strip->play();
      break;

    case AURORA_COMMANDS::CMD_OFF:
      strip->off();
      break;

    default:
      WARN("unknown command [%d]", message[1]);

  }
}


void setup() {
  delay(2000);

#ifdef ESP8266
  Serial.begin(115200);
#endif

  INFO("Starting Aurora Node \n");
  strip = Strip::getInstance(NUM_LEDS);
  pinMode(PAIR_BTN, INPUT_PULLUP);
  espNow = ESPNowWrapper::getInstance();
  espNow->begin();
  espNow->registerHandler(MSG_TYPE::MSG_AURORA_CMD, auroraHndlr);

  strip->test();
  strip->off();
  strip->setFx(Animator::FX::RAINBOW);
  strip->pause();
}

void loop() {
  static unsigned long backOff_A = millis();
  strip->update();

  if (digitalRead(PAIR_BTN) == LOW && backOff_A < millis()) {
    INFO("Requesting to pair \n");
    if (strip->getState() == Strip::STATE::OFF) {
      strip->play();
    } else {
      strip->off();
    }

    espNow->requestToPair();
    backOff_A = millis() + 2000;
  }
};
