# fishio.2 — PCB design (carrier board)

A 2-layer carrier PCB for the fishio.2 aquarium monitor. It sockets an
**ESP32-WROOM-32D DevKitC** on female headers and breaks every signal in
[HARDWARE.md](../HARDWARE.md) out to proper connectors, with the reserved
relay / analog / I²C / expansion provisions populated as footprints.

This document is the authoritative source for the schematic netlist, BOM,
layout, and fab settings. Pin assignments **must** match
[HARDWARE.md](../HARDWARE.md) and [Config.h](../src/Config.h); change all three
together.

## Design decision: carrier vs. integrated module

| | Carrier board (this design) | Integrated WROOM-32D |
| --- | --- | --- |
| ESP32 | DevKitC on 2× female headers | Bare module soldered down |
| USB / flashing | DevKit's onboard CP2102 + auto-reset | Must add USB-UART + EN/IO0 auto-program |
| Antenna | Handled by DevKit, overhangs board edge | RF keepout + impedance you must get right |
| Risk / effort | Low — no RF or USB design | High — needs RF review, more SMT |
| Repairability | Pop the DevKit out and replace | Rework the module |

The repo specifies a **DevKit**, so this revision is the carrier board: the
fastest path to a reliable soldered build with zero RF/USB risk. An integrated
WROOM-32D layout is a sensible *rev B* once the carrier is proven — the netlist
below ports over unchanged except for the added USB-UART/auto-program block.

## Board

| | |
| --- | --- |
| Size | 80 × 60 mm, rounded corners (R3) |
| Layers | 2 (top signal + components, bottom ground pour) |
| Stackup | 1.6 mm FR4, 1 oz copper, HASL or ENIG |
| Mounting | 4× M3 holes, Ø3.2 mm, 4 mm from each corner, Ø7 mm keepout |
| Input | 5 V via 2-pin screw terminal (permanent supply); DevKit USB stays accessible for flashing |

## Reference designators

| Ref | Part | Notes |
| --- | --- | --- |
| U1 | ESP32-WROOM-32D DevKitC | socketed on J0A/J0B female headers |
| J0A / J0B | 1×19 (or 1×15) female headers, 2.54 mm | match your DevKit's row pitch — measure it |
| J1 | 2-pin 5.08 mm screw terminal | +5V, GND input |
| J2 | 7-pin header, 2.54 mm | GC9A01 display, module pin order |
| J3 | 3-pin 3.5 mm screw terminal | DS18B20 bus (DATA, VDD, GND) |
| J4 | 5-pin header, 2.54 mm | relay outputs + 5V + GND |
| J5 | 6-pin header, 2.54 mm | analog probes (ADC1) + 3V3 + GND |
| J6 | 4-pin JST-SH (Qwiic/STEMMA order) | I²C expansion: GND, 3V3, SDA, SCL |
| J7 | 4-pin header, 2.54 mm | spare GPIO13, GPIO25, GPIO19 + GND |
| LS1 | 12 mm passive piezo, TH | buzzer |
| Q1 | AO3401 P-MOSFET, SOT-23 | reverse-polarity protection on +5V |
| Q2 | MMBT3904 NPN, SOT-23 | buzzer low-side driver (**DNP** — fit only for loud/magnetic) |
| U2 | AP2112K-3.3 LDO, SOT-23-5 | **DNP** — dedicated 3V3 option (bypasses DevKit reg) |
| C1 | 470 µF / 10 V electrolytic | 5V bulk |
| C2 | 100 µF / 10 V electrolytic | 3V3 bulk |
| C3 | 100 nF | 3V3 decoupling at display connector |
| C4 | 100 nF | DS18B20 VDD–GND |
| C5, C6 | 1 µF | LDO in/out (only if U2 fitted) |
| R1 | 4.7 kΩ | DS18B20 OneWire pull-up to 3V3 |
| R2 | 100 Ω | buzzer series (or 0 Ω link) |
| R3 | 1 kΩ | Q2 base (only if Q2 fitted) |
| R4–R6 | 10 kΩ | relay GPIO pull-downs (32, 33, 27) |
| R7, R8 | 4.7 kΩ | I²C pull-ups to 3V3 (**DNP** if module carries its own) |
| D1 | SS34 Schottky | buzzer flyback (only if magnetic buzzer fitted) |

