#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include <EEPROM.h>
#include <map>

class Settings {
  private:
  static Settings* instance;
  Settings();
  std::map<String, String> settingsMap;
  const int EEPROM_SIZE = 512;
  void loadFromEEPROM();
  void saveToEEPROM();

  public:
  static Settings* getInstance();
  template<typename T>
  void setSetting(String key, T value) {
    settingsMap[key] = String(value);
    saveToEEPROM();
  }
  int getSetting(String key);
  bool getSettingBool(String key);
  float getSettingFloat(String key);
  String getSettingString(String key);
};

#endif