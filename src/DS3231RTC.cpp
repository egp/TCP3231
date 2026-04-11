#include "DS3231RTC.h"

DS3231RTC::DS3231RTC(BBI2C& bus, uint8_t address)
    : bus_(bus), address_(address), lastError_("not implemented") {}

bool DS3231RTC::begin() {
  lastError_ = "not implemented";
  return false;
}

bool DS3231RTC::readTime(DateTime& dt) {
  (void)dt;
  lastError_ = "not implemented";
  return false;
}

bool DS3231RTC::setTime(const DateTime& dt) {
  (void)dt;
  lastError_ = "not implemented";
  return false;
}

bool DS3231RTC::readStatus(Status& status) {
  (void)status;
  lastError_ = "not implemented";
  return false;
}

bool DS3231RTC::clearOscillatorStopFlag() {
  lastError_ = "not implemented";
  return false;
}

bool DS3231RTC::readTemperatureC(float& tempC) {
  tempC = 0.0f;
  lastError_ = "not implemented";
  return false;
}

const char* DS3231RTC::errorString() const {
  return lastError_;
}