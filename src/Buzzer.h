#pragma once

#include <Arduino.h>

// Passive piezo buzzer driven by the ESP32 LEDC peripheral (ESP32 has no tone()).
class Buzzer {
public:
  Buzzer(uint8_t pin, uint8_t ledcChannel = 0);

  void begin();
  void playMelody();        // Startup chime
  void alert(int beeps);    // n short alert beeps
  void tone(uint32_t freq, uint32_t durationMs);
  void noTone();

private:
  uint8_t _pin;
  uint8_t _channel;
};
