#pragma once
#include <Arduino.h>
#include "EspNowWrapper.h"

#define BUFFER_SIZE 64
#define DELIMITER '\n'


#define NODE_CMD_TOKEN "node"
#define NODE_CMD_ADD_TOKEN "add"
#define NODE_CMD_LS_TOKEN "ls"
#define NODE_CMD_MV_TOKEN "mv"
#define NODE_CMD_RM_TOKEN "rm"
#define NODE_CMD_SEND_TOKEN "send" 
#define NODE_CMD_UPDATE_TOKEN "update"


#define ROUTE_CMD_TOKEN "route"

#define SYSTEM_CMD_TOKEN "system"
#define SYSTEM_RESTART_TOKEN "reboot"
#define SYSTEM_UPDATE_TOKEN "update"

typedef void (*CommandHandler)(uint8_t argc, char* args[BUFFER_SIZE]);

enum Commands {
  /** System specific commands, only applicable to current node */
  CMD_SYSTEM_RESTART,
  CMD_SYSTEM_UPDATE,
  CMD_SYSTEM_CONFIG_SET,
  CMD_SYSTEM_CONFIG_GET,
  CMD_SYSTEM_INFO,
  CMD_SYSTEM_CFG_DUMP,
  CMD_SYSTEM_CFG_CLEAR,
  CMD_SYSTEM_CFG_IMPORT,          // set config from input (rules, nodes? what else)

  /* Node specific commands, applicable to registered nodes */
  CMD_NODE_ADD,         // Add NODE
  CMD_NODE_LS,          // list NODEs
  CMD_NODE_MV,          // rename node
  CMD_NODE_RM,          // clear NODEs ,remove NODE
  CMD_NODE_SEND,        // send message to NODE
  CMD_NODE_SYS_UPDATE,   // request node OTA update

  /* Routing specific commands, not implemented yet */
  CMD_ROUTE_ADD,        // Add a routing rule
  CMD_ROUTE_LS,         // list routing rules
  CMD_ROUTE_RM,         // remove routing rule
  CMD_ROUTE_UPDATE,     // update routing rule

  /* General purpose commands */
  CMD_HELP,             // print help 

  //placehodler
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