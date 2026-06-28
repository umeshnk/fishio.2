#include "DisplayManager.h"

#include <Arduino_GFX_Library.h>
#include "Config.h"

// VSPI data bus. GC9A01 is write-only, so MISO is not connected.
static Arduino_DataBus* bus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, GFX_NOT_DEFINED);

// GC9A01 panel: rotation 0, IPS = true.
static Arduino_GFX* gfx = new Arduino_GC9A01(bus, TFT_RST, 0, true);

bool DisplayManager::begin() {
  if (!gfx->begin(TFT_SPI_SPEED)) {
    Serial.println("Display init failed!");
    return false;
  }

  // No BLK pin on this module — backlight is hardwired on.
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  return true;
}

void DisplayManager::displayText(const String& text) {
  displayText(text.c_str());
}

void DisplayManager::displayText(const char* text) {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  // The round panel is 240x240; nudge in from the edge so text clears the bezel.
  gfx->setCursor(20, 40);
  gfx->println(text);
}

void DisplayManager::clear() {
  gfx->fillScreen(BLACK);
}
