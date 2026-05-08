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
  Device* devices[MAX_NODES] = {};
  MessageHandler handlers[(uint8_t)MSG_TYPE::MSG_TYPE_COUNT];
  bool autoPair;
  bool isBroker;
  Preferences prefs;
  uint64_t pairReqTTL = 0;

  void printMac(const uint8_t* macAddr);
  static void onMessage(uint8_t* macAddr, uint8_t* msg, uint8_t len);
  static void pairRequestHandler(uint8_t* macAddr, uint8_t* onMessage, uint8_t len);
  static void pairOkHandler(uint8_t* macAddr, uint8_t* msg, uint8_t len);

  void loadDevices();
  void storeDevices();
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

  // Device management methods
  void listDevices();

  /** Adds a new device to the list of registered devices */
  bool addDevice(const Device* device, bool asBroker = false);
  bool addDevice(uint8_t macAddr[6], uint8_t name[26], bool asBroker = false);

  bool removeDevice(const uint8_t macAddr[6]);
  bool removeDevice(const uint8_t index);

  bool updateDevice(uint8_t index, const Device* device);
  // bool callDevice(uint8_t macAddr[6], uint8_t* payload, uint8_t len);
  bool sendToDevice(const Device* device, const uint8_t* msg, uint8_t len);
  bool sendToDevice(uint8_t index, const uint8_t* msg, uint8_t len);

  Device* getDevice(const uint8_t macAddr[6]);
  Device** getDevices();
};