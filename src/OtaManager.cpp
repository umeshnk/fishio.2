#include "OtaManager.h"

#include <ArduinoOTA.h>
#include <ESPmDNS.h>
#include <WiFi.h>

OtaManager::OtaManager(const char* hostname, const char* password)
  : _hostname(hostname), _password(password) {}

void OtaManager::begin() {
  ArduinoOTA.setHostname(_hostname);
  ArduinoOTA.setPassword(_password);

  ArduinoOTA.onStart([]() { Serial.println("OTA: update starting"); });
  ArduinoOTA.onEnd([]() { Serial.println("\nOTA: update complete"); });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA progress: %u%%\r", (progress * 100) / total);
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA error [%u]\n", error);
  });

  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void OtaManager::loop() {
  ArduinoOTA.handle();
}
