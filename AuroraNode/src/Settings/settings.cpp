#include "settings.h"

Settings* Settings::instance = nullptr;

Settings::Settings() {
  EEPROM.begin(EEPROM_SIZE);
  loadFromEEPROM();
}

Settings* Settings::getInstance() {
  if (instance == nullptr) {
    instance = new Settings();
  }
  return instance;
}

int Settings::getSetting(String key) {
  if (settingsMap.find(key) != settingsMap.end()) {
    return settingsMap[key].toInt();
  }
  return 0; // default value
}

bool Settings::getSettingBool(String key) {
  if (settingsMap.find(key) != settingsMap.end()) {
    return settingsMap[key].toInt() != 0;
  }
  return false; // default value
}

float Settings::getSettingFloat(String key) {
  if (settingsMap.find(key) != settingsMap.end()) {
    return settingsMap[key].toFloat();
  }
  return 0.0; // default value
}

String Settings::getSettingString(String key) {
  if (settingsMap.find(key) != settingsMap.end()) {
    return settingsMap[key];
  }
  return ""; // default value
}

void Settings::loadFromEEPROM() {
  String data;
  for (int i = 0; i < EEPROM_SIZE; i++) {
    char c = EEPROM.read(i);
    if (c == '\0') break;
    data += c;
  }
  // Parse data: key1=value1\nkey2=value2\n
  int start = 0;
  int end = data.indexOf('\n', start);
  while (end != -1) {
    String line = data.substring(start, end);
    int eqPos = line.indexOf('=');
    if (eqPos != -1) {
      String key = line.substring(0, eqPos);
      String value = line.substring(eqPos + 1);
      settingsMap[key] = value;
    }
    start = end + 1;
    end = data.indexOf('\n', start);
  }
}

void Settings::saveToEEPROM() {
  String data;
  for (auto& pair : settingsMap) {
    data += pair.first + "=" + pair.second + "\n";
  }
  for (int i = 0; i < data.length() && i < EEPROM_SIZE; i++) {
    EEPROM.write(i, data[i]);
  }
  EEPROM.write(data.length(), '\0');
  EEPROM.commit();
}