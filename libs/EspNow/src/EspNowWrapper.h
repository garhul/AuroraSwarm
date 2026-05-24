#pragma once

#include "types.h"
#include <Arduino.h>
#include "Logger.h"

#ifdef ESP32



#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_mac.h> 
#include <esp_now.h>

#elif defined(ESP8266)
#define ESP_OK 0
#include <ESP8266WiFi.h>
#include <espnow.h>

#endif


#include <Preferences.h>


#define MAX_NODES 16 // first peer is broadcast addr
#define ESPNOW_WIFI_CHANNEL 6
#define MAX_PAYLOAD_SIZE 128
#define PAIR_REQ_TTL 2000 // TTL of a pair request in ms
#define AS_BROKER false

typedef void (*MessageHandler)(uint8_t* macAddr, uint8_t* message, uint8_t len);

class ESPNowWrapper {
  private:
  static ESPNowWrapper* instance;
  Node* nodes[MAX_NODES] = {};
  MessageHandler handlers[(uint8_t)MSG_TYPE::MSG_TYPE_COUNT];
  bool autoPair;
  bool isBroker;
  Preferences prefs;
  uint64_t pairReqTTL = 0;

  void printMac(const uint8_t* macAddr);
  static void onMessage(uint8_t* macAddr, uint8_t* msg, uint8_t len);
  static void pairRequestHandler(uint8_t* macAddr, uint8_t* onMessage, uint8_t len);
  static void pairOkHandler(uint8_t* macAddr, uint8_t* msg, uint8_t len);

  void loadNodes();
  void storeNodes();
  void addPeer(const uint8_t macAddr[6]);
  ESPNowWrapper(bool asBroker);

  public:
  ESPNowWrapper(const ESPNowWrapper& obj) = delete;

  // Static method to get the Singleton instance
  static ESPNowWrapper* getInstance(bool as_broker);
  static ESPNowWrapper* getInstance();


  void begin();
  void broadcast(const uint8_t* msg, uint8_t len);
  void sendToAll(const uint8_t* msg, uint8_t len);
  void requestToPair();

  void registerHandler(MSG_TYPE id, MessageHandler handler);
  void unregisterHandler(MSG_TYPE id);
  void setAutoPair(bool enable);

  // Node management methods
  void listNodes();

  /** Adds a new node to the list of registered nodes */
  bool addNode(const Node* node, bool asBroker = false);
  bool addNode(uint8_t macAddr[6], uint8_t name[26], bool asBroker = false);

  bool removeNode(const uint8_t macAddr[6]);
  bool removeNode(const uint8_t index);

  bool updateNode(uint8_t index, const Node* node);
  // bool callNode(uint8_t macAddr[6], uint8_t* payload, uint8_t len);
  bool sendToNode(const Node* node, const uint8_t* msg, uint8_t len);
  bool sendToNode(uint8_t index, const uint8_t* msg, uint8_t len);

  Node* getNode(const uint8_t macAddr[6]);
  Node** getNode();
};