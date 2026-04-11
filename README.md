# DS3231RTC

Arduino library for the Maxim/Analog Devices DS3231 RTC using `BitBang_I2C` and a caller-owned `BBI2C` bus object.

## Status

Initial library scaffold. Core RTC implementation is in progress.

## Goals

- Use `BitBang_I2C` instead of exposing `Wire` in the public API
- Support caller-owned `BBI2C` bus instances
- Read and set date/time
- Read RTC status flags
- Read onboard temperature
- Keep the API small and predictable

## Dependency

This library depends on `BitBang_I2C`.

## Installation

1. Install `BitBang_I2C`
2. Copy this library into your Arduino libraries folder
3. Restart the Arduino IDE

## Planned API

```cpp
#include <BitBang_I2C.h>
#include <DS3231RTC.h>

BBI2C rtcBus{};
DS3231RTC rtc(rtcBus);