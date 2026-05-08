
#ifdef ESP32
  // Todo - add settings manager and persist settings to eeprom
#define NUM_LEDS 10
#define PAIR_BTN 5

#define LOG(...) printf(__VA_ARGS__)

// may be needed to prevent FastLED from disabling interrupts, which can cause issues with ESP-NOW
// #define FASTLED_ALLOW_INTERRUPTS 0
#elif defined(ESP8266)


#define NUM_LEDS 10
#define PAIR_BTN D5
#endif