// #pragma once
// #include <Arduino.h>

// #define MAX_PAYLOAD_SIZE 128

// enum MSG_TYPE {
//   MSG_PAIR_REQUEST,
//   MSG_PAIR_OK,
//   MSG_STATE_UPDATE,
//   MSG_AURORA_CMD,
//   MSG_AURORA_ST,
//   MSG_SENSOR_ST,
//   MSG_TYPE_COUNT
// };

// template <typename T>
// struct Message {
//   uint8_t msgType;
//   T payload;
// };

// typedef uint8_t BasePayload[MAX_PAYLOAD_SIZE];

// typedef struct {
//   char deviceName[26];
// } PairRequestPayload;

// typedef struct {
//   uint8_t msgType;
//   uint8_t data[32];
// } PairOkPayload;

// // 32 bytes for a device
// typedef struct {
//   uint8_t macAddress[6];
//   uint8_t name[26];
// } Device;


// typedef struct {
//   uint8_t inBoundAddr[6];
//   uint8_t outBoundAddr[6];
// } RoutingRule;