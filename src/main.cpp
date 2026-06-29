// main.cpp — fishio.2 (ESP32 + GC9A01)
#include <WiFi.h>
#include "Buzzer.h"
#include "Config.h"
#include "DisplayManager.h"
#include "MqttManager.h"
#include "OtaManager.h"
#include "SystemInfoManager.h"
#include "TelegramAlert.h"
#include "TempSensorManager.h"
#include "WifiManager.h"

// Managers
WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
WiFiClient espClient;
MqttManager mqtt(MQTT_SERVER, MQTT_PORT, MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
TempSensorManager sensors(ONE_WIRE_BUS);
TelegramAlert telegram(TELEGRAM_BOT_TOKEN, TELEGRAM_CHAT_ID);
Buzzer buzzer(BUZZER_PIN);
SystemInfoManager sysInfo(mqtt, MQTT_CLIENT_ID, 60000);  // Every 60 seconds
OtaManager ota(MQTT_CLIENT_ID, OTA_PASSWORD);

// Store alert state per sensor
bool alertState[MAX_SENSORS] = { false };

// --- Fixed-size buffers ---
static char idBuf[32];
static char topicBuf[64];
static char msgBuf[160];  // Alerts / telegram messages, boot screens
static char dtBuf[16];    // Temperature as string for MQTT

// Display dashboard data. nameStore backs the readings' name pointers so they
// stay valid until showReadings() copies them.
static char nameStore[MAX_SENSORS][20];
static DisplayManager::TankReading readings[MAX_SENSORS];

// Timing
const unsigned long SENSORS_INTERVAL_MS = 10UL * 1000UL;  // 10 seconds between cycles
unsigned long lastSensorsMillis = 0;

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT message [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  buzzer.begin();
  DisplayManager::begin();
  DisplayManager::displayText("Fishio 2\nstarting...");

  wifi.connect();
  ota.begin();
  mqtt.begin(espClient, mqttCallback);
  mqtt.connect();

  sensors.begin();

  snprintf(msgBuf, sizeof(msgBuf), "Fishio 2 starting up.");
  telegram.sendMessage(msgBuf);
  buzzer.playMelody();

  lastSensorsMillis = millis();

  const char* ipAddress = wifi.getAddress();
  snprintf(msgBuf, sizeof(msgBuf), "WiFi Connected\nIP: %s", ipAddress);
  DisplayManager::displayText(msgBuf);
}

void processSensorsOnce() {
  // Ensure WiFi and MQTT are connected
  if (!wifi.isConnected()) wifi.connect();
  if (!mqtt.isConnected()) mqtt.connect();

  mqtt.loop();
  ota.loop();

  int count = sensors.getCount();
  if (count < 0) return;

  // One bus-wide conversion per cycle (not once per sensor).
  sensors.requestTemperatures();

  int rcount = 0;
  for (int i = 0; i < count && i < MAX_SENSORS; i++) {
    const String sensorIdStr = sensors.getId(i);
    strncpy(idBuf, sensorIdStr.c_str(), sizeof(idBuf));
    idBuf[sizeof(idBuf) - 1] = '\0';

    float temp = sensors.getTempC(i);

    // Resolve tank name (Config TANKS array), falling back to the raw id.
    const char* matched = nullptr;
    for (int j = 0; j < MAX_SENSORS; j++) {
      if (TANKS[j].id == nullptr) continue;
      if (strcmp(idBuf, TANKS[j].id) == 0) {
        matched = TANKS[j].name;
        break;
      }
    }
    strncpy(nameStore[rcount], matched ? matched : idBuf, sizeof(nameStore[rcount]));
    nameStore[rcount][sizeof(nameStore[rcount]) - 1] = '\0';
    const char* name = nameStore[rcount];

    snprintf(topicBuf, sizeof(topicBuf), "temp/aquarium/%s", idBuf);

    DisplayManager::Status status;
    if (temp == DEVICE_DISCONNECTED_C) {
      status = DisplayManager::STATUS_ERROR;
      snprintf(msgBuf, sizeof(msgBuf), "%s sensor disconnected!", name);
      Serial.println(msgBuf);
      buzzer.alert(5);
      if (!alertState[i]) {
        telegram.sendMessage(msgBuf);
        alertState[i] = true;
      }
    } else {
      dtostrf(temp, 4, 2, dtBuf);
      mqtt.publish(topicBuf, dtBuf);
      Serial.printf("Published %s (Topic: %s, Temp: %.2fC)\n", name, topicBuf, temp);

      if (temp < TEMP_LOW || temp > TEMP_HIGH) {
        status = DisplayManager::STATUS_ALERT;
        snprintf(msgBuf, sizeof(msgBuf),
                 "%s temperature out of range (Temp: %.2fC, Low: %d, High: %dC)",
                 name, temp, TEMP_LOW, TEMP_HIGH);
        Serial.println(msgBuf);
        buzzer.alert(3);
        if (!alertState[i]) {
          telegram.sendMessage(msgBuf);
          alertState[i] = true;
        }
      } else {
        status = DisplayManager::STATUS_OK;
        if (alertState[i]) {
          snprintf(msgBuf, sizeof(msgBuf),
                   "%s temperature back to normal (Temp: %.2fC)", name, temp);
          Serial.println(msgBuf);
          telegram.sendMessage(msgBuf);
          buzzer.alert(1);
          alertState[i] = false;
        }
      }
    }

    readings[rcount].name = nameStore[rcount];
    readings[rcount].tempC = temp;
    readings[rcount].status = status;
    rcount++;
  }  // end for sensors

  DisplayManager::showReadings(readings, rcount);
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorsMillis >= SENSORS_INTERVAL_MS) {
    lastSensorsMillis = now;
    processSensorsOnce();
  }

  // Keep services responsive between sensor cycles.
  sysInfo.loop();
  mqtt.loop();
  ota.loop();
  DisplayManager::tick();  // advances the carousel between data updates

  delay(10);
}
