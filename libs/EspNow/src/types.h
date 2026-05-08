#pragma once
#include <Arduino.h>

#define MAX_PAYLOAD_SIZE 128
#define NODE_NAME_LEN 32
#define MAC_ADDR_LEN 6

enum class MSG_TYPE {
  MSG_PAIR_REQUEST, // message to request pairing - broadcast from node
  MSG_PAIR_OK, // message to accept pairing returned to pairing request from broker - > node
  MSG_STATE_UPDATE, // message emitted by node on state update node -> broker 
  MSG_AURORA_CMD, // command from broker to auroranode
  MSG_AURORA_ST, // request state update from auroraNode, broker -> node
  MSG_SENSOR_ST, // request state update from sensorNode, broker -> node
  MSG_TYPE_COUNT
};

enum class AURORA_COMMANDS {
  CMD_PLAY,
  CMD_OFF,
  CMD_PAUSE,
  CMD_FX,
  CMD_FX_NEXT,
  CMD_FX_PREV,
  CMD_FX_SPEED,
  CMD_SET_BR,
  CMD_SET_HSV,
  CMD_SET_PX,
  COMMANDS_COUNT
};



template <typename T>
struct Message {
  uint8_t msgType;
  T payload;
};

typedef uint8_t BasePayload[MAX_PAYLOAD_SIZE];

typedef struct {
  char deviceName[NODE_NAME_LEN];
} PairRequestPayload;

typedef struct {
  uint8_t msgType;
  uint8_t data[32];
} PairOkPayload;

// 32 bytes for a device
typedef struct {
  uint8_t macAddress[MAC_ADDR_LEN];
  uint8_t name[NODE_NAME_LEN];
} Device;


typedef struct {
  uint8_t inBoundAddr[MAC_ADDR_LEN];
  uint8_t outBoundAddr[MAC_ADDR_LEN];
} RoutingRule;