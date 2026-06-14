#pragma once
#include <Arduino.h>

#define MAX_PAYLOAD_SIZE 128
#define NODE_NAME_LEN 32
#define STATE_PAYLOAD_SIZE 64
#define MAC_ADDR_LEN 6

enum class MSG_TYPE {
  MSG_PAIR_REQUEST,   // message to request pairing - broadcast from node
  MSG_PAIR_OK,        // message to accept pairing returned to pairing request from broker - > node
  MSG_STATE_UPDATE,   // message emitted by node on state update node -> broker 
  MSG_NODE_CMD,       // command from broker to node
  MSG_NODE_ST,        // request state update from node, broker -> node
  MSG_TYPE_COUNT
};

enum class PAYLOAD_TYPE {
  PAYLOAD_PAIR_REQUEST,
  PAYLOAD_PAIR_OK,
  PAYLOAD_STATE_UPDATE,
  PAYLOAD_NODE_CMD,
  PAYLOAD_NODE_ST
};

enum class AURORA_COMMANDS {
  CMD_PLAY,
  CMD_OFF,
  CMD_PAUSE,
  CMD_PAUSE_RESUME,
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
  char nodeName[NODE_NAME_LEN];
} PairRequestPayload;

typedef struct {
  uint8_t msgType;
  uint8_t data[32];
} PairOkPayload;

// 32+6 bytes for a node
typedef struct {
  uint8_t macAddress[MAC_ADDR_LEN];
  uint8_t name[NODE_NAME_LEN];
} Node;

typedef struct {
  uint8_t inBoundAddr[MAC_ADDR_LEN];
  uint8_t outBoundAddr[MAC_ADDR_LEN];
} RoutingRule;

typedef struct {
  uint8_t msgType;
  uint8_t payloadType;
  uint8_t payload[STATE_PAYLOAD_SIZE];
} StateMessage;

typedef struct {
  uint8_t type;
  uint8_t name[32];
  uint8_t ledState[32];
} AuroraNodeStatePayload;