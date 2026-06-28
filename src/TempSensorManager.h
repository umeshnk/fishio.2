#pragma once

#include <Arduino.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "Config.h"

class TempSensorManager {
public:
  TempSensorManager(int dataPin);

  void begin();
  int getCount();
  void requestTemperatures();  // Trigger one bus-wide conversion per cycle
  float getTempC(int index);
  String getId(int index);

private:
  OneWire _oneWire;
  DallasTemperature _sensors;
  DeviceAddress _addresses[MAX_SENSORS];
  String _ids[MAX_SENSORS];
  int _count;

  String formatAddress(DeviceAddress address);
};
