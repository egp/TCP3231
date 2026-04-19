# TCP3231

TCP3231 is an Arduino library for the Maxim / Analog Devices DS3231 real-time clock.

The library uses a caller-owned `BBI2C` bus supplied by `TCP1819`. It does not own the bus and does not expose `Wire` in its public API.

## Features

TCP3231 provides a small DS3231-focused API to:

- initialize and probe the RTC
- read date and time
- set date and time
- read status flags
- clear the oscillator-stop flag

## Dependency

TCP3231 depends on `TCP1819` for:

- `BBI2C`
- `I2CInit(...)`
- low-level I2C read/write helpers

Install both libraries before building examples:

- `TCP3231`
- `TCP1819`

Example layout:

- `~/Documents/Arduino/libraries/TCP3231`
- `~/Documents/Arduino/libraries/TCP1819`

## Public API

class TCP3231 {
public:
  struct DateTime {
    uint16_t year;      // 2000..2199
    uint8_t month;      // 1..12
    uint8_t day;        // 1..31
    uint8_t hour;       // 0..23
    uint8_t minute;     // 0..59
    uint8_t second;     // 0..59
    uint8_t dayOfWeek;  // 1..7, user-defined but sequential
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
};

## Minimal example

#include <string.h>
#include <TCP1819.h>
#include <TCP3231.h>

static const int kRtcSdaPin = 12;
static const int kRtcSclPin = 13;
static const uint32_t kI2CFrequencyHz = 100000UL;

BBI2C rtcBus{};
TCP3231 rtc(rtcBus);

void setup() {
  Serial.begin(115200);

  memset(&rtcBus, 0, sizeof(rtcBus));
  rtcBus.bWire = 0;
  rtcBus.iSDA = kRtcSdaPin;
  rtcBus.iSCL = kRtcSclPin;
  I2CInit(&rtcBus, kI2CFrequencyHz);

  if (!rtc.begin()) {
    Serial.print("rtc.begin() failed: ");
    Serial.println(rtc.errorString());
    while (true) {
    }
  }

  TCP3231::DateTime now{};
  if (!rtc.readTime(now)) {
    Serial.print("readTime failed: ");
    Serial.println(rtc.errorString());
    while (true) {
    }
  }

  Serial.print(now.year);
  Serial.print('-');
  Serial.print(now.month);
  Serial.print('-');
  Serial.println(now.day);
}

void loop() {
}

## Status flags

`readStatus()` returns:

- `oscillatorStopped` for OSF
- `busy` for BSY
- `alarm1Flag` for A1F
- `alarm2Flag` for A2F
- `en32kHz` for EN32kHz

`clearOscillatorStopFlag()` clears OSF.

## Included example

See:

- `examples/SetAndReadTime/SetAndReadTime.ino`

It demonstrates:

- bus setup
- RTC probe
- status readout
- optional time set
- round-trip verification
- periodic time printing

## Notes

- DS3231 default 7-bit address is `0x68`
- `dayOfWeek` is user-defined but must be sequential `1..7`
- on failure, methods return `false`; call `errorString()` for a short diagnostic

## Local development

Typical commands:

make ci TCP1819_SRC=../TCP1819
make compile FQBN=arduino:avr:uno TCP1819_SRC=../TCP1819
make clean

## CI

GitHub Actions compiles the example on:

- Arduino Uno
- Arduino Uno R4 Minima
- Arduino Uno R4 WiFi

## License

MIT