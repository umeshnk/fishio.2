#pragma once

#include <Arduino.h>

// GC9A01 240x240 round IPS LCD over SPI (pins from Config.h).
class DisplayManager {
public:
  static bool begin();
  static void displayText(const String& text);
  static void displayText(const char* text);
  static void clear();
};
