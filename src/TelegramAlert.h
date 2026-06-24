#pragma once

#include <Arduino.h>
#include <WiFiClientSecure.h>

class TelegramAlert {
public:
  TelegramAlert(const char* botToken, const char* chatId);
  void sendMessage(const char* message);

private:
  const char* _botToken;
  const char* _chatId;
  WiFiClientSecure _secureClient;  // Persistent: configured once, reused per alert
  bool _tlsConfigured = false;
  String urlEncode(const char* msg);
};
