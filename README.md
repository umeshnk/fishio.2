# fishio.2

Next-generation temperature monitoring and control system for aquariums —
the ESP32 successor to the original [fishio](../fishio).

Monitors water temperature across multiple tanks with DS18B20 sensors, shows
live readings on a GC9A01 round LCD, publishes to MQTT, and alerts via Telegram
and a passive buzzer. Designed with headroom for relay/solenoid control
(heaters, lights, pumps) and analog probes (pH, TDS, ...) — see
[HARDWARE.md](HARDWARE.md).

## Hardware

| Component   | Detail                                            |
| ----------- | ------------------------------------------------- |
| MCU         | ESP32-WROOM-32D (DevKit)                           |
| Display     | GC9A01 240×240 round IPS LCD, SPI                  |
| Temperature | DS18B20 on OneWire (GPIO25, shared bus)            |
| Buzzer      | Passive piezo, LEDC PWM (GPIO26)                   |

Full pin allocation and future-expansion provisions are documented in
[HARDWARE.md](HARDWARE.md).

## Architecture

Single-purpose manager classes wired together in [`src/main.cpp`](src/main.cpp):
`WifiManager`, `MqttManager`, `TempSensorManager`, `DisplayManager` (GC9A01 via
Arduino_GFX), `Buzzer` (LEDC), `TelegramAlert`, `SystemInfoManager`, `OtaManager`.

Every 10 s each sensor is read, mapped to a tank name from `TANKS[]` in
[`src/Config.h`](src/Config.h), published to `temp/aquarium/<id>`, and shown on
the display. Out-of-range temps or sensor disconnects raise a buzzer + Telegram
alert (debounced per sensor). Device health is published to
`system/<clientId>/status` every 60 s.

## Setup

1. Copy `src/Secrets.h.example` to `src/Secrets.h` and fill in WiFi, Telegram,
   and OTA credentials.
2. Review MQTT settings, thresholds, and the `TANKS[]` map in `src/Config.h`.

## Build & flash

```bash
pio run                      # build
pio run --target upload      # flash over USB
pio device monitor           # serial monitor (115200)
```

After the first USB flash, updates can be pushed over the air (password-protected
ArduinoOTA).

### Dependencies

- `paulstoffregen/OneWire`
- `milesburton/DallasTemperature`
- `knolleary/PubSubClient`
- `moononournation/GFX Library for Arduino` (pinned to 1.3.7 for ESP32 core 2.x)
