#include "TempSensorManager.h"

TempSensorManager::TempSensorManager(int dataPin)
  : _oneWire(dataPin), _sensors(&_oneWire), _count(0) {}

void TempSensorManager::begin() {
  _sensors.begin();
  _count = _sensors.getDeviceCount();
  Serial.printf("Found %d DS18B20 sensor(s)\n", _count);

  for (int i = 0; i < _count && i < MAX_SENSORS; i++) {
    if (_sensors.getAddress(_addresses[i], i)) {
      _ids[i] = formatAddress(_addresses[i]);
      Serial.printf("Sensor %d initialized (ID: %s)\n", i, _ids[i].c_str());
    }
  }
}

int TempSensorManager::getCount() {
  return _count;
}

float TempSensorManager::getTempC(int index) {
  _sensors.requestTemperatures();
  return _sensors.getTempC(_addresses[index]);
}

void TempSensorManager::printAddresses() {
  for (int i = 0; i < _count; i++) {
    Serial.print("Sensor ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(_sensors.getTempC(_addresses[i]));
  }
}

String TempSensorManager::getId(int index) {
  if (index >= _count) return "";
  return _ids[index];
}

String TempSensorManager::formatAddress(DeviceAddress address) {
  char id[17];
  for (uint8_t i = 0; i < 8; i++) sprintf(&id[i * 2], "%02X", address[i]);
  return String(id);
}
