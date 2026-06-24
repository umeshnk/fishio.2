#include "MqttManager.h"

MqttManager::MqttManager(const char* server, int port,
                         const char* clientId,
                         const char* user, const char* password)
  : _server(server), _port(port),
    _clientId(clientId), _user(user), _password(password) {}

void MqttManager::begin(WiFiClient& wifiClient, std::function<void(char*, byte*, unsigned int)> callback) {
  _client.setClient(wifiClient);
  _client.setServer(_server, _port);
  _client.setCallback(callback);
}

void MqttManager::connect() {
  int tries = 0;
  while (!_client.connected() && tries < 10) {
    Serial.print("Connecting to MQTT ");
    if (_client.connect(_clientId, _user, _password)) {
      Serial.println("was successful");
    } else {
      Serial.println("failed, retrying...");
      delay(5000);
      tries++;
    }
  }
}

bool MqttManager::isConnected() {
  return _client.connected();
}

void MqttManager::loop() {
  _client.loop();
}

void MqttManager::publish(const char* topic, const char* payload) {
  if (_client.connected()) _client.publish(topic, payload);
}

void MqttManager::subscribe(const char* topic) {
  if (_client.connected()) _client.subscribe(topic);
}
