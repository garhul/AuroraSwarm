#include "terminal.h"

Terminal::Terminal() {
  memset(&buffer, 0, BUFFER_SIZE);

  for (uint8_t i = 0; i < CMD_COUNT; i++) {
    this->handlers[i] = nullptr;
  }

  this->bindHandlers();
};

void Terminal::printHelp(String cmdToken) {
  if (cmdToken == NODE_CMD_TOKEN) {
    Serial.println("[ HELP ] NODE:");
    Serial.println("  - node ls                                   Lists all registered nodes");
    Serial.println("  - node rm -i <NODE_INDEX>                   Removes a node by slot index");
    Serial.println("  - node rm -m <MAC_ADDRESS>                  Removes a node by MAC address");
    Serial.println("  - node add <MAC_ADDRESS> <NAME>             Adds a node (max len: 32 chars)");
    Serial.println("  - node send -i <NODE_INDEX> <PAYLOAD>       Calls node at index with payload");
    Serial.println("  - node send -l <NODE_INDEX_LIST> <PAYLOAD>  Calls node at index with payload");
    Serial.println("  - node send -a <PAYLOAD>                    Broadcasts to all nodes with payload");
    Serial.println("  - node update -i <NODE_INDEX>               Sends an OTA Update message to a node");
    Serial.println();

  } else if (cmdToken == ROUTE_CMD_TOKEN) {
    Serial.println("[ HELP ] ROUTE: *** NOT IMPLEMENTED *** ");
    Serial.println("  - route ls                                Lists all routing rules");
    Serial.println("  - route rm <ROUTE_INDEX>                  Removes a routing rule by index");
    Serial.println("  - route add <INBOUND_MAC> <OUTBOUND_MAC>  Adds a routing rule");
    Serial.println();

  } else if (cmdToken == SYSTEM_CMD_TOKEN) {
    Serial.println("[ HELP ] REBOOT:");
    Serial.println("  - reboot                          Reboots the node");
    Serial.println();

  } else {
    Serial.println("[ HELP ] - Available commands:");
    Serial.println("  - node                             Node management commands");
    Serial.println("  - route                            Route management commands");
    Serial.println("  - system                           System management commands");
    Serial.println();
    Serial.println("Type <COMMAND> for more details on usage");
  }
}

inline uint8_t getCmdType(String cmdToken, String actionToken) {
  uint8_t cmdType = 99;

  if (cmdToken == NODE_CMD_TOKEN) {
    if (actionToken == NODE_CMD_LS_TOKEN) {
      cmdType = CMD_NODE_LS;
    } else if (actionToken == NODE_CMD_RM_TOKEN) {
      cmdType = CMD_NODE_RM;
    } else if (actionToken == NODE_CMD_ADD_TOKEN) {
      cmdType = CMD_NODE_ADD;
    } else if (actionToken == NODE_CMD_SEND_TOKEN) {
      cmdType = CMD_NODE_SEND;
    } else if (actionToken == NODE_CMD_UPDATE_TOKEN) {
      cmdType = CMD_NODE_SYS_UPDATE;
    } else {
      Serial.printf("[ ERROR ] Action [ %s ] not recognized. \n", String(actionToken));
    }
  } else if (cmdToken == ROUTE_CMD_TOKEN) {
    if (actionToken == "ls") {
      cmdType = CMD_ROUTE_LS;
    } else if (actionToken == "rm") {
      cmdType = CMD_ROUTE_RM;
    } else if (actionToken == "add") {
      cmdType = CMD_ROUTE_ADD;
    } else {
      Serial.printf("[ ERROR ] Action [%s] not recognized. \n", String(actionToken));
    }
  } else if (cmdToken == SYSTEM_CMD_TOKEN) {
    if (actionToken == SYSTEM_RESTART_TOKEN) {
      cmdType = CMD_SYSTEM_RESTART;
    } else if (actionToken == SYSTEM_UPDATE_TOKEN) {
      cmdType = CMD_SYSTEM_UPDATE;
    }
    // else if (cmdToken == "config") {
    //   Serial.println("[ INFO ] config requested");
    // }
  }

  else {
    Serial.println("[ ERROR ] Command not recognized");
  }

  return cmdType;
}

void Terminal::parseCommand() {
  char* tokens[BUFFER_SIZE];
  // Serial.printf("[ DEBUG ] - Received %s \n", String(buffer));

  uint8_t tokensCount = 0;
  char* token = strtok(buffer, " ");

  while (token != NULL) {
    tokens[tokensCount] = token;
    DEBUG(" Token [%d]:%s \n", tokensCount, String(token));
    token = strtok(NULL, " ");
    tokensCount++;
  }

  //remove line jump from last token
  String lt = String(tokens[tokensCount - 1]);
  lt.trim();
  tokens[tokensCount - 1] = (char*)lt.c_str();

  String cmdToken = String(tokens[0]);
  String actionToken = (tokensCount > 1) ? String(tokens[1]) : String("");
  actionToken.toLowerCase();
  cmdToken.toLowerCase();

  uint8_t cmdType = getCmdType(cmdToken, actionToken);

  if (cmdType == 99) {
    printHelp(cmdToken);
    return;
  }

  handleCommand(cmdType, tokensCount, tokens);

}

void Terminal::handleCommand(uint8_t cmdType, uint8_t argc, char* argv[BUFFER_SIZE]) {
  DEBUG("Handling command %d, arguments: %d \n", cmdType, argc);

  if (this->handlers[cmdType] != nullptr) {
    this->handlers[cmdType](argc, argv);
  } else {
    ERROR("No handler registered for cmdType %d \n", cmdType);
  }

}

void Terminal::attachHandler(Commands cmdType, CommandHandler hndlr) {
  if (cmdType >= CMD_COUNT) {
    Serial.printf("[ ERROR ] - Attaching command handler, command %d is out of bounds \n", cmdType);
  }

  this->handlers[cmdType] = hndlr;

  Serial.printf(" [ INFO ] - Handler %d attached \n", cmdType);

}

void Terminal::poll() {
  static uint8_t idx = 0;

  while (Serial.available() > 0) {
    buffer[idx] = Serial.read();
    // Serial.print(buffer[idx]);

    // Serial.printf("[ DEBUG ] - Buffer -> %02x delimiter -> %02x \n", buffer[idx], this->delimiter);

    if (buffer[idx] == this->delimiter || idx == (BUFFER_SIZE - 1)) {
      // Serial.println(buffer);
      parseCommand();
      idx = 0;
      memset(&buffer, 0, BUFFER_SIZE);
      return;
    }

    idx++;
  }
}

/** HANDLERS FOR TERMINAL COMMANDS */
void nodesListHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
  espNow->listNodes();
}

void nodesRemoveHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
  if (argc != 4) {
    Serial.println("[ ERROR ] - Unercognized command format too few arguments, use either");
    Serial.println("        node rm -i INDEX");
    Serial.println("        or ");
    Serial.println("        node rm -m MAC_ADDRESS");
    return;
  }

  String mode = String(args[2]);
  mode.toLowerCase();

  // node rm -m MAC_ADDR
  if (mode == "-m") {
    // parse transform mac into 6 bytes
    // from text FF:DC:AA:00:1B:B0
    uint8_t macAddr[6];
    if (sscanf(args[3], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
      &macAddr[0], &macAddr[1], &macAddr[2],
      &macAddr[3], &macAddr[4], &macAddr[5]) != 6) {
      Serial.println("[ ERROR ] - Invalid MAC address format");
      return;
    }

    espNow->removeNode(macAddr);
    return;
  }


  if (mode == "-i") {
    uint8_t index = atoi(args[3]);
    Serial.printf("[ INFO ] - Removing node at slot %d \n", index);
    espNow->removeNode(index);
    return;
  }
}

void nodesAddHandler(uint8_t argc, char* args[BUFFER_SIZE]) {

  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
  // node add MAC_ADDR NAME

  if (argc < 4) {
    Serial.println("[ ERROR ] - invalid input arguments");
    Serial.println("   Usage: node add MAC_ADDR NAME");
    return;
  }

  uint8_t macAddr[6];
  if (sscanf(args[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
    &macAddr[0], &macAddr[1], &macAddr[2],
    &macAddr[3], &macAddr[4], &macAddr[5]) != 6) {
    Serial.println("[ ERROR ] - Invalid MAC address format");
    return;
  }

  Node* d = new Node();

  memcpy(d->macAddress, macAddr, 6);
  memcpy(d->name, args[3], 26);

  if (espNow->addNode(d)) {
    Serial.println("[ INFO ] - OK");
  }
}

void nodeSendHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();

  if (argc != 4 && argc != 5) {
    Serial.printf("[ ERROR ] - Invalid argument count, expected 5 or 4received %d \n", argc);
    return;
  }

  String mode = String(args[2]);
  mode.trim();
  mode.toLowerCase();

  String payload = String((char)MSG_TYPE::MSG_NODE_CMD);
  for (uint8_t i = 4; i < argc; i++) {
    payload += String(args[i]) + " ";
  }
  payload.trim();

  DEBUG("MODE :%s , PAYLOAD,:%s", mode, String(payload));

  if (mode == "-i") {
    DEBUG("Sending Payload: %s, to nodes %s \n", String(payload).c_str(), args[3]);

    char* idx = strtok(args[3], ":");
    while (idx != NULL) {
      espNow->sendToNode((uint8_t)atoi(idx), (const uint8_t*)payload.c_str(), (const uint8_t)payload.length());
      idx = strtok(NULL, ":");
    }

  } else if (mode == "-a") {
    espNow->broadcast((const uint8_t*)payload.c_str(), (const uint8_t)payload.length());
  } else {
    Serial.println("[ ERROR ] - Invalid mode, expected -i or -a");
  }
}

void systemRestartHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  Serial.println("[ INFO ] Restarting system...");
  ESP.restart();
}

void Terminal::bindHandlers() {
  /** System command handlers */
  attachHandler(CMD_SYSTEM_RESTART, systemRestartHandler);
  // attachHandler(CMD_SYSTEM_UPDATE, systemUpdateHandler);
  // attachHandler(CMD_SYSTEM_CONFIG_SET, systemConfigSetHandler);
  // attachHandler(CMD_SYSTEM_CONFIG_GET, systemConfigGetHandler);
  // attachHandler(CMD_SYSTEM_CFG_DUMP, systemConfigDumpHandler);
  // attachHandler(CMD_SYSTEM_CFG_CLEAR, systemConfigClearHandler);


  /** Node command handlers */
  attachHandler(CMD_NODE_LS, nodesListHandler);
  attachHandler(CMD_NODE_RM, nodesRemoveHandler);
  attachHandler(CMD_NODE_ADD, nodesAddHandler);
  attachHandler(CMD_NODE_SEND, nodeSendHandler);



  //Routes are not implemented for now





}