#include <header.hpp>
#include <ArduinoJson/Deserialization/DeserializationError.hpp>
#include <FS.h>
#include <ArduinoJson/Json/JsonDeserializer.hpp>
#include <ArduinoJson/Json/JsonSerializer.hpp>


String macToStr(const uint8_t* mac) {
  String result;
  for (int i = 0; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], HEX);
    if (i < 5)
      result += ':';
  }
  result.toUpperCase();
  return result;
}

String macLastThreeSegments(const uint8_t* mac) {
  String result;
  for (int i = 3; i < 6; ++i) {
    if (mac[i] < 0x10) result += "0";
    result += String(mac[i], HEX);
  }
  result.toUpperCase();
  return result;
}

bool readConfig() {
  File stateFile = SPIFFS.open("/config.json", "r");
  if (!stateFile) {
    Serial.println("[CONF] Failed to read config file... first run?");
    Serial.println("[CONF] Creating new file...");
    saveConfig();
    return false;
  }
  DeserializationError error = deserializeJson(json, stateFile.readString());
  stateFile.close();
  return true;
}

bool saveConfig() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("[CONF] Failed to open config file for writing");
    return false;
  }
  serializeJson(json, configFile);
  //serializeJson(json, Serial);
  configFile.close();
  return true;
}


/** IP to String */
String toStringIp(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}

/** Is this an IP? */
boolean isIp(String str) {
  for (size_t i = 0; i < str.length(); i++) {
    int c = str.charAt(i);
    if (c != '.' && (c < '0' || c > '9')) {
      return false;
    }
  }
  return true;
}