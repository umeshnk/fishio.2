// diag_display.cpp — standalone GC9A01 display diagnostic.
//
// Build/flash with the "diag" environment (does NOT include the main firmware):
//   pio run -e diag -t upload && pio device monitor -e diag
//
// It talks to the panel directly (bypassing DisplayManager) and cycles
// full-screen color fills, geometry, and text so you can see how far the SPI
// signal is getting. Watch the serial log alongside the screen.
//
// If the screen stays blank, lower the SPI speed in platformio.ini:
//   [env:diag] build_flags = -DDIAG_SPI_SPEED=8000000

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "Config.h"

#ifndef DIAG_SPI_SPEED
#define DIAG_SPI_SPEED 24000000  // 24 MHz; drop to 8000000 if unstable/blank
#endif

static Arduino_DataBus* bus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, GFX_NOT_DEFINED);
static Arduino_GFX* gfx = new Arduino_GC9A01(bus, TFT_RST, 0 /*rotation*/, true /*IPS*/);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("=== GC9A01 display diagnostic ===");
  Serial.printf("Pins -> SCLK=%d MOSI=%d DC=%d CS=%d RST=%d (no BLK)\n",
                TFT_SCLK, TFT_MOSI, TFT_DC, TFT_CS, TFT_RST);
  Serial.printf("SPI speed: %d Hz\n", DIAG_SPI_SPEED);

  bool ok = gfx->begin(DIAG_SPI_SPEED);
  Serial.printf("gfx->begin() = %s\n", ok ? "true" : "false");
  if (!ok) {
    Serial.println("begin() returned false — check power and the RST/DC/CS pins.");
  }

  gfx->fillScreen(RED);
  Serial.println("Filled RED — if you see red, SPI + init are working.");
}

void loop() {
  const int16_t w = gfx->width();
  const int16_t h = gfx->height();

  struct ColorStep { const char* name; uint16_t color; };
  const ColorStep steps[] = {
    { "RED", RED }, { "GREEN", GREEN }, { "BLUE", BLUE },
    { "WHITE", WHITE }, { "BLACK", BLACK },
  };
  for (const ColorStep& s : steps) {
    gfx->fillScreen(s.color);
    Serial.printf("Fill %s\n", s.name);
    delay(1000);
  }

  // Geometry on black: border, diagonals, concentric circles (round panel).
  gfx->fillScreen(BLACK);
  gfx->drawRect(0, 0, w, h, WHITE);
  gfx->drawLine(0, 0, w - 1, h - 1, RED);
  gfx->drawLine(0, h - 1, w - 1, 0, RED);
  gfx->drawCircle(w / 2, h / 2, h / 2 - 2, GREEN);
  gfx->drawCircle(w / 2, h / 2, h / 4, BLUE);
  Serial.println("Drew border, diagonals, circles");
  delay(1500);

  // Text on black.
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(3);
  gfx->setCursor(45, h / 2 - 30);
  gfx->println("FISHIO");
  gfx->setTextColor(YELLOW);
  gfx->setTextSize(2);
  gfx->setCursor(55, h / 2 + 10);
  gfx->println("display OK");
  Serial.println("Drew text");
  delay(3000);
}
