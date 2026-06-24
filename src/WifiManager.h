#pragma once

#include <Arduino.h>
#include <WiFi.h>

class WifiManager {
public:
  WifiManager(const char* ssid, const char* password);

  void connect();
  bool isConnected();
  const char* getAddress();

private:
  const char* _ssid;
  const char* _password;
  String _ip;
};
