#include "EspNowWrapper.h"
ESPNowWrapper* ESPNowWrapper::instance = nullptr;

inline void debugPrint(uint8_t* data, size_t len) {
  DEBUG("Data (%d bytes): ", len);

  for (size_t i = 0; i < len; i++) {
    PRINTF("%02x ", data[i]);
  }
  PRINTF("\n");
};

/** Iterates through all devices in the list and prints out the macAddress value */
// void listAllDevices(Device* d[MAX_NODES]) {
//   INFO("Listing all devices:");
//   for (int i = 0; i < MAX_NODES; i++) {
//     if (d[i] != nullptr) {
//      PRINTF("  Device %d: [%02x:%02x:%02x:%02x:%02x:%02x]\n",
//         i,
//         d[i]->macAddress[0], d[i]->macAddress[1], d[i]->macAddress[2],
//         d[i]->macAddress[3], d[i]->macAddress[4], d[i]->macAddress[5]);
//     }
//   }
// }

#ifdef ESP32
void OnDataSent(const uint8_t* mac_addr, esp_now_send_status_t status) {
  DEBUG("SENT DATA: ");
  debugPrint((uint8_t*)mac_addr, 6);
  DEBUG("STATUS %s \n", String(status).c_str());
}
#endif

ESPNowWrapper* ESPNowWrapper::getInstance(bool as_broker = AS_BROKER) {
  if (instance == nullptr) {
    new ESPNowWrapper(as_broker);
  }

  return instance;
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
  INFO("Pairing request received size[%d] \n", len);
  PRINTF("  Payload: addr[%02x:%02x:%02x:%02x:%02x:%02x] \n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  if (!instance->autoPair) {
    PRINTF("Request rejected, autopairing is disabled \n");
    return;
  }

  Message<PairRequestPayload> msg;
  memset(&msg, 0, sizeof(Message<PairRequestPayload>));
  memcpy(&msg, message, sizeof(Message<PairRequestPayload>));
  PRINTF("  Payload: name[%s] \n", msg.payload.deviceName);

  Device* d = new Device();
  memcpy(d->macAddress, macAddr, 6);
  memcpy(d->name, msg.payload.deviceName, sizeof(d->name));

  if (!instance->addDevice(d)) {
    ERROR("Error adding device \n");
    return;
  };

  Message<PairOkPayload> reqOkMsg;
  reqOkMsg.msgType = (uint8_t)MSG_TYPE::MSG_PAIR_OK;
  memset(&reqOkMsg.payload, 0, sizeof(reqOkMsg.payload));
  strncpy((char*)&reqOkMsg.payload.data, "OK", sizeof(reqOkMsg.payload.data));

  PRINTF("SENDING DATA \n");
  instance->sendToDevice(d, (uint8_t*)&reqOkMsg, sizeof(Message<PairOkPayload>));
  return;
}

void ESPNowWrapper::registerHandler(MSG_TYPE id, MessageHandler handler) {
  this->handlers[(uint8_t)id] = handler;
}

void ESPNowWrapper::pairOkHandler(uint8_t* macAddr, uint8_t* msg, uint8_t len) {
  INFO("Pairing OK received");
  PRINTF("  From: [%02x:%02x:%02x:%02x:%02x:%02x] \n", macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5]);

  if (instance->pairReqTTL < millis()) {
    PRINTF("Request rejected, pairing window expired\n");
    return;
  }

  INFO("Adding broker \n");

  Device* broker = new Device();
  memcpy(broker->macAddress, macAddr, 6);
  memcpy(broker->name, "Broker", 7);
  instance->addDevice(broker, true);
}

void ESPNowWrapper::addPeer(const uint8_t macAddr[6]) {
#ifdef ESP32

  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
  memcpy(peerInfo.peer_addr, macAddr, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_is_peer_exist(peerInfo.peer_addr)) {
    WARN("Peer already known \n");
  } else  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    ERROR("Failed to add peer \n");
  }

