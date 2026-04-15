#include <Arduino.h>
#include <TCP1819.h> // cloned into https://github.com/egp/TCP1819
#include <TCP3231.h> // 

static const int kRtcSdaPin = 12;
static const int kRtcSclPin = 13;

static const uint32_t kI2CFrequencyHz = 100000;

// Operator-editable target time/date.
static const bool kSetTimeOnBoot = false;
static const uint16_t kSetYear = 2026;
static const uint8_t kSetMonth = 4;
static const uint8_t kSetDay = 15;
static const uint8_t kSetHour = 9;
static const uint8_t kSetMinute = 40;
static const uint8_t kSetSecond = 0;
static const uint8_t kSetDayOfWeek = 7;  // 1..7, user-defined but sequential

// Optional: clear OSF after a successful write/read cycle.
static const bool kClearOscillatorStopFlagAfterSet = false;

BBI2C rtcBus{};
TCP3231 rtc(rtcBus);

static void printTwoDigits(uint8_t value) {
  if (value < 10) {
    Serial.print('0');
  }
  Serial.print(value);
}

static void printFourDigits(uint16_t value) {
  if (value < 1000) Serial.print('0');
  if (value < 100) Serial.print('0');
  if (value < 10) Serial.print('0');
  Serial.print(value);
}

static void printDateTime(const TCP3231::DateTime& dt) {
  printFourDigits(dt.year);
  Serial.print('-');
  printTwoDigits(dt.month);
  Serial.print('-');
  printTwoDigits(dt.day);
  Serial.print(' ');
  printTwoDigits(dt.hour);
  Serial.print(':');
  printTwoDigits(dt.minute);
  Serial.print(':');
  printTwoDigits(dt.second);
  Serial.print("  DOW=");
  Serial.print(dt.dayOfWeek);
}

static void printStatus(const TCP3231::Status& status) {
  Serial.print("OSF=");
  Serial.print(status.oscillatorStopped ? "1" : "0");
  Serial.print(" BSY=");
  Serial.print(status.busy ? "1" : "0");
  Serial.print(" A1F=");
  Serial.print(status.alarm1Flag ? "1" : "0");
  Serial.print(" A2F=");
  Serial.print(status.alarm2Flag ? "1" : "0");
  Serial.print(" EN32kHz=");
  Serial.println(status.en32kHz ? "1" : "0");
}

static bool sameDateTime(const TCP3231::DateTime& a, const TCP3231::DateTime& b) {
  return a.year == b.year && a.month == b.month && a.day == b.day && a.hour == b.hour && a.minute == b.minute && a.second == b.second && a.dayOfWeek == b.dayOfWeek;
}

static void printCurrentStatus() {
  TCP3231::Status status{};
  if (!rtc.readStatus(status)) {
    Serial.print("readStatus failed: ");
    Serial.println(rtc.errorString());
    return;
  }

  Serial.print("RTC status: ");
  printStatus(status);
}

static bool readAndPrintCurrentTime(const char* label, TCP3231::DateTime& dt) {
  if (!rtc.readTime(dt)) {
    Serial.print(label);
    Serial.print(" readTime failed: ");
    Serial.println(rtc.errorString());
    return false;
  }

  Serial.print(label);
  Serial.print(' ');
  printDateTime(dt);
  Serial.println();
  return true;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("beginning setup()");

  memset(&rtcBus, 0, sizeof(rtcBus));
  rtcBus.bWire = 0;
  rtcBus.iSDA = kRtcSdaPin;
  rtcBus.iSCL = kRtcSclPin;
  I2CInit(&rtcBus, kI2CFrequencyHz);

  Serial.println();
  Serial.println("TCP3231 SetAndReadTime starting");

  if (!rtc.begin()) {
    Serial.print("rtc.begin() failed: ");
    Serial.println(rtc.errorString());
    while (true) {
      delay(1000);
    }
  }

  printCurrentStatus();

  TCP3231::DateTime before{};
  (void)readAndPrintCurrentTime("Before set:", before);

  const TCP3231::DateTime target = {
    kSetYear,
    kSetMonth,
    kSetDay,
    kSetHour,
    kSetMinute,
    kSetSecond,
    kSetDayOfWeek
  };

  Serial.print("Target time: ");
  printDateTime(target);
  Serial.println();

  if (kSetTimeOnBoot) {
    if (!rtc.setTime(target)) {
      Serial.print("setTime failed: ");
      Serial.println(rtc.errorString());
      while (true) {
        delay(1000);
      }
    }
    Serial.println("setTime succeeded");
  } else {
    Serial.println("kSetTimeOnBoot is false; skipping setTime");
  }

  TCP3231::DateTime after{};
  if (!readAndPrintCurrentTime("After set: ", after)) {
    while (true) {
      delay(1000);
    }
  }

  if (kSetTimeOnBoot) {
    if (sameDateTime(target, after)) {
      Serial.println("ROUND TRIP OK");
    } else {
      Serial.println("ROUND TRIP FAILED");
      Serial.print("Expected: ");
      printDateTime(target);
      Serial.println();
      Serial.print("Actual:   ");
      printDateTime(after);
      Serial.println();
    }
  }

  printCurrentStatus();

  if (kSetTimeOnBoot && kClearOscillatorStopFlagAfterSet) {
    if (!rtc.clearOscillatorStopFlag()) {
      Serial.print("clearOscillatorStopFlag failed: ");
      Serial.println(rtc.errorString());
    } else {
      Serial.println("OSF cleared");
      printCurrentStatus();
    }
  }
}

void loop() {
  static uint32_t lastPrintMs = 0;
  const uint32_t nowMs = millis();
  if ((nowMs - lastPrintMs) < 1000) {
    return;
  }
  lastPrintMs = nowMs;

  TCP3231::DateTime now{};
  if (!rtc.readTime(now)) {
    Serial.print("loop readTime failed: ");
    Serial.println(rtc.errorString());
    return;
  }

  printDateTime(now);
  Serial.println();
}