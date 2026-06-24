#include "TelegramAlert.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

TelegramAlert::TelegramAlert(const char* botToken, const char* chatId)
  : _botToken(botToken), _chatId(chatId) {}

// Minimal percent-encoding so spaces/°/symbols don't break the GET request.
String TelegramAlert::urlEncode(const char* msg) {
  String out;
  for (const char* p = msg; *p; p++) {
    char c = *p;
    if (isalnum((unsigned char)c) || c == '-' || c == '_' || c == '.' || c == '~') {
      out += c;
    } else {
      char buf[4];
      snprintf(buf, sizeof(buf), "%%%02X", (unsigned char)c);
      out += buf;
    }
  }
  return out;
}

void TelegramAlert::sendMessage(const char* message) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();  // Skip cert verification (simple)

  HTTPClient https;
  String url = String("https://api.telegram.org/bot") + _botToken
               + "/sendMessage?chat_id=" + _chatId
               + "&text=" + urlEncode(message);

  https.begin(client, url);
  int httpCode = https.GET();

  if (httpCode > 0) {
    Serial.printf("Telegram alert sent (Response: %d)\n", httpCode);
  } else {
    Serial.printf("Telegram alert failed (Error: %s)\n", https.errorToString(httpCode).c_str());
  }

  https.end();
}
