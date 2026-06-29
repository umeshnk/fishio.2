#pragma once

#include <Arduino.h>

// GC9A01 240x240 round IPS LCD over SPI (pins from Config.h).
// Renders an aquarium dashboard in one of two layouts selected by DISPLAY_MODE:
//   DISPLAY_CAROUSEL — one tank at a time, large, auto-cycling with a status ring
//   DISPLAY_LIST     — all tanks as rows on one screen
class DisplayManager {
public:
  enum Status { STATUS_OK, STATUS_ALERT, STATUS_ERROR };

  struct TankReading {
    const char* name;
    float tempC;
    Status status;
  };

  static bool begin();

  // Simple full-screen message (boot/status screens).
  static void displayText(const char* text);

  // Push the latest readings; redraws immediately.
  static void showReadings(const TankReading* readings, int count);

  // Call frequently from loop(); advances the carousel between data updates.
  static void tick();
};
