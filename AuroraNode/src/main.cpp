// #include "main.h"
#include <Arduino.h>
#include "EspNow/wrapper.h"
#include "Strip/strip.hpp"


/* TODO:: make these params part of the eeprom and configurable via ESPNOW*/
#define LED_COUNT 100
// #define 


#define BTN_A D6
#define BTN_B D7

ESPNowWrapper espNow(false);
Strip strip;

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

  espNow.begin();
  digitalWrite(LED_BUILTIN, LOW);

  strip.begin(LED_COUNT);

  // bindHandlers();

}

void loop() {
  static unsigned long backOff_A = millis();
  // static unsigned long backOff_B = millis();

  strip.update();
  if (digitalRead(BTN_A) == LOW && backOff_A < millis()) {
    if (digitalRead(LED_BUILTIN) == LOW) {
      espNow.requestToPair();
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
}
