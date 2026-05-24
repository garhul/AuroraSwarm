
#include <Arduino.h>
#include "Logger.h"
#include "defaults.h"
#include "Strip/strip.h"
#include "EspNowWrapper.h"
#include "types.h"
// #include "Settings/settings.h"
// #include "Inputs/Inputs.h"

#define BUFFER_SIZE 32

ESPNowWrapper* espNow = nullptr;
Strip* strip = nullptr;


inline void processCmd(AURORA_COMMANDS cmd, uint8_t* args) {


  switch (cmd) {
    case AURORA_COMMANDS::CMD_SET_PX:
      strip->pause();
      strip->setPixelColor(args[0], args[1], args[2], args[3]);
      break;

    case AURORA_COMMANDS::CMD_SET_HSV:
      DEBUG("Setting HSV: [%d, %d, %d] \n", args[0], args[1], args[2]);
      strip->pause();
      strip->clearToHSV(args[0], args[1], args[2]);
      break;

    case AURORA_COMMANDS::CMD_SET_BR:
      strip->setMaxBrightness(args[0]);
      break;

    case AURORA_COMMANDS::CMD_FX_SPEED:
      strip->setAnimationSpeed(args[0]);
      break;

    case AURORA_COMMANDS::CMD_PLAY:
      strip->play();
      break;

    case AURORA_COMMANDS::CMD_OFF:
      strip->off();
      break;

    default:
      WARN("unknown command [%d] \n", cmd);
  }
}

// Todo:: move this into some handler stuff
void auroraHndlr(uint8_t* macAddr, uint8_t* message, uint8_t len) {

  message[len] = '\0'; // Null-terminate the message for safe string operations

  if (message[1] == 'H') {
    DEBUG("Received human readable message: %s \n", String((char*)&message[2]).c_str());
    char* tokens[BUFFER_SIZE];
       /* Examples of messages
    Hfx:1
    Hspd:10
    Hbr:255
    Hhsv:120:255:50
    Hpx:5:120:255:50
    Hplay
    Hpause
    Hoff
  */


    uint8_t tokensCount = 0;
    uint8_t payloadBuff[8];
    char* token = strtok((char*)&message[1], ":");

    while (token != NULL) {
      tokens[tokensCount] = token;
      payloadBuff[tokensCount] = atoi(token);
      DEBUG(" Token [%d]:%s \n", tokensCount, String(token));
      token = strtok(NULL, ":");
      tokensCount++;
    }

    String payload = String(tokens[1]);
    payload.toLowerCase();

    if (payload.startsWith("fx")) {
      processCmd(AURORA_COMMANDS::CMD_FX, &payloadBuff[2]);
    } else if (payload.startsWith("spd")) {
      processCmd(AURORA_COMMANDS::CMD_FX_SPEED, &payloadBuff[2]);
    } else if (payload.startsWith("br")) {
      processCmd(AURORA_COMMANDS::CMD_SET_BR, &payloadBuff[2]);
    } else if (payload.startsWith("hsl")) {
      processCmd(AURORA_COMMANDS::CMD_SET_HSV, &payloadBuff[2]);
    } else if (payload.startsWith("px")) {
      processCmd(AURORA_COMMANDS::CMD_SET_PX, &payloadBuff[2]);
    } else if (payload.startsWith("play")) {
      processCmd(AURORA_COMMANDS::CMD_PLAY, nullptr);
    } else if (payload.startsWith("pause")) {
      processCmd(AURORA_COMMANDS::CMD_PAUSE, nullptr);
    } else if (payload.startsWith("off")) {
      processCmd(AURORA_COMMANDS::CMD_OFF, nullptr);
    } else {
      WARN("Unknown human readable command: %s \n", payload);
    }
  } else {
    processCmd((AURORA_COMMANDS)message[2], &message[3]);
  }
}


void setup() {
  delay(2000);

#ifdef ESP8266
  Serial.begin(115200);
#endif

  INFO("Starting Aurora Node \n");
  strip = Strip::getInstance(NUM_LEDS);
  // strip->setMaxBrightness(2);
  pinMode(PAIR_BTN, INPUT_PULLUP);
  espNow = ESPNowWrapper::getInstance();
  espNow->begin();
  espNow->registerHandler(MSG_TYPE::MSG_NODE_CMD, auroraHndlr);

  strip->test();
  strip->off();

  espNow->requestToPair();
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
