#include "WifiManager.h"

WifiManager::WifiManager(const char* ssid, const char* password)
  : _ssid(ssid), _password(password) {}

void WifiManager::connect() {
  if (WiFi.status() == WL_CONNECTED) return;

  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid, _password);
  Serial.print("Connecting to WiFi");

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 40) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    _ip = WiFi.localIP().toString();
    Serial.printf("\nWiFi connected (IP: %s)\n", _ip.c_str());
  } else {
    Serial.println("\nWiFi connection failed");
  }
}

bool WifiManager::isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

const char* WifiManager::getAddress() {
  _ip = WiFi.localIP().toString();
  return _ip.c_str();
}