## Netlist (by signal)

All GPIO numbers are ESP32 logical pins, identical to
[Config.h](../src/Config.h).

### Power

```
J1.1  +5V_IN ── Q1 (P-FET reverse prot.) ── +5V ── C1(470u) ── U1.VIN(5V)
J1.2  GND ───────────────────────────────────── GND
U1.3V3 ── +3V3 ── C2(100u) ── C3(100n)         (DevKit's onboard reg powers 3V3)

# DNP dedicated-regulator option (cut SJ1, fit U2):
+5V ── C5(1u) ── U2.IN ; U2.OUT ── C6(1u) ── +3V3 ; U2.EN ── +5V
```

`SJ1` is a solder jumper on the +3V3 net. Default = closed (3V3 from DevKit).
To run a dedicated LDO instead, cut SJ1 and fit U2/C5/C6.

### GC9A01 display — J2 (module header order)

| J2 pin | Net | ESP32 GPIO |
| --- | --- | --- |
| 1 VCC | +3V3 | — |
| 2 GND | GND | — |
| 3 SCL | TFT_SCLK | GPIO18 |
| 4 SDA | TFT_MOSI | GPIO23 |
| 5 DC | TFT_DC | GPIO16 |
| 6 CS | TFT_CS | GPIO4 |
| 7 RST | TFT_RST | GPIO17 |

C3 (100 nF) sits across J2.1–J2.2. SPI runs at 8 MHz today; the soldered board
should tolerate higher — keep these five traces short and grouped.

### DS18B20 — J3

| J3 pin | Net | ESP32 GPIO |
| --- | --- | --- |
| 1 DATA | ONE_WIRE_BUS | GPIO5 |
| 2 VDD | +3V3 | — |
| 3 GND | GND | — |

R1 (4.7 kΩ) DATA→+3V3. C4 (100 nF) VDD→GND. Multiple probes wire in parallel
on the one terminal. GPIO5 is a strapping pin held HIGH at boot by R1 — safe.

### Buzzer — LS1

```
Default (passive piezo): GPIO26 ── R2(100R) ── LS1 ── GND
Loud/magnetic option:    GPIO26 ── R3(1k) ── Q2.base ; LS1 between +5V and Q2.collector ; D1 flyback across LS1
```

### Relays (reserved) — J4

| J4 pin | Net | ESP32 GPIO |
| --- | --- | --- |
| 1 | Relay 1 | GPIO32 |
| 2 | Relay 2 | GPIO33 |
| 3 | Relay 3 | GPIO27 |
| 4 | +5V | — |
| 5 | GND | — |

R4/R5/R6 (10 kΩ) pull each relay GPIO to GND so a floating pin at reset can't
switch a heater on. Drive opto-isolated relay/SSR boards only; actuators get
their own PSU sharing GND.

### Analog probes (reserved, ADC1) — J5

| J5 pin | Net | ESP32 GPIO | ADC |
| --- | --- | --- | --- |
| 1 | pH | GPIO36 (VP) | ADC1_0 |
| 2 | TDS | GPIO39 (VN) | ADC1_3 |
| 3 | aux | GPIO34 | ADC1_6 |
| 4 | aux | GPIO35 | ADC1_7 |
| 5 | +3V3 | — | — |
| 6 | GND | — | — |

Each input has DNP RC-filter pads (series R + cap to GND) for a low-pass if a
probe is added. Prefer an external ADS1115 on J6 over the raw ADC.

### I²C expansion (reserved) — J6

| J6 pin | Net | ESP32 GPIO |
| --- | --- | --- |
| 1 | GND | — |
| 2 | +3V3 | — |
| 3 | SDA | GPIO21 |
| 4 | SCL | GPIO22 |

