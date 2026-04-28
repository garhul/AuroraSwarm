#include "EspNowWrapper.h"
ESPNowWrapper* ESPNowWrapper::instance = nullptr;

void debugPrint(uint8_t* data, size_t len) {
  Serial.printf("[ DEBUG ] - Data (%d bytes): ", len);

  for (size_t i = 0; i < len; i++) {
    Serial.printf("%02x ", data[i]);
  }
  Serial.println("");
};

/** Iterates through all devices in the list and prints out the macAddress value */
void listAllDevices(Device* d[MAX_NODES]) {
  Serial.println("[ INFO ] - Listing all devices:");
  for (int i = 0; i < MAX_NODES; i++) {
    if (d[i] != nullptr) {
      Serial.printf("  Device %d: [%02x:%02x:%02x:%02x:%02x:%02x]\n",
        i,
        d[i]->macAddress[0], d[i]->macAddress[1], d[i]->macAddress[2],
        d[i]->macAddress[3], d[i]->macAddress[4], d[i]->macAddress[5]);
    }
  }
}

ESPNowWrapper* ESPNowWrapper::getInstance() {
  if (instance == nullptr) {
    new ESPNowWrapper(AS_BROKER);
  }

  return instance;
}

ESPNowWrapper::ESPNowWrapper(bool isBroker) {
  instance = this;
  instance->isBroker = isBroker;
}

void ESPNowWrapper::pairRequestHandler(uint8_t* macAddr, uint8_t* message, uint8_t len) {
  Serial.printf("[ INFO ] - Pairing request received size[%d] \n", len);
  Serial.printf("  Payload: addr[%02x:%02x:%02x:%02x:%02x:%02x] \n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  if (!instance->autoPair) {
    Serial.println("Request rejected, autopairing is disabled");
    return;
  }

  Message<PairRequestPayload> msg;
  memset(&msg, 0, sizeof(Message<PairRequestPayload>));
  memcpy(&msg, message, sizeof(Message<PairRequestPayload>));
  Serial.printf("  Payload: name[%s] \n", msg.payload.deviceName);

  Device d;
  memset(&d, 0, sizeof(Device));
  memcpy(&d.macAddress, macAddr, 6);
  memcpy(&d.name, msg.payload.deviceName, sizeof(d.name));

  if (!instance->addDevice(&d)) {
    return;
  };

  Message<PairOkPayload> reqOkMsg;
  reqOkMsg.msgType = MSG_PAIR_OK;
  memset(&reqOkMsg.payload, 0, sizeof(reqOkMsg.payload));
  strncpy((char*)&reqOkMsg.payload.data, "OK", sizeof(reqOkMsg.payload.data));

  instance->sendToDevice(d, (uint8_t*)&reqOkMsg, sizeof(Message<PairOkPayload>));
  return;
}

void ESPNowWrapper::registerHandler(MSG_TYPE id, MessageHandler handler) {
  this->handlers[id] = handler;
}

void ESPNowWrapper::pairOkHandler(uint8_t* macAddr, uint8_t* msg, uint8_t len) {
  Serial.println("[ INFO ] - Pairing OK received");
  Serial.printf("  From: [%02x:%02x:%02x:%02x:%02x:%02x] \n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  if (instance->pairReqTTL < millis()) {
    Serial.println("Request rejected, pairing window expired");
    return;
  }

  Serial.println("[ INFO ] - Adding broker");

  Device* broker = new Device();
  memcpy(broker->macAddress, macAddr, 6);
  memcpy(broker->name, "Broker", 7);
  instance->addDevice(broker, true);
}

void ESPNowWrapper::addPeer(const uint8_t* macAddr) {
#ifdef ESP32

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, macAddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("[ ERROR ] - Failed to add peer");
  }

#elif defined(ESP8266)
  esp_now_add_peer((uint8_t*)macAddr, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
#endif

}

void ESPNowWrapper::begin() {
  Serial.printf("broker - > %d", instance->isBroker);
  if (instance == nullptr) {
    Serial.println("[ ERROR ] no instance!");
    return;
  }


  instance->prefs.begin("broker_prefs", false);
  uint32_t boot = instance->prefs.getUInt("boot", 0);
  boot++;
  instance->prefs.putUInt("boot", boot);
  instance->prefs.end();

  Serial.printf("[ INFO ] - Boot count: %d\n", boot);

  autoPair = false;
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.println("[ INFO ] - ESP-NOW initialized");
  uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  addPeer(broadcastAddress);

  Serial.println("[ INFO ] - Broadcast peer added");

#ifdef ESP32

  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);

  if (ret == ESP_OK) {
    Serial.printf("[ INFO ] - MAC Address: [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    Serial.println("[ ERROR ] - Failed to read MAC address");
  }

#elif defined(ESP8266)
  Serial.printf("[ INFO ] - local mac: %s\n", String(WiFi.macAddress()).c_str());
#endif

  Serial.printf("[ INFO ] - Wi-Fi Channel: %d\n", WiFi.channel());

#ifdef ESP32
  esp_now_register_recv_cb((esp_now_recv_cb_t)&ESPNowWrapper::onMessage);
#elif defined(ESP8266)
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(&ESPNowWrapper::onMessage);
#endif

  registerHandler(MSG_PAIR_REQUEST, &ESPNowWrapper::pairRequestHandler);

  if (!isBroker) {
    registerHandler(MSG_PAIR_OK, &ESPNowWrapper::pairOkHandler);
  }

  loadDevices();

}

