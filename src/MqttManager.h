#pragma once

#include <Arduino.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <functional>

class MqttManager {
public:
  MqttManager(const char* server, int port,
              const char* clientId,
              const char* user, const char* password);

  void begin(WiFiClient& wifiClient, std::function<void(char*, byte*, unsigned int)> callback);
  void connect();
  bool isConnected();
  void loop();
  void publish(const char* topic, const char* payload);
  void subscribe(const char* topic);

private:
  const char* _server;
  int _port;
  const char* _clientId;
  const char* _user;
  const char* _password;
  PubSubClient _client;
};