#elif defined(ESP8266)
  esp_now_add_peer((uint8_t*)macAddr, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
#endif

}

void ESPNowWrapper::begin() {
  INFO("broker: %d \n", instance->isBroker);
  if (instance == nullptr) {
    ERROR("no instance!");
    return;
  }

  instance->prefs.begin("broker_prefs", false);
  uint32_t boot = instance->prefs.getUInt("boot", 0);
  boot++;
  instance->prefs.putUInt("boot", boot);
  instance->prefs.end();

  INFO(" Boot count: %d\n", boot);

  autoPair = false;
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    PRINTF("Error initializing ESP-NOW");
    return;
  }


#ifdef ESP32
  WiFi.enableLongRange(true);
  INFO("TX POWER: %d \n", WiFi.getTxPower());
  esp_wifi_config_espnow_rate(WIFI_IF_STA, WIFI_PHY_RATE_LORA_250K);
#endif

  INFO("ESP-NOW initialized\n");
  uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
  addPeer(broadcastAddress);

  INFO("Broadcast peer added\n");

#ifdef ESP8266 
  WiFi.printDiag(Serial);
#endif

#ifdef ESP32

  uint8_t baseMac[6];
  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);

  if (ret == ESP_OK) {
    INFO(" Node Name %s\n", __BASE_NAME__);
    INFO(" MAC Address: [%02x:%02x:%02x:%02x:%02x:%02x]\n",
                  baseMac[0], baseMac[1], baseMac[2],
                  baseMac[3], baseMac[4], baseMac[5]);
  } else {
    ERROR("Failed to read MAC address \n");
  }

#elif defined(ESP8266)
  INFO(" local mac: %s \n", String(WiFi.macAddress()).c_str());
#endif

  INFO(" Wi-Fi Channel: %d\n", WiFi.channel());

#ifdef ESP32
  esp_now_register_send_cb(esp_now_send_cb_t(OnDataSent));
  esp_now_register_recv_cb((esp_now_recv_cb_t)&ESPNowWrapper::onMessage);
#elif defined(ESP8266)
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_recv_cb(&ESPNowWrapper::onMessage);
#endif

  if (!isBroker) {
    registerHandler(MSG_TYPE::MSG_PAIR_OK, &ESPNowWrapper::pairOkHandler);
  } else {
    registerHandler(MSG_TYPE::MSG_PAIR_REQUEST, &ESPNowWrapper::pairRequestHandler);
  }

  loadDevices();

}

void ESPNowWrapper::onMessage(uint8_t* macAddr, uint8_t* msg, uint8_t len) {
  DEBUG("Received message \n");
  debugPrint(msg, len);

  MSG_TYPE messageType = (MSG_TYPE)msg[0];

  if (msg[0] > (uint8_t)MSG_TYPE::MSG_TYPE_COUNT) {
    WARN("Message type [%d] is out of bounds [%d]", msg[0], (uint8_t)MSG_TYPE::MSG_TYPE_COUNT);
    return;
  }

  if (instance->handlers[msg[0]] == nullptr) {
    WARN("Unregistered handler for message type [%d] \n", msg[0]);
    return;
  }

  if (messageType == MSG_TYPE::MSG_PAIR_REQUEST && !instance->isBroker) {
    INFO("Ignoring pair request, not a broker\n");
    return;
  }

  if (instance->handlers[msg[0]] == nullptr) {
    WARN("No handler registered for msgType: %d \n", msg[0]);
    return;
  }

  if (messageType != MSG_TYPE::MSG_PAIR_REQUEST && messageType != MSG_TYPE::MSG_PAIR_OK && instance->getDevice(macAddr) == nullptr) {
    WARN("Ignoring message from unknown peer [%02x:", macAddr[0]);
    PRINTF("%02x:", macAddr[1]);
    PRINTF("%02x:", macAddr[2]);
    PRINTF("%02x:", macAddr[3]);
    PRINTF("%02x:", macAddr[4]);
    PRINTF("%02x]\n", macAddr[5]);
    return;
  }

  // Guard for payload sizing (to prevent crashes)
  bool oversized = false;
  switch (messageType) {
    case MSG_TYPE::MSG_PAIR_REQUEST:
      oversized = (len > sizeof(PairRequestPayload) + 1);
      break;

    case MSG_TYPE::MSG_PAIR_OK:
      oversized = (len > sizeof(PairOkPayload) + 1);
      break;

    case MSG_TYPE::MSG_STATE_UPDATE:
      oversized = (len > sizeof(BasePayload) + 1);
      break;

    default:
      oversized = (len > sizeof(BasePayload) + 1);
  }

  if (oversized) {
    WARN("Msg (%d) payload size (%d)is larger than expected, dropping message \n", msg[0], len);
    return;
  }

  instance->handlers[msg[0]](macAddr, msg, len);
}

