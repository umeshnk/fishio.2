# fishio.2

Next-generation temperature monitoring and control system for aquariums —
the ESP32 successor to the original [fishio](../fishio).

Monitors water temperature across multiple tanks with DS18B20 sensors, shows
live readings on a GC9A01 round LCD, publishes to MQTT, and alerts via Telegram
and a passive buzzer. Designed with headroom for relay/solenoid control
(heaters, lights, pumps) and analog probes (pH, TDS, ...) — see
[HARDWARE.md](HARDWARE.md).

📖 A full **HTML user manual** (hardware, pin layout, wiring diagrams, build
and troubleshooting) lives at [docs/manual.html](docs/manual.html) — open it in
any browser.

## Hardware

| Component   | Detail                                            |
| ----------- | ------------------------------------------------- |
| MCU         | ESP32-WROOM-32D (DevKit)                           |
| Display     | GC9A01 240×240 round IPS LCD, SPI                  |
| Temperature | DS18B20 on OneWire (GPIO5, shared bus)             |
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

## Display layouts

The round dashboard has two layouts, selected by `DISPLAY_MODE` in
[`src/Config.h`](src/Config.h):

- `DISPLAY_CAROUSEL` — one tank at a time, large, auto-cycling with a colored
  status ring (dwell time set by `CAROUSEL_INTERVAL_MS`).
- `DISPLAY_LIST` — all tanks as rows on a single screen.

Status colors: **green** = in range, **red** = out of range, **yellow** =
sensor error/disconnected.

## Setup

1. Copy `src/Secrets.h.example` to `src/Secrets.h` and fill in WiFi, Telegram,
   and OTA credentials.
2. Review MQTT settings, thresholds, and the `TANKS[]` map in `src/Config.h`.

## Build & flash

```bash
pio run                      # build the main firmware
pio run --target upload      # flash over USB
pio device monitor           # serial monitor (115200)
```

After the first USB flash, updates can be pushed over the air (password-protected
ArduinoOTA).

### Display diagnostic

A standalone `diag` environment brings up the GC9A01 (color fills, shapes, text)
independently of the main firmware — useful for isolating wiring vs. SPI-speed
issues:

```bash
pio run -e diag -t upload
pio device monitor
```

### Dependencies

- `paulstoffregen/OneWire`
- `milesburton/DallasTemperature`
- `knolleary/PubSubClient`
- `moononournation/GFX Library for Arduino` (pinned to 1.3.7 for ESP32 core 2.x)
