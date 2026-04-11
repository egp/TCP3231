#include "DS3231RTC.h"

namespace {
static const uint8_t kRegSeconds = 0x00;
static const uint8_t kRegMinutes = 0x01;
static const uint8_t kRegHours   = 0x02;
static const uint8_t kRegDay     = 0x03;
static const uint8_t kRegDate    = 0x04;
static const uint8_t kRegMonth   = 0x05;
static const uint8_t kRegYear    = 0x06;
static const uint8_t kRegStatus  = 0x0F;

static const uint8_t kHour12_24Mask = 0x40;
static const uint8_t kHourPmMask    = 0x20;
static const uint8_t kMonthCenturyMask = 0x80;

static const uint8_t kStatusOsfMask    = 0x80;
static const uint8_t kStatusEn32kHzMask = 0x08;
static const uint8_t kStatusBsyMask    = 0x04;
static const uint8_t kStatusA2fMask    = 0x02;
static const uint8_t kStatusA1fMask    = 0x01;

static const uint8_t kTimeRegisterCount = 7;
static const uint8_t kMaxWritePayload = 16;
}  // namespace

DS3231RTC::DS3231RTC(BBI2C& bus, uint8_t address)
    : bus_(bus), address_(address), lastError_("ok") {}

bool DS3231RTC::begin() {
  uint8_t status = 0;
  if (!readRegister(kRegStatus, status)) {
    return false;
  }
  setError("ok");
  return true;
}

bool DS3231RTC::readTime(DateTime& dt) {
  uint8_t raw[kTimeRegisterCount] = {0};

  if (!readRegisters(kRegSeconds, raw, kTimeRegisterCount)) {
    return false;
  }

  const uint8_t second = bcdToDec(static_cast<uint8_t>(raw[0] & 0x7F));
  const uint8_t minute = bcdToDec(static_cast<uint8_t>(raw[1] & 0x7F));
  const uint8_t hourReg = raw[2];
  const uint8_t dayOfWeek = static_cast<uint8_t>(raw[3] & 0x07);
  const uint8_t day = bcdToDec(static_cast<uint8_t>(raw[4] & 0x3F));
  const bool century = (raw[5] & kMonthCenturyMask) != 0;
  const uint8_t month = bcdToDec(static_cast<uint8_t>(raw[5] & 0x1F));
  const uint16_t year = static_cast<uint16_t>(2000 + (century ? 100 : 0) + bcdToDec(raw[6]));

  uint8_t hour = 0;
  if ((hourReg & kHour12_24Mask) != 0) {
    const uint8_t hour12 = bcdToDec(static_cast<uint8_t>(hourReg & 0x1F));
    const bool pm = (hourReg & kHourPmMask) != 0;
    if (hour12 < 1 || hour12 > 12) {
      setError("invalid RTC hour register");
      return false;
    }
    hour = pm ? static_cast<uint8_t>(hour12 % 12 + 12) : static_cast<uint8_t>(hour12 % 12);
  } else {
    hour = bcdToDec(static_cast<uint8_t>(hourReg & 0x3F));
    if (hour > 23) {
      setError("invalid RTC hour register");
      return false;
    }
  }

  dt.year = year;
  dt.month = month;
  dt.day = day;
  dt.hour = hour;
  dt.minute = minute;
  dt.second = second;
  dt.dayOfWeek = dayOfWeek;

  if (!isValidDateTime(dt)) {
    setError("invalid RTC date/time");
    return false;
  }

  setError("ok");
  return true;
}

bool DS3231RTC::setTime(const DateTime& dt) {
  if (!isValidDateTime(dt)) {
    setError("invalid date/time");
    return false;
  }

  uint8_t raw[kTimeRegisterCount];
  raw[0] = decToBcd(dt.second);
  raw[1] = decToBcd(dt.minute);
  raw[2] = decToBcd(dt.hour); // always write 24-hour mode
  raw[3] = decToBcd(dt.dayOfWeek);
  raw[4] = decToBcd(dt.day);

  uint8_t month = decToBcd(dt.month);
  if (dt.year >= 2100) {
    month = static_cast<uint8_t>(month | kMonthCenturyMask);
  }
  raw[5] = month;
  raw[6] = decToBcd(static_cast<uint8_t>(dt.year % 100));

  if (!writeRegisters(kRegSeconds, raw, kTimeRegisterCount)) {
    return false;
  }

  setError("ok");
  return true;
}