bool ESPNowWrapper::addDevice(const Device* device, bool asBroker) {
  uint8_t idx = 0;
  INFO(" Adding device: %s \n", device->name);

  if (!asBroker) {
    if (getDevice(device->macAddress) != nullptr) {
      INFO("Device already registered \n");
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
      ERROR("Unable to add Device, list is full \n");
      return false;
    }
  } else {
    INFO(" Adding new Broker [%02x:%02x:%02x:%02x:%02x:%02x]\n",
          device->macAddress[0],
          device->macAddress[1],
          device->macAddress[2],
          device->macAddress[3],
          device->macAddress[4],
          device->macAddress[5]
    );
  }

  INFO("Adding new Peer [%02x:%02x:%02x:%02x:%02x:%02x]\n",
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
  DEBUG("storing under key %s \n", key.c_str());
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
  ERROR("Device not found \n");
  return false;
}

bool ESPNowWrapper::removeDevice(uint8_t idx) {

  if (idx >= MAX_NODES) {
    ERROR("Index[%d] out of bounds trying to remove device \n", idx);
  }

  if (instance->devices[idx] != nullptr) {
    if (esp_now_del_peer((uint8_t*)(devices[idx]->macAddress)) != ESP_OK) {
      WARN("Failed to unregister peer [%02x:%02x:%02x:%02x:%02x:%02x], may not be registered\n",
        devices[idx]->macAddress[0], devices[idx]->macAddress[1], devices[idx]->macAddress[2],
        devices[idx]->macAddress[3], devices[idx]->macAddress[4], devices[idx]->macAddress[5]);
    }

    instance->prefs.begin("devices", false);
    if (!instance->prefs.remove(String("slot_" + String(idx)).c_str())) {
      ERROR("Error deleting device key from flash ram \n");
      instance->prefs.end();
      return false;
    };

    instance->prefs.end();
    delete devices[idx];
    devices[idx] = nullptr;
    INFO(" Device removed and preferences updated.\n");
    return true;
  }

  ERROR("Failed to remove peer [%02x:%02x:%02x:%02x:%02x:%02x] NOT FOUND \n",
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
  INFO("Devices stored in flash.\n");
}

void ESPNowWrapper::listDevices() {
  for (int i = 0; i < MAX_NODES; i++) {
    if (instance->devices[i] == nullptr) {
      continue;
    }
    INFO("[ DEVICE ] - SLOT %d - MAC [%02x:%02x:%02x:%02x:%02x:%02x] ",
            i,
           devices[i]->macAddress[0], devices[i]->macAddress[1], devices[i]->macAddress[2],
           devices[i]->macAddress[3], devices[i]->macAddress[4], devices[i]->macAddress[5]);
    INFO("  NAME [%s]\n", devices[i]->name);
  }
}

void ESPNowWrapper::loadDevices() {
  instance->prefs.begin("devices", false);

  for (int i = 0; i < MAX_NODES; i++) {
    String id = String("slot_" + String(i));
    INFO("Loading %s... ", id.c_str());

    if (!instance->prefs.isKey(id.c_str())) {
      PRINTF("NOT FOUND \n");
      instance->devices[i] = nullptr;
      continue;
    }

    PRINTF("FOUND ->");

    uint8_t buffer[sizeof(Device)];

    // Device* device;
    instance->prefs.getBytes(id.c_str(), &buffer, sizeof(Device));

    instance->devices[i] = new Device();
    memcpy(&instance->devices[i]->macAddress, &buffer, 6);
    memcpy(&instance->devices[i]->name, &buffer[6], sizeof(Device) - 6);

    PRINTF("MAC [%02x:", devices[i]->macAddress[0]);
    PRINTF("%02x:", devices[i]->macAddress[1]);
    PRINTF("%02x:", devices[i]->macAddress[2]);
    PRINTF("%02x:", devices[i]->macAddress[3]);
    PRINTF("%02x:", devices[i]->macAddress[4]);
    PRINTF("%02x]", devices[i]->macAddress[5]);
    PRINTF("  NAME [%s]\n", devices[i]->name);

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

bool ESPNowWrapper::sendToDevice(const Device* device, const uint8_t* message, const uint8_t len) {
  INFO(" Sending message to device: %s \n", String((char*)device->name).c_str());

  debugPrint((uint8_t*)device->macAddress, 6);

  if (!esp_now_is_peer_exist((uint8_t*)device->macAddress)) {
    ERROR("peer doesn't exist \n");
  }

  int result = esp_now_send((uint8_t*)device->macAddress, (uint8_t*)message, len);

  if (result == ESP_OK) {
    INFO("Message sent successfully\n");
    return true;
  } else {
    ERROR("Error sending message %d\n", result);
    return false;
  }
}

bool ESPNowWrapper::sendToDevice(uint8_t index, const uint8_t* message, const uint8_t len) {
  if (index >= MAX_NODES) {
    ERROR("Slot [%d] is out of bounds \n", index);
    return false;
  }

  if (devices[index] == nullptr) {
    ERROR("No device found on index %d\n", index);
  }

  return sendToDevice(devices[index], message, len);
}

void ESPNowWrapper::sendToAll(const uint8_t* message, const uint8_t len) {
  INFO("Sending message to all devices\n");

  for (int i = 0; i < MAX_NODES; i++) {
    if (devices[i] != nullptr) {
      sendToDevice(devices[i], message, len);
    }
  }

}

void ESPNowWrapper::broadcast(const uint8_t* message, const uint8_t len) {
  INFO("Broadcasting message: %s \n", String((char*)message).c_str());
  debugPrint((uint8_t*)message, len);

  uint8_t broadcastAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

  int result = esp_now_send((uint8_t*)broadcastAddress, (uint8_t*)message, len);
  INFO("Result: [%d]", result);

  if (result == ESP_OK) {
    INFO("Broadcast sent successfully \n");
  } else {
    ERROR("Error sending broadcast %d\n", result);
  }
};

void ESPNowWrapper::setAutoPair(bool enable) {
  autoPair = enable;
}

void ESPNowWrapper::requestToPair() {
  pairReqTTL = millis() + PAIR_REQ_TTL;
  Message<PairRequestPayload> m;
  m.msgType = (uint8_t)MSG_TYPE::MSG_PAIR_REQUEST;
  memset(&m.payload, 0, sizeof(m.payload));

#ifdef ESP32
  uint8_t baseMac[6];

  esp_err_t ret = esp_wifi_get_mac(WIFI_IF_STA, baseMac);
  if (ret != ESP_OK) {
    ERROR("Failed to read MAC address \n");
    return;
  }

  String deviceName = String(__BASE_NAME__) + String(WiFi.macAddress());
  strncpy((char*)&m.payload, deviceName.c_str(), sizeof(m.payload) - 1);
#elif defined(ESP8266)
  String deviceName = String(__BASE_NAME__) + String(WiFi.macAddress());
  strncpy((char*)&m.payload, deviceName.c_str(), sizeof(m.payload) - 1);
#endif



  broadcast((uint8_t*)&m, sizeof(Message<PairRequestPayload>));
}