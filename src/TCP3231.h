#ifndef TCP3231_H
#define TCP3231_H

#include <Arduino.h>
#include <TCP1819.h>

class TCP3231 {
public:
  struct DateTime {
    uint16_t year;     // 2000..2199
    uint8_t month;     // 1..12
    uint8_t day;       // 1..31
    uint8_t hour;      // 0..23
    uint8_t minute;    // 0..59
    uint8_t second;    // 0..59
    uint8_t dayOfWeek; // 1..7, user-defined but sequential
  };

  struct Status {
    bool oscillatorStopped; // OSF
    bool busy;              // BSY
    bool alarm1Flag;        // A1F
    bool alarm2Flag;        // A2F
    bool en32kHz;           // EN32kHz
  };

  explicit TCP3231(BBI2C& bus, uint8_t address = 0x68);

  bool begin();
  bool readTime(DateTime& dt);
  bool setTime(const DateTime& dt);
  bool readStatus(Status& status);
  bool clearOscillatorStopFlag();

  const char* errorString() const;

private:
  bool readRegister(uint8_t reg, uint8_t& value);
  bool readRegisters(uint8_t startReg, uint8_t* data, uint8_t len);
  bool writeRegister(uint8_t reg, uint8_t value);
  bool writeRegisters(uint8_t startReg, const uint8_t* data, uint8_t len);

  static uint8_t decToBcd(uint8_t value);
  static uint8_t bcdToDec(uint8_t value);
  static bool isLeapYear(uint16_t year);
  static uint8_t daysInMonth(uint16_t year, uint8_t month);
  static bool isValidDateTime(const DateTime& dt);

  void setError(const char* message);

  BBI2C& bus_;
  uint8_t address_;
  const char* lastError_;
};

#endif