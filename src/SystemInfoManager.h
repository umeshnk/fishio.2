#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"
#include "MqttManager.h"

class SystemInfoManager {
private:
  MqttManager& mqtt;
  String deviceId;
  unsigned long lastPublish;
  unsigned long publishInterval;

public:
  SystemInfoManager(MqttManager& mqttClient, const char* id, unsigned long intervalMs = 60000)
    : mqtt(mqttClient), deviceId(id), lastPublish(0), publishInterval(intervalMs) {}

  void publishNow() {
    if (!mqtt.isConnected()) return;

    char payload[256];
    snprintf(payload, sizeof(payload),
      "{"
      "\"chipId\":%u,"
      "\"heapFree\":%u,"
      "\"flashSize\":%u,"
      "\"wifiRSSI\":%d,"
      "\"uptimeSec\":%lu"
      "}",
      (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF),
      ESP.getFreeHeap(),
      (uint32_t)ESP.getFlashChipSize(),
      WiFi.RSSI(),
      millis() / 1000UL
    );

    char topic[64];
    snprintf(topic, sizeof(topic), "system/%s/status", deviceId.c_str());

    mqtt.publish(topic, payload);
    Serial.printf("Published system info for %s (%s)\n", MQTT_CLIENT_ID, payload);
  }

  void loop() {
    unsigned long now = millis();
    if (now - lastPublish >= publishInterval) {
      publishNow();
      lastPublish = now;
    }
  }

  void setInterval(unsigned long ms) {
    publishInterval = ms;
  }
};