bool DS3231RTC::readStatus(Status& status) {
  uint8_t raw = 0;
  if (!readRegister(kRegStatus, raw)) {
    return false;
  }

  status.oscillatorStopped = (raw & kStatusOsfMask) != 0;
  status.busy = (raw & kStatusBsyMask) != 0;
  status.alarm1Flag = (raw & kStatusA1fMask) != 0;
  status.alarm2Flag = (raw & kStatusA2fMask) != 0;
  status.en32kHz = (raw & kStatusEn32kHzMask) != 0;

  setError("ok");
  return true;
}

bool DS3231RTC::clearOscillatorStopFlag() {
  uint8_t raw = 0;
  if (!readRegister(kRegStatus, raw)) {
    return false;
  }

  raw = static_cast<uint8_t>(raw & static_cast<uint8_t>(~kStatusOsfMask));

  if (!writeRegister(kRegStatus, raw)) {
    return false;
  }

  setError("ok");
  return true;
}

const char* DS3231RTC::errorString() const {
  return lastError_;
}

bool DS3231RTC::readRegister(uint8_t reg, uint8_t& value) {
  return readRegisters(reg, &value, 1);
}

/***********************/

bool DS3231RTC::readRegisters(uint8_t startReg, uint8_t* data, uint8_t len) {
  if (data == nullptr || len == 0) {
    setError("invalid read buffer");
    return false;
  }

  for (uint8_t i = 0; i < len; ++i) {
    uint8_t reg = static_cast<uint8_t>(startReg + i);

    const int pointerWritten = I2CWrite(&bus_, address_, &reg, 1);
    if (pointerWritten != 1) {
      setError("I2C register select failed");
      return false;
    }

    const int received = I2CRead(&bus_, address_, &data[i], 1);
    if (received != 1) {
      setError("I2C read failed");
      return false;
    }
  }

  return true;
}

/***********************/

bool DS3231RTC::writeRegister(uint8_t reg, uint8_t value) {
  return writeRegisters(reg, &value, 1);
}

bool DS3231RTC::writeRegisters(uint8_t startReg, const uint8_t* data, uint8_t len) {
  if (data == nullptr || len == 0) {
    setError("invalid write buffer");
    return false;
  }
  if (len > kMaxWritePayload) {
    setError("write too large");
    return false;
  }

  uint8_t buffer[kMaxWritePayload + 1];
  buffer[0] = startReg;
  for (uint8_t i = 0; i < len; ++i) {
    buffer[static_cast<uint8_t>(i + 1)] = data[i];
  }

  const int written = I2CWrite(&bus_, address_, buffer, static_cast<int>(len + 1));
  if (written != static_cast<int>(len + 1)) {
    setError("I2C write failed");
    return false;
  }

  return true;
}

uint8_t DS3231RTC::decToBcd(uint8_t value) {
  return static_cast<uint8_t>(((value / 10u) << 4) | (value % 10u));
}

uint8_t DS3231RTC::bcdToDec(uint8_t value) {
  return static_cast<uint8_t>(((value >> 4) * 10u) + (value & 0x0Fu));
}

bool DS3231RTC::isLeapYear(uint16_t year) {
  if ((year % 400u) == 0u) {
    return true;
  }
  if ((year % 100u) == 0u) {
    return false;
  }
  return (year % 4u) == 0u;
}

uint8_t DS3231RTC::daysInMonth(uint16_t year, uint8_t month) {
  switch (month) {
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12:
      return 31;
    case 4:
    case 6:
    case 9:
    case 11:
      return 30;
    case 2:
      return isLeapYear(year) ? 29 : 28;
    default:
      return 0;
  }
}

bool DS3231RTC::isValidDateTime(const DateTime& dt) {
  if (dt.year < 2000 || dt.year > 2199) {
    return false;
  }
  if (dt.month < 1 || dt.month > 12) {
    return false;
  }
  if (dt.dayOfWeek < 1 || dt.dayOfWeek > 7) {
    return false;
  }
  if (dt.hour > 23 || dt.minute > 59 || dt.second > 59) {
    return false;
  }

  const uint8_t maxDay = daysInMonth(dt.year, dt.month);
  if (maxDay == 0) {
    return false;
  }
  if (dt.day < 1 || dt.day > maxDay) {
    return false;
  }

  return true;
}

void DS3231RTC::setError(const char* message) {
  lastError_ = message;
}