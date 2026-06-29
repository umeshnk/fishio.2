#include "DisplayManager.h"

#include <Arduino_GFX_Library.h>
#include "Config.h"

// VSPI data bus. GC9A01 is write-only, so MISO is not connected.
static Arduino_DataBus* bus =
    new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCLK, TFT_MOSI, GFX_NOT_DEFINED);

// GC9A01 panel: rotation 0, IPS = true.
static Arduino_GFX* gfx = new Arduino_GC9A01(bus, TFT_RST, 0, true);

static const int16_t CX = 120;  // screen centre (240x240)
static const int16_t CY = 120;
static const uint16_t GRAY = 0x8410;
static const uint16_t DKGRAY = 0x4208;

// Cached copy of the latest readings so the carousel can redraw on its own.
struct Entry {
  char name[20];
  float tempC;
  uint8_t status;
};
static Entry s_entries[MAX_SENSORS];
static int s_count = 0;
static int s_index = 0;
static unsigned long s_lastAdvance = 0;

static uint16_t statusColor(uint8_t s) {
  switch (s) {
    case DisplayManager::STATUS_OK: return GREEN;
    case DisplayManager::STATUS_ALERT: return RED;
    default: return YELLOW;  // STATUS_ERROR
  }
}

// Print a string centred horizontally on CX and vertically on cy.
static void printCentered(const char* s, int16_t cy, uint8_t size, uint16_t color) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->setTextSize(size);
  gfx->getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  gfx->setTextColor(color);
  gfx->setCursor(CX - (int16_t)w / 2 - x1, cy - (int16_t)h / 2 - y1);
  gfx->print(s);
}

static void drawNoSensors() {
  gfx->fillScreen(BLACK);
  printCentered("No sensors", CY, 2, GRAY);
}

static void drawCarousel(int idx) {
  const Entry& e = s_entries[idx];
  uint16_t col = statusColor(e.status);

  gfx->fillScreen(BLACK);

  // Thick status ring around the rim.
  for (int16_t r = 119; r >= 113; r--) gfx->drawCircle(CX, CY, r, col);

  char name[16];
  strncpy(name, e.name, sizeof(name));
  name[sizeof(name) - 1] = '\0';
  printCentered(name, 62, 2, WHITE);

  char num[12];
  if (e.status == DisplayManager::STATUS_ERROR) strcpy(num, "--");
  else dtostrf(e.tempC, 0, 1, num);
  printCentered(num, CY, 5, col);

  // Degree ring just past the top-right of the big number.
  int16_t x1, y1;
  uint16_t w, h;
  gfx->setTextSize(5);
  gfx->getTextBounds(num, 0, 0, &x1, &y1, &w, &h);
  gfx->drawCircle(CX + (int16_t)w / 2 + 12, CY - (int16_t)h / 2 + 4, 4, col);

  // Page indicator: dots if they fit, otherwise "i/n".
  if (s_count > 1) {
    if (s_count <= 9) {
      int total = s_count * 14;
      int sx = CX - total / 2 + 7;
      for (int k = 0; k < s_count; k++) {
        if (k == idx) gfx->fillCircle(sx + k * 14, 196, 4, col);
        else gfx->drawCircle(sx + k * 14, 196, 3, DKGRAY);
      }
    } else {
      char p[12];
      snprintf(p, sizeof(p), "%d/%d", idx + 1, s_count);
      printCentered(p, 196, 2, WHITE);
    }
  }
}

static void drawList() {
  gfx->fillScreen(BLACK);
  printCentered("FISHIO", 38, 1, GRAY);

  const int rows = s_count < 4 ? s_count : 4;
  const int16_t ys[4] = { 78, 108, 138, 168 };
  for (int k = 0; k < rows; k++) {
    const Entry& e = s_entries[k];

    char name[10];
    strncpy(name, e.name, sizeof(name));
    name[sizeof(name) - 1] = '\0';

    char tstr[10];
    if (e.status == DisplayManager::STATUS_ERROR) strcpy(tstr, "--");
    else dtostrf(e.tempC, 0, 1, tstr);

    char row[24];
    snprintf(row, sizeof(row), "%s  %s", name, tstr);

    int16_t x1, y1;
    uint16_t w, h;
    gfx->setTextSize(2);
    gfx->getTextBounds(row, 0, 0, &x1, &y1, &w, &h);
    int16_t leftX = CX - (int16_t)w / 2 - x1;

    gfx->fillCircle(leftX - 12, ys[k], 5, statusColor(e.status));
    gfx->setTextColor(WHITE);
    gfx->setCursor(leftX, ys[k] - (int16_t)h / 2 - y1);
    gfx->print(row);
  }

  if (s_count > rows) {
    char more[12];
    snprintf(more, sizeof(more), "+%d more", s_count - rows);
    printCentered(more, 200, 1, GRAY);
  }
}

static void renderCurrent() {
  if (s_count == 0) {
    drawNoSensors();
    return;
  }
#if DISPLAY_MODE == DISPLAY_LIST
  drawList();
#else
  drawCarousel(s_index);
#endif
}

bool DisplayManager::begin() {
  if (!gfx->begin(TFT_SPI_SPEED)) {
    Serial.println("Display init failed!");
    return false;
  }

  // No BLK pin on this module — backlight is hardwired on.
  gfx->fillScreen(BLACK);
  return true;
}

void DisplayManager::displayText(const char* text) {
  gfx->fillScreen(BLACK);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  // The round panel is 240x240; nudge in from the edge so text clears the bezel.
  gfx->setCursor(20, 40);
  gfx->println(text);
}

void DisplayManager::showReadings(const TankReading* readings, int count) {
  if (count > MAX_SENSORS) count = MAX_SENSORS;
  s_count = count;
  for (int i = 0; i < count; i++) {
    strncpy(s_entries[i].name, readings[i].name ? readings[i].name : "", sizeof(s_entries[i].name));
    s_entries[i].name[sizeof(s_entries[i].name) - 1] = '\0';
    s_entries[i].tempC = readings[i].tempC;
    s_entries[i].status = (uint8_t)readings[i].status;
  }
  if (s_index >= s_count) s_index = 0;
  s_lastAdvance = millis();
  renderCurrent();
}

void DisplayManager::tick() {
#if DISPLAY_MODE == DISPLAY_CAROUSEL
  if (s_count > 1 && millis() - s_lastAdvance >= CAROUSEL_INTERVAL_MS) {
    s_index = (s_index + 1) % s_count;
    s_lastAdvance = millis();
    drawCarousel(s_index);
  }
#endif
}