R7/R8 (4.7 kΩ) pull-ups, DNP. For ADS1115 (precise ADC), MCP23017 (relay
bank), RTC, etc.

### Spare — J7

GPIO13, GPIO25, GPIO19 (free), + GND. Left as a header for future use.

## Optimized placement

Floorplan (top view), DevKit centered with its antenna overhanging the **left**
edge:

```
 ┌────────────────────────────────────────────────────────┐
 │ ●mtg                                              mtg●  │
 │  ┌──────┐    ┌───────── U1: ESP32 DevKit ───────┐      │
 │  │ PWR  │    │  ▒▒▒ antenna keepout (no copper)  │  J2  │ ← display, top edge
 │  │ J1   │    │                                    │ disp │   near GPIO4/16/17/18/23
 │  │Q1 C1 │    │   J0A ░░░░░░░░░░░░░░░░░░░░░ J0B    │      │
 │  │ U2*  │    │       (female header socket)       │  J3  │ ← DS18B20 + R1
 │  └──────┘    └────────────────────────────────────┘     │
 │              LS1        J4 relays    J5 analog   J6 J7   │
 │ ●mtg            (digital/switching kept from analog) mtg●│
 └────────────────────────────────────────────────────────┘
```

Placement rationale:

- **Antenna keepout** — orient the DevKit so its PCB antenna hangs over the
  board's left edge. No copper, no pour, no traces under the antenna footprint
  (~15 mm). Notch the board outline under it if the DevKit doesn't overhang far
  enough.
- **Display connector J2 on the top edge**, hard against the GPIO4/16/17/18/23
  side of the socket, so the SPI group (SCLK, MOSI, DC, CS, RST) routes as five
  short parallel traces with no detours.
- **DS18B20 J3 near J2** on the same side (GPIO5 lives on the display header
  side). R1/C4 right at the terminal.
- **Power block left**, away from analog. Bulk cap C1 next to VIN.
- **Analog header J5 far from buzzer and relays** — keep ADC traces clear of
  the buzzer PWM and relay switching nodes. Short, direct runs to GPIO34–39.
- **Buzzer LS1** bottom-left, mechanically clear of connectors.

## Routing & grounding

- **Bottom layer = solid ground pour**, stitched to the top pour with vias
  every ~10 mm and around connectors. The only gap is the antenna keepout.
- Power on the top layer: **+5V 0.6 mm (24 mil)**, **+3V3 0.5 mm (20 mil)**,
  signals **0.25 mm (10 mil)**.
- Keep the SPI group short and together; don't run analog traces parallel to
  them or to the buzzer net.
- Star-ground the analog header to the pour at a single point if probes are
  used; keep relay/buzzer return currents off that path.
- Decoupling caps (C3 at display, C4 at DS18B20, module 100 nF) as close to
  their pins as the footprint allows.

## Fab settings (JLCPCB economy / 2-layer)

| Rule | Value |
| --- | --- |
| Min trace / space | 0.2 mm / 0.2 mm (8 mil) |
| Min via | 0.3 mm drill / 0.6 mm pad |
| Min annular ring | 0.13 mm |
| Copper | 1 oz |
| Surface finish | HASL (lead-free) or ENIG |
| Silkscreen | label every connector with pin 1, net name, and GPIO |

## Assembly notes

- Socket the DevKit on female headers — never solder it directly, so it can be
  reflashed/replaced and the antenna stays clear.
- DNP by default: Q2, D1, U2, C5, C6, R7, R8 (and J5's RC pads). Populate per
  the feature you're enabling.
- Mark SJ1 state in silkscreen ("3V3: DevKit | LDO").
- Actuator loads (heaters/solenoids/lights) never touch the ESP32 rail — they
  get isolated power with a common ground, and DC solenoids need a flyback
  diode at the relay board.

## What this is not

This is the design intent (schematic netlist + placement + fab rules), not
Gerbers. Drop the netlist into KiCad/EasyEDA to capture the schematic, then lay
out per the floorplan above. I can generate KiCad project files (schematic +
board) from this on request.
