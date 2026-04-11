#ifndef DS3231RTC_H
#define DS3231RTC_H

#include <Arduino.h>
#include <BitBang_I2C.h>

class DS3231RTC {
public:
  struct DateTime {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t dayOfWeek;
  };

  struct Status {
    bool oscillatorStopped;
    bool busy;
    bool alarm1Flag;
    bool alarm2Flag;
    bool en32kHz;
  };

  explicit DS3231RTC(BBI2C& bus, uint8_t address = 0x68);

  bool begin();
  bool readTime(DateTime& dt);
  bool setTime(const DateTime& dt);
  bool readStatus(Status& status);
  bool clearOscillatorStopFlag();
  bool readTemperatureC(float& tempC);

  const char* errorString() const;

private:
  BBI2C& bus_;
  uint8_t address_;
  const char* lastError_;
};

#endif