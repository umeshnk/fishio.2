#include "Buzzer.h"

// LEDC config: 8-bit resolution, 50% duty for a clean square-wave tone.
static const uint8_t LEDC_RES_BITS = 8;
static const uint32_t LEDC_DUTY = 128;  // ~50% of 8-bit range

Buzzer::Buzzer(uint8_t pin, uint8_t ledcChannel)
  : _pin(pin), _channel(ledcChannel) {}

void Buzzer::begin() {
  ledcSetup(_channel, 2000, LEDC_RES_BITS);
  ledcAttachPin(_pin, _channel);
  ledcWrite(_channel, 0);  // silent
}

void Buzzer::tone(uint32_t freq, uint32_t durationMs) {
  ledcWriteTone(_channel, freq);
  ledcWrite(_channel, LEDC_DUTY);
  delay(durationMs);
  noTone();
}

void Buzzer::noTone() {
  ledcWrite(_channel, 0);
  ledcWriteTone(_channel, 0);
}

void Buzzer::playMelody() {
  const uint16_t notes[] = { 523, 659, 784, 1047 };  // C5 E5 G5 C6
  for (uint8_t i = 0; i < 4; i++) {
    tone(notes[i], 120);
    delay(40);
  }
}

void Buzzer::alert(int beeps) {
  for (int i = 0; i < beeps; i++) {
    tone(2000, 150);
    delay(120);
  }
}
