#pragma once

#include "Secrets.h"

/*
 * fishio.2 configuration.
 * Pin assignments follow HARDWARE.md — do not change without updating it.
 * WiFi / Telegram / OTA credentials live in Secrets.h (git-ignored).
 */

#define VERSION "2.0.0"

// ====== MQTT Settings ======
#define MQTT_SERVER "10.0.1.8"
#define MQTT_PORT 1883
#define MQTT_USER "mqtt_user"
#define MQTT_PASSWORD "mqtt_user"

// Unique per device (e.g. "FISHIO_TOWER", "FISHIO_FARM", ...)
#define MQTT_CLIENT_ID "FISHIO2_TOWER"

// ====== Display: GC9A01 (SPI / VSPI) ======
#define TFT_SCLK 18
#define TFT_MOSI 23
#define TFT_CS 13
#define TFT_DC 16
#define TFT_RST 17
#define TFT_BL 4  // Backlight (PWM-capable)

// ====== DS18B20 (OneWire) ======
#define ONE_WIRE_BUS 25  // Shared bus; 4.7k pull-up to 3V3
#define MAX_SENSORS 16

// ====== Buzzer (passive piezo, LEDC PWM) ======
#define BUZZER_PIN 26

// ====== Temperature thresholds (°C) ======
#define TEMP_HIGH 28
#define TEMP_LOW 20

// ---------------------------------------------------------------------------
// Reserved for future expansion (see HARDWARE.md). Break out to headers when
// soldering. Uncomment/use as features are added.
// ---------------------------------------------------------------------------
// Relay / solenoid outputs (heaters, lights, pumps) — add 10k pull-down each
// #define RELAY_1_PIN 32
// #define RELAY_2_PIN 33
// #define RELAY_3_PIN 27
//
// Analog probes — ADC1 only (WiFi-safe), input-only pins
// #define PH_ADC_PIN 36
// #define TDS_ADC_PIN 39
// #define ADC_AUX1_PIN 34
// #define ADC_AUX2_PIN 35
//
// I2C expansion bus (ADS1115 precise ADC, MCP23017 relay bank, RTC, ...)
// #define I2C_SDA_PIN 21
// #define I2C_SCL_PIN 22

// ---------------------------------------------------------------------------
// 🐠 Tank sensor-ID → name pairing
// ---------------------------------------------------------------------------
struct TankInfo {
  const char* id;
  const char* name;
};

const TankInfo TANKS[MAX_SENSORS] = {
  { "281A61BF0000005D", "Juwel" },
  { "28A615BB0000005B", "Guppies" },
  { "2871A6BF00000015", "Tower 1" },
  { "2889F4BE0000005F", "Tower 2" },
  { "288D89BC00000005", "Tower 3" },
  { "28C343BE000000FB", "Tower 4" },
  { "280488BF0000003C", "Farm 1" },
  { "28CE1ABB00000056", "Farm 2" },
  { "28B18CBC00000047", "Farm 3" },
  { "284275BF000000B1", "Demo One" },
  { "28E9D8BF000000CA", "Demo Two" },
  { "28E4B6BF00000033", "Fishio 1.5" }
};
