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
  // Single non-blocking attempt; the main loop retries each cycle so we never
  // stall the device (display/sensors) waiting on an unreachable broker.
  if (_client.connected()) return;

  Serial.print("Connecting to MQTT... ");
  if (_client.connect(_clientId, _user, _password)) {
    Serial.println("connected");
  } else {
    Serial.printf("failed (state %d), will retry next cycle\n", _client.state());
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
