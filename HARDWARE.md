# fishio.2 — Hardware & Pin Allocation

Next-generation aquarium monitor/controller. Successor to the original `fishio`
(ESP8266 + SSD1306). This document is the single source of truth for the board
layout, pin assignments, and reserved-for-future provisions.

## Platform

| | |
| --- | --- |
| MCU | ESP32-WROOM-32D (DevKit) |
| Framework | Arduino / PlatformIO |
| Display | GC9A01 240×240 round IPS LCD, SPI |
| Temperature | DS18B20 on OneWire (1+ sensors on shared bus) |
| Audio | Buzzer (passive piezo via LEDC PWM — TBD: confirm passive vs active) |

## ESP32 pin rules (why pins are chosen the way they are)

- **GPIO6–11** — wired to internal SPI flash. **Never use.**
- **GPIO34–39** — input-only, no internal pull-up, no output. Good only for
  analog/digital *inputs*.
- **GPIO0 / 2 / 5 / 12 / 15** — strapping pins; wrong level at boot prevents
  booting or flashing. Avoid for anything that idles at a fixed level or clicks
  a relay.
- **GPIO1 / 3** — UART0 TX/RX (USB serial + flashing). Leave alone.
- **ADC2 pins are unusable while WiFi is active.** All analog sensing must use
  **ADC1 = GPIO32–39**.

## Current build — active connections

### GC9A01 display (SPI / VSPI)

GC9A01 is write-only, so MISO is not connected (frees GPIO19). Using the native
VSPI pins lets the SPI hardware peripheral run at 40–80 MHz.

Rows below are in the module's physical header order (VCC, GND, SCL, SDA, DC,
CS, RST) for easy soldering.

| Display pin | ESP32 GPIO | Function |
| --- | --- | --- |
| VCC | 3V3 | — |
| GND | GND | — |
| SCL | GPIO18 | SPI clock (VSPI SCLK) |
| SDA | GPIO23 | SPI data (VSPI MOSI) |
| DC | GPIO16 | Data/command |
| CS | GPIO4 | Chip select |
| RST | GPIO17 | Reset |

This module has no BLK pin (backlight hardwired on). CS uses GPIO4 so all five
display signals sit on the same DevKit header side; GPIO13 is left free.

### DS18B20 temperature (OneWire)

| Sensor pin | ESP32 GPIO |
| --- | --- |
| DATA | GPIO5 (+ 4.7 kΩ pull-up to 3V3, one per bus) |
| VDD | 3V3 |
| GND | GND |

GPIO5 sits on the display header side. It is a strapping pin, but the OneWire
pull-up holds it HIGH at boot (the level the strap requires), so it is safe here.

Multiple DS18B20s share GPIO5 on the one-wire bus.

### Buzzer

| Buzzer | ESP32 GPIO |
| --- | --- |
| + | GPIO26 (LEDC PWM — ESP32 has no `tone()`) |
| – | GND |

Small passive piezo can sit on the GPIO directly; a louder active/magnetic
buzzer needs an NPN transistor + base resistor.

## Reserved for future expansion

Break these out to headers/pads during soldering even if unused now.

### Relay / solenoid outputs (heaters, lights, pumps)

| Use | ESP32 GPIO | Notes |
| --- | --- | --- |
| Relay 1 | GPIO32 | Safe at boot, not strapping |
| Relay 2 | GPIO33 | Safe at boot |
| Relay 3 | GPIO27 | Safe at boot |
| More | I²C expander | MCP23017 on the I²C bus → 16 more outputs |

- Use opto-isolated relay/SSR boards. Add a 10 kΩ **pull-down** on each relay
  GPIO so a floating pin at reset can't switch a heater on.
- The ESP32 only drives the relay/SSR logic input — actuators get their own PSU
  with a **common ground**. DC solenoids need a **flyback diode**.

### Analog monitoring (pH, TDS, ORP, water level) — ADC1 only

| Probe | ESP32 GPIO | ADC channel |
| --- | --- | --- |
| pH | GPIO36 (VP) | ADC1_0 |
| TDS | GPIO39 (VN) | ADC1_3 |
| ORP / spare | GPIO34 | ADC1_6 |
| Level / spare | GPIO35 | ADC1_7 |

- Probe front-ends often exceed 3.3 V or need amplifiers — condition/clamp to
  0–3.3 V before the ADC.
- The ESP32's built-in ADC is noisy/non-linear. Preferred path: an **ADS1115**
  (16-bit) on the I²C bus instead of the raw GPIOs above.

### I²C expansion bus

| Signal | ESP32 GPIO |
| --- | --- |
| SDA | GPIO21 |
| SCL | GPIO22 |

Left fully free for ADS1115 (precise ADC), MCP23017 (relay bank), RTC, etc.

## Allocation summary

| GPIO | Assignment |
| --- | --- |
| 4 | Display CS |
| 5 | DS18B20 data (strapping, but pulled HIGH at boot) |
| 13 | spare (free) |
| 16 | Display DC |
| 17 | Display RST |
| 18 | Display SCLK |
| 23 | Display MOSI |
| 25 | spare (free) |
| 26 | Buzzer |
| 27 | *reserved* Relay 3 |
| 32 | *reserved* Relay 1 |
| 33 | *reserved* Relay 2 |
| 34 | *reserved* analog (ADC1) |
| 35 | *reserved* analog (ADC1) |
| 36 | *reserved* analog pH (ADC1) |
| 39 | *reserved* analog TDS (ADC1) |
| 21 / 22 | *reserved* I²C bus |
| 19 | spare (SPI MISO, unused by GC9A01) |
| 6–11 | DO NOT USE — flash |
| 0/2/12/15 | avoid — strapping (GPIO5 strap is satisfied by the OneWire pull-up) |
| 1/3 | UART0 / USB serial |

## Power notes

- GC9A01 logic and DS18B20 run at 3.3 V.
- This display's backlight is hardwired on (no BLK control pin).
- For a permanent build, feed clean 3.3 V (or 5 V into VIN) from a dedicated
  regulator rather than USB; add 100 nF decoupling at the display and at any
  DS18B20 on a long cable.
- Actuator loads (heaters/solenoids/lights) get isolated power, never the ESP32
  rail.
