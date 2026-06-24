#pragma once

#include <Arduino.h>

class TelegramAlert {
public:
  TelegramAlert(const char* botToken, const char* chatId);
  void sendMessage(const char* message);

private:
  const char* _botToken;
  const char* _chatId;
  String urlEncode(const char* msg);
};