void ESPNowWrapper::onMessage(uint8_t* macAddr, uint8_t* msg, uint8_t len) {
  if (instance->handlers[msg[0]] == nullptr) {
    Serial.printf("[ WARN ] No handler registered for msgType: %d \n", msg[0]);
    return;
  }

  if (msg[0] != MSG_PAIR_REQUEST && msg[0] != MSG_PAIR_OK && instance->getDevice(macAddr) == nullptr) {
    Serial.printf("[ WARN ] - Ignoring message from unknown peer [%02x:%02x:%02x:%02x:%02x:%02x]\n",
      macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);
    return;
  }

  // Guard for payload sizing (to prevent crashes)
  bool oversized = false;
  switch (msg[0]) {
    case MSG_PAIR_REQUEST:
      oversized = (len > sizeof(PairRequestPayload) + 1);
      break;

    case MSG_PAIR_OK:
      oversized = (len > sizeof(PairOkPayload) + 1);
      break;

    case MSG_STATE_UPDATE:
      oversized = (len > sizeof(BasePayload) + 1);
      break;

    default:
      oversized = (len > sizeof(BasePayload) + 1);
  }

  if (oversized) {
    Serial.printf("[ WARNING ] - Msg (%d) payload size (%d)is larger than expected, dropping message \n", msg[0], len);
    return;
  }

  if (instance->handlers[msg[0]] == nullptr) {
    Serial.printf("[ WARNING ] - Unregistered handler for message type [%d] \n", msg[0]);
    return;
  }

  instance->handlers[msg[0]](macAddr, msg, len);
}

bool ESPNowWrapper::addDevice(const Device* device, bool asBroker) {
  uint8_t idx = 0;
  Serial.printf("[ INFO ] - Adding device: %s\n", device->name);

  if (!asBroker) {
    if (getDevice(device->macAddress) != nullptr) {
      Serial.println("[ INFO ] - Device already registered");
      return true;
    }

    // Add the device to the list, device position 0 is reserved for broker
    for (int i = 1; i < MAX_NODES; i++) {
      if (instance->devices[i] == nullptr) {
        idx = i;
        break;
      }
    }

    if (idx == 0) {
      Serial.println("[ ERROR ] - Unable to add Device, list is full");
      return false;
    }
  } else {
    Serial.printf("[ INFO ] - Adding new Broker [%02x:%02x:%02x:%02x:%02x:%02x]\n",
          device->macAddress[0],
          device->macAddress[1],
          device->macAddress[2],
          device->macAddress[3],
          device->macAddress[4],
          device->macAddress[5]
    );
  }

  Serial.printf("[ INFO ] - Adding new Peer [%02x:%02x:%02x:%02x:%02x:%02x]\n",
          device->macAddress[0],
          device->macAddress[1],
          device->macAddress[2],
          device->macAddress[3],
          device->macAddress[4],
          device->macAddress[5]
  );

  if (instance->devices[idx] != nullptr) {
    delete instance->devices[idx];
  }

  instance->devices[idx] = (Device*)device;

  instance->prefs.begin("devices", false);
  String key = String("slot_" + String(idx)).c_str();
  Serial.printf("[ DEBUG ] - Storing under key %s \n", key.c_str());
  instance->prefs.putBytes(key.c_str(), (uint8_t*)device, sizeof(Device));
  instance->prefs.end();

  instance->addPeer(device->macAddress);
  return true;
}

bool ESPNowWrapper::removeDevice(const uint8_t macAddres[6]) {
  for (int i = 0; i < MAX_NODES; i++) {
    if (devices[i] != nullptr && memcmp(instance->devices[i]->macAddress, macAddres, 6) == 0) {
      return instance->removeDevice(i);
    }
  }
  Serial.println("[ ERROR ] - Device not found");
  return false;
}

bool ESPNowWrapper::removeDevice(uint8_t idx) {

  if (idx >= MAX_NODES) {
    Serial.printf("[ ERROR ] - Index[%d] out of bounds trying to remove device \n", idx);
  }

  if (instance->devices[idx] != nullptr) {
    if (esp_now_del_peer((uint8_t*)(devices[idx]->macAddress)) != ESP_OK) {
      Serial.printf("[ ERROR ] - Failed to remove peer [%02x:%02x:%02x:%02x:%02x:%02x]\n",
        devices[idx]->macAddress[0], devices[idx]->macAddress[1], devices[idx]->macAddress[2],
        devices[idx]->macAddress[3], devices[idx]->macAddress[4], devices[idx]->macAddress[5]);
      return false;
    }

    instance->prefs.begin("devices", false);
    if (!instance->prefs.remove(String("slot_" + String(idx)).c_str())) {
      Serial.println("[ ERROR ] - Error deleting device key from flash ram");
      instance->prefs.end();
      return false;
    };

    instance->prefs.end();
    delete devices[idx];
    devices[idx] = nullptr;
    Serial.printf("[ INFO ] - Device removed and preferences updated.\n");
    return true;
  }

  Serial.printf("[ ERROR ] - Failed to remove peer [%02x:%02x:%02x:%02x:%02x:%02x] NOT FOUND \n",
          devices[idx]->macAddress[0], devices[idx]->macAddress[1], devices[idx]->macAddress[2],
          devices[idx]->macAddress[3], devices[idx]->macAddress[4], devices[idx]->macAddress[5]);
  return false;
}

