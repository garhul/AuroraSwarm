#pragma once

#include <Arduino.h>

#if defined(ESP8266)

#define DEBUG(...) Serial.print("[ DEBUG ] - ");Serial.printf(__VA_ARGS__);
#define INFO(...) Serial.print("[ INFO ] - ");Serial.printf(__VA_ARGS__);
#define PRINTF(...) Serial.printf(__VA_ARGS__);
#define ERROR(...) Serial.print("[ ERROR ] - ");Serial.printf(__VA_ARGS__);
#define WARN(...) Serial.print("[ WARNING ] - ");Serial.printf(__VA_ARGS__);

#elif defined(ESP32)

#define DEBUG(...) printf("[ DEBUG ] - ");printf(__VA_ARGS__);
#define INFO(...)  printf("[ INFO ] - ");printf(__VA_ARGS__);
#define PRINTF(...) printf(__VA_ARGS__);
#define ERROR(...) printf("[ ERROR ] - ");printf(__VA_ARGS__);
#define WARN(...)  printf("[ WARNING ] - ");printf(__VA_ARGS__);

#endif

