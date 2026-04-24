#include "terminal.h"

Terminal::Terminal() {
  memset(&buffer, 0, BUFFER_SIZE);

  for (uint8_t i = 0; i < CMD_COUNT; i++) {
    this->handlers[i] = nullptr;
  }

  this->bindHandlers();
};

void Terminal::printHelp(String cmdToken) {
  if (cmdToken == "device") {
    Serial.println("[ HELP ] DEVICE:");
    Serial.println("  - device ls                               Lists all registered devices");
    Serial.println("  - device rm -i <DEVICE_INDEX>             Removes a device by slot index");
    Serial.println("  - device rm -m <MAC_ADDRESS>              Removes a device by MAC address");
    Serial.println("  - device add <MAC_ADDRESS> <NAME>         Adds a device (max name: 26 chars)");
    Serial.println("  - device call -i <DEVICE_INDEX> <PAYLOAD> Calls device at index with payload");
    Serial.println("  - device call -a <PAYLOAD>                Calls all devices with payload");
    Serial.println("  - device update -i <DEVICE_INDEX>         Sends an OTA Update message to a device");
    Serial.println();

  }
  else if (cmdToken == "route") {
    Serial.println("[ HELP ] ROUTE:");
    Serial.println("  - route ls                                Lists all routing rules");
    Serial.println("  - route rm <ROUTE_INDEX>                  Removes a routing rule by index");
    Serial.println("  - route add <INBOUND_MAC> <OUTBOUND_MAC>  Adds a routing rule");
    Serial.println();

  }
  else if (cmdToken == "reboot") {
    Serial.println("[ HELP ] REBOOT:");
    Serial.println("  - reboot                          Reboots the device");
    Serial.println();

  }
  else {
    Serial.println("[ HELP ] - Available commands:");
    Serial.println("  - device                           Device management commands");
    Serial.println("  - route                            Route management commands");
    Serial.println("  - reboot                           Reboot the device");
    Serial.println();
    Serial.println("Type <COMMAND> for more details on usage");
  }
}

void Terminal::parseCommand() {
  char* tokens[BUFFER_SIZE];
  Serial.printf("[ DEBUG ] - Received %s \n", String(buffer));

  uint8_t tokensCount = 0;
  char* token = strtok(buffer, " ");

  while (token != NULL) {
    tokens[tokensCount] = token;
    Serial.printf("[ DEBUG ] - Token [%d]:%s \n", tokensCount, String(token));
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
  uint8_t cmdType = 99;

  if (cmdToken == "device") {
    if (actionToken == "ls") {
      cmdType = CMD_DEVICE_LS;
    }
    else if (actionToken == "rm") {
      cmdType = CMD_DEVICE_RM;
    }
    else if (actionToken == "add") {
      cmdType = CMD_DEVICE_ADD;
    }
    else if (actionToken == "call") {
      cmdType = CMD_DEVICE_CALL;
    }
    else if (actionToken == "update") {
      cmdType = CMD_DEVICE_UPDATE;
    }
    else {
      Serial.printf("[ ERROR ] Action [ %s ] not recognized. \n", String(actionToken));
      printHelp(cmdToken);
      return;
    }
  }
  else if (cmdToken == "route") {
    if (actionToken == "ls") {
      cmdType = CMD_ROUTE_LS;
    }
    else if (actionToken == "rm") {
      cmdType = CMD_ROUTE_RM;
    }
    else if (actionToken == "add") {
      cmdType = CMD_ROUTE_ADD;
    }
    else {
      Serial.printf("[ ERROR ] Action [%s] not recognized. \n", String(actionToken));
      printHelp(cmdToken);
      return;
    }
  }
  else if (cmdToken == "reboot") {
    Serial.println("[ INFO ] reboot requested");
  }
  else {
    Serial.println("[ ERROR ] Command not recognized");
    printHelp("");
    return;
  }

  handleCommand(cmdType, tokensCount, tokens);

}

void Terminal::handleCommand(uint8_t cmdType, uint8_t argc, char* argv[BUFFER_SIZE]) {
  Serial.printf("[ DEBUG ] Handling command %d, arguments: %d \n", cmdType, argc);

  if (this->handlers[cmdType] != nullptr) {
    this->handlers[cmdType](argc, argv);
  }
  else {
    Serial.printf("[ ERROR ] - No handler registered for cmdType %d \n", cmdType);
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

void deviceListHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
  espNow->listDevices();
}

void deviceRemoveHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
  if (argc != 4) {
    Serial.println("[ ERROR ] - Unercognized command format too few arguments, use either");
    Serial.println("        device rm -i INDEX");
    Serial.println("        or ");
    Serial.println("        device rm -m MAC_ADDRESS");
  }

  String mode = String(args[2]);
  mode.toLowerCase();

  // device rm -m MAC_ADDR
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

    espNow->removeDevice(macAddr);
    return;
  }


  if (mode == "-i") {
    uint8_t index = atoi(args[3]);
    Serial.printf("[ INFO ] - Removing device at slot %d \n", index);
    espNow->removeDevice(index);
    return;
  }
}

void deviceAddHandler(uint8_t argc, char* args[BUFFER_SIZE]) {

  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
  // device add MAC_ADDR NAME

  if (argc < 4) {
    Serial.println("[ ERROR ] - invalid input arguments");
    Serial.println("   Usage: device add MAC_ADDR NAME");
  }

  // TODO, validate mac address?
  Device* d = new Device();

  uint8_t macAddr[6];
  if (sscanf(args[2], "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
    &macAddr[0], &macAddr[1], &macAddr[2],
    &macAddr[3], &macAddr[4], &macAddr[5]) != 6) {
    Serial.println("[ ERROR ] - Invalid MAC address format");
    return;
  }

  memcpy(d->macAddress, macAddr, 6);
  memcpy(d->name, args[3], 26);

  if (espNow->addDevice(d)) {
    Serial.println("OK");
  }
}

void deviceCallHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  ESPNowWrapper* espNow = ESPNowWrapper::getInstance();
 //command example
 // device call -i 3 FX 3 10
 // device call -i 3 BR 35
 // device call -i 3 STOP
 // device call -i 3 OFF

  if (argc < 4) {
    Serial.printf("[ ERROR ] - Invalid argument count, expected more than 4 received %d \n", argc);
    return;
  }

  String mode = String(args[2]);
  mode.trim();
  mode.toLowerCase();

  String payload;

  for (uint8_t i = 4; i < argc; i++) {
    payload += String(args[i]) + " ";
  }

  payload.trim();
  Serial.print("[ DEBUG ] - Payload: ");
  Serial.println(payload);

  if (mode == "-i") {
    espNow->sendToDevice((uint8_t)atoi(args[3]), (const uint8_t*)payload.c_str(), (const uint8_t)payload.length());
  }
}

void systemRestartHandler(uint8_t argc, char* args[BUFFER_SIZE]) {
  Serial.println("[ INFO ] Restarting system...");
  ESP.restart();
}


void Terminal::bindHandlers() {
  attachHandler(CMD_DEVICE_LS, deviceListHandler);
  attachHandler(CMD_DEVICE_RM, deviceRemoveHandler);
  attachHandler(CMD_DEVICE_ADD, deviceAddHandler);
  attachHandler(CMD_DEVICE_CALL, deviceCallHandler);


  //Routes are not implemented for now



  attachHandler(CMD_RESTART, systemRestartHandler);
}