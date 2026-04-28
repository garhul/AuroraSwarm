
#include <Arduino.h>
#include "EspNowWrapper.h"
#include "Strip/strip.h"
// #include "Settings/settings.h"
// #include "Inputs/Inputs.h"

#define BTN_A D6
#define BTN_B D7

// Todo - add settings manager and persist settings to eeprom
#define NUM_LEDS 28

ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
Strip* strip = Strip::getInstance(NUM_LEDS);

// void hndlr(uint8_t* macAddr, uint8_t* message, uint8_t len) {
//   Serial.println("Sample command received");
//   digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
// }

void bindHandlers() {

}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  espNow->begin();
  digitalWrite(LED_BUILTIN, LOW);

  // bindHandlers();

  strip->test();
  strip->off();
  strip->setFx(Animator::FX::RAINBOW);
  strip->play();

}

void loop() {

  strip->update();

  static unsigned long backOff_A = millis();
  // static unsigned long backOff_B = millis();

  // Strip.update();
  if (digitalRead(BTN_A) == LOW && backOff_A < millis()) {
    if (digitalRead(LED_BUILTIN) == LOW) {
      espNow->requestToPair();
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
    backOff_A = millis() + 1000;
  }

  // if (digitalRead(BTN_B) == LOW && backOff_B < millis()) {
  //   Serial.println("[ INFO ] - Sending custom command");
  //   const uint8_t msg[4] = { MSG_SAMPLE_CMD, 0, 0, 0 };
  //   espNow.sendToAll((const uint8_t*)&msg, 4);
  //   backOff_B = millis() + 1000;
  // }
};