void ESPNowWrapper::storeDevices() {
  instance->prefs.begin("devices", false);

  for (int i = 0; i < MAX_NODES; i++) {
    if (devices[i] != nullptr) {
      instance->prefs.putBytes(String("slot_" + String(i)).c_str(), (uint8_t*)devices[i], sizeof(Device));
    }
  }

  instance->prefs.end();
  Serial.println("[ INFO ] - Devices stored in flash.");
}

void ESPNowWrapper::loadDevices() {
  instance->prefs.begin("devices", false);

  for (int i = 0; i < MAX_NODES; i++) {
    String id = String("slot_" + String(i));
    Serial.printf("[ DEBUG ] - Loading %s... ", id.c_str());

    if (!instance->prefs.isKey(id.c_str())) {
      Serial.printf(" NOT FOUND \n");
      continue;
    }

    Serial.print(" FOUND ");

    uint8_t buffer[sizeof(Device)];

    // Device* device;
    instance->prefs.getBytes(id.c_str(), &buffer, sizeof(Device));

    instance->devices[i] = new Device();
    memcpy(&instance->devices[i]->macAddress, &buffer, 6);
    memcpy(&instance->devices[i]->name, &buffer[6], sizeof(Device) - 6);

    Serial.printf("  MAC [%02x:%02x:%02x:%02x:%02x:%02x] ",
        devices[i]->macAddress[0], devices[i]->macAddress[1], devices[i]->macAddress[2],
        devices[i]->macAddress[3], devices[i]->macAddress[4], devices[i]->macAddress[5]);
    Serial.printf("  NAME [%s]\n", devices[i]->name);
    instance->addPeer(devices[i]->macAddress);
  }

  instance->prefs.end();
}

Device* ESPNowWrapper::getDevice(const uint8_t macAddr[6]) {
  for (int i = 0; i < MAX_NODES; i++) {
    if (devices[i] != nullptr && memcmp(instance->devices[i]->macAddress, macAddr, 6) == 0) {
      return instance->devices[i];
    }
  }
  return nullptr;
};

Device** ESPNowWrapper::getDevices() {
  return devices;
}

bool ESPNowWrapper::sendToDevice(const Device& device, const uint8_t* message, const uint8_t len) {
  Serial.printf("[ INFO ] - Sending message to device: %s\n", device.name);

  int result = esp_now_send((uint8_t*)device.macAddress, (uint8_t*)message, len);

  if (result == ESP_OK) {
    Serial.println("[ INFO ] - Message sent successfully");
    return true;
  } else {
    Serial.printf("[ ERROR ] - Error sending message %d\n", result);
    return false;
  }
}

bool ESPNowWrapper::sendToDevice(uint8_t index, const uint8_t* message, const uint8_t len) {
  if (index >= MAX_NODES) {
    Serial.printf("[ ERROR ] - Slot [%d] is out of bounds \n", index);
    return false;
  }

  return sendToDevice(*devices[index], message, len);
}

void ESPNowWrapper::sendToAll(const uint8_t* message, const uint8_t len) {
  Serial.println("[ INFO ] - Sending message to all devices");
  for (int i = 0; i < MAX_NODES; i++) {
    if (devices[i] != nullptr) {
      sendToDevice(*devices[i], message, len);
    }
  }
}

void ESPNowWrapper::broadcast(const uint8_t* message, const uint8_t len) {
  Serial.println("[ INFO ] - Broadcasting message: " + String((char*)message));
  uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  int result = esp_now_send(broadcastAddress, (uint8_t*)message, len);
  if (result == ESP_OK) {
    Serial.println("[ INFO ] - Broadcast sent successfully");
  } else {
    Serial.printf("[ ERROR ] - Error sending broadcast %d\n", result);
  }
};

void ESPNowWrapper::setAutoPair(bool enable) {
  autoPair = enable;
}

void ESPNowWrapper::requestToPair() {
  pairReqTTL = millis() + PAIR_REQ_TTL;
  Message<PairRequestPayload> m;
  m.msgType = MSG_PAIR_REQUEST;
  memset(&m.payload, 0, sizeof(m.payload));
  strncpy((char*)&m.payload, "Vivan las tetas", sizeof(m.payload) - 1);
  broadcast((uint8_t*)&m, sizeof(Message<PairRequestPayload>));
}