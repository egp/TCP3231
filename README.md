# TCP3231

Arduino library for the Maxim / Analog Devices DS3231 real-time clock using a caller-owned `BBI2C` bus.

## What it does

TCP3231 provides a small API for the DS3231:

- initialize and probe the RTC
- read date and time
- set date and time
- read status flags
- clear the oscillator-stop flag

The library does not own the I2C bus. You create and initialize a `BBI2C` bus, then pass it to `TCP3231`.

## Dependency

TCP3231 uses `TCP1819` for the `BBI2C` type and low-level I2C helpers.

## Public API

```cpp
#include <Arduino.h>
#include <TCP1819.h>
#include <TCP3231.h>

class TCP3231 {
public:
  struct DateTime {
    uint16_t year;     // 2000..2199
    uint8_t month;     // 1..12
    uint8_t day;       // 1..31
    uint8_t hour;      // 0..23
    uint8_t minute;    // 0..59
    uint8_t second;    // 0..59
    uint8_t dayOfWeek; // 1..7
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
```

## Minimal example

```cpp
#include <Arduino.h>
#include <TCP1819.h>
#include <TCP3231.h>

static const int kRtcSdaPin = 12;
static const int kRtcSclPin = 13;
static const uint32_t kI2CFrequencyHz = 100000;

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
    while (true) {}
  }

  TCP3231::DateTime now{};
  if (!rtc.readTime(now)) {
    Serial.print("readTime failed: ");
    Serial.println(rtc.errorString());
    while (true) {}
  }

  Serial.print(now.year);
  Serial.print('-');
  Serial.print(now.month);
  Serial.print('-');
  Serial.println(now.day);
}

void loop() {
}
```

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

It shows:

- bus setup
- RTC probe
- status readout
- optional time set
- round-trip verification
- periodic time printing

## Notes

- DS3231 default 7-bit address is `0x68`.
- `dayOfWeek` is user-defined but must be sequential `1..7`.
- On failure, methods return `false`; call `errorString()` for a short diagnostic.

## License

MIT
