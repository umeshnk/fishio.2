#pragma once

#include <Arduino.h>

class OtaManager {
public:
  OtaManager(const char* hostname, const char* password);
  void begin();
  void loop();

private:
  const char* _hostname;
  const char* _password;
};
