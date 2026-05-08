#include "main.h"

ESPNowWrapper* espNow = nullptr;
Terminal term;

// void hndlr(uint8_t* macAddr, uint8_t* message, uint8_t len) {
//   Serial.println("Sample command received");
//   digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
// }

void setup() {
// #ifndef ARDUINO_USB_MODE
  Serial.begin(115200);
// #endif
  delay(2000);

  pinMode(BTN_A, INPUT_PULLUP);
  pinMode(BTN_B, INPUT_PULLUP);

  espNow = ESPNowWrapper::getInstance(true);
  espNow->begin();
  espNow->setAutoPair(true);
}

inline void handleBtnA() {
  INFO("Sending CMD PLAY to all devices");
  uint8_t msg[2] = {
   (uint8_t)MSG_TYPE::MSG_AURORA_CMD,
   (uint8_t)AURORA_COMMANDS::CMD_PLAY
  };

  espNow->sendToAll(msg, 2);
}

inline void handleBtnB() {
  INFO("Sending CMD OFF to all devices");
  uint8_t msg[2] = {
   (uint8_t)MSG_TYPE::MSG_AURORA_CMD,
   (uint8_t)AURORA_COMMANDS::CMD_OFF
  };

  espNow->sendToAll(msg, 2);
}


inline void nextPx() {
  static uint8_t h = 0;
  static uint8_t idx = 0;
  static uint8_t l = 0;

  INFO("Sending CMD SET PX to all devices");
  h++;


  uint8_t msg[6] = {
  (uint8_t)MSG_TYPE::MSG_AURORA_CMD,
  (uint8_t)AURORA_COMMANDS::CMD_SET_PX,
  (uint8_t)idx,
   h,
   255,
   l
  };

  espNow->sendToAll(msg, 6);

  if (l == 50) {
    l = 0;
  } else {
    idx++;
    if (idx > 10) idx = 0;
    l = 50;
  }
}

void loop() {
  static bool doBurst = false;
  static unsigned long backOff_A = millis();
  static unsigned long backOff_B = millis();
  static unsigned long lastmsg = millis();
  term.poll();

  if (digitalRead(BTN_A) == LOW && backOff_A < millis()) {
    // handleBtnA();
    doBurst = !doBurst;
    backOff_A = millis() + 1000;
  }

  // if (digitalRead(BTN_B) == LOW && backOff_B < millis()) {
  //   handleBtnB();
  //   backOff_B = millis() + 1000;
  // }

  if (doBurst) {
    if (lastmsg < millis()) {
      lastmsg = millis() + 1000;
      nextPx();
    }
  }
}

// if (digitalRead(BTN_B) == LOW && backOff_B < millis()) {
//   Serial.println("[ INFO ] - Sending custom command");
//   const uint8_t msg[4] = { MSG_SAMPLE_CMD, 0, 0, 0 };
//   espNow.sendToAll((const uint8_t*)&msg, 4);
//   backOff_B = millis() + 1000;
// }
