#include "TelegramAlert.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include "Config.h"  // brings in Secrets.h (optional TELEGRAM_ROOT_CA)

TelegramAlert::TelegramAlert(const char* botToken, const char* chatId)
  : _botToken(botToken), _chatId(chatId) {}

// Minimal percent-encoding so spaces/°/symbols don't break the request body.
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

  // Configure TLS once on the reused client (avoids per-alert object churn).
  if (!_tlsConfigured) {
#ifdef TELEGRAM_ROOT_CA
    _secureClient.setCACert(TELEGRAM_ROOT_CA);  // Pinned: certificate is verified
#else
    _secureClient.setInsecure();  // No pinning; define TELEGRAM_ROOT_CA to verify
    Serial.println("Telegram: TLS certificate NOT verified (set TELEGRAM_ROOT_CA to pin)");
#endif
    _tlsConfigured = true;
  }

  HTTPClient https;
  // POST so chat_id/text travel in the body, not the URL (keeps message text out
  // of logs/proxies). The bot token in the path is required by the Telegram API.
  String url = String("https://api.telegram.org/bot") + _botToken + "/sendMessage";
  if (!https.begin(_secureClient, url)) {
    Serial.println("Telegram: begin() failed");
    return;
  }
  https.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String body = String("chat_id=") + _chatId + "&text=" + urlEncode(message);
  int httpCode = https.POST(body);

  if (httpCode > 0) {
    Serial.printf("Telegram alert sent (Response: %d)\n", httpCode);
  } else {
    Serial.printf("Telegram alert failed (Error: %s)\n", https.errorToString(httpCode).c_str());
  }

  https.end();
}
