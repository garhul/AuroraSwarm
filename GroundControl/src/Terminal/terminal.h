#pragma once
#include <Arduino.h>
#include "handlers.cpp"
#include "EspNow/wrapper.h"

#define BUFFER_SIZE 64
#define DELIMITER '\n'

typedef void (*CommandHandler)(uint8_t argc, char* args[BUFFER_SIZE]);

enum Commands {
  CMD_RESTART,
  CMD_DEVICE_UPDATE,  // request device update, label/name device
  CMD_DEVICE_RM,      // clear devices ,remove device
  CMD_DEVICE_LS,      // list devices
  CMD_DEVICE_ADD,     // Add device
  CMD_DEVICE_CALL,    // send message to device
  CMD_ROUTE_ADD,      // Add a routing rule
  CMD_ROUTE_LS,       // list routing rules
  CMD_ROUTE_RM,       // remove routing rule
  CMD_HELP,           // print help 
  CMD_CFG_EXPORT,      // print config (rules, devices) ?
  CMD_CFG_SET,        // set config from input (rules, devices? what else)
  CMD_COUNT
};

class Terminal {
  private:
  ESPNowWrapper* espNow;
  CommandHandler handlers[CMD_COUNT];
  char buffer[BUFFER_SIZE];
  char delimiter = DELIMITER;
  void parseCommand();
  void handleCommand(uint8_t cmdType, uint8_t argc, char* argv[BUFFER_SIZE]);
  void printHelp(String cmdToken);
  void bindHandlers();

  public:
  Terminal();
  void poll();
  void attachHandler(Commands type, CommandHandler handler);
};