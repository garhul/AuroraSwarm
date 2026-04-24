#include "main.h"


ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
Terminal term;

// void hndlr(uint8_t* macAddr, uint8_t* message, uint8_t len) {
//   Serial.println("Sample command received");
//   digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
// }




void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);

  espNow->begin();
  espNow->setAutoPair(false);
  digitalWrite(LED_BUILTIN, LOW);
}

void loop() {
  static unsigned long backOff_A = millis();
  static unsigned long backOff_B = millis();
  term.poll();

  if (digitalRead(BTN_A) == LOW && backOff_A < millis()) {
    if (digitalRead(LED_BUILTIN) == LOW) {
      espNow->setAutoPair(true);
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      espNow->setAutoPair(false);
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
