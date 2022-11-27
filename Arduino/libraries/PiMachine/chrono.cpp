
#include <Arduino.h>
#include "chrono.h"

uint32_t Time::_last_ms32 = 0;
uint32_t Time::_high_ms = 0;


// This must be called at least once per 32-bit (49.7 day)
// interval in order to detect rollover.
int64_t Time::now64()
{
  uint32_t low_ms32 = millis();
  if (low_ms32 < _last_ms32)
    _high_ms++;

  _last_ms32 = low_ms32;

  return (int64_t(_high_ms) << 32) + low_ms32;
}


Time Time::now()
{
  return now64(); // implicit constructor call
}


// return hours, minutes, seconds, milliseconds
void Interval::hmsm(uint32_t& h, uint32_t& m, uint32_t& s, uint32_t& ms) const
{
  uint64_t ms64 = (_ms64 >= 0 ? _ms64 : -_ms64);
  uint64_t s64 = ms64 / 1000;
  ms64 -= (s64 * 1000);
  uint64_t m64 = s64 / 60;
  s64 -= (m64 * 60);
  uint64_t h64 = m64 / 60;
  m64 -= (h64 * 60);
  h = h64;
  m = m64;
  s = s64;
  ms = ms64;
}


// friend of Time
Interval operator-(const Time& t1, const Time& t2)
{
  return Interval(t1._ms64 - t2._ms64);
}


// friend of Interval
bool operator<(const Interval& i1, const Interval& i2)
{
  return i1._ms64 < i2._ms64;
}


// friend of both Time and Interval
Time operator+(const Time& t1, const Interval& i2)
{
  // Interval can be negative, but Time cannot be negative
  if (i2._ms64 >= 0) {
    return Time(t1._ms64 + i2._ms64);
  } else {
    // i2 < 0
    if (t1._ms64 >= -i2._ms64)
      return Time(t1._ms64 + i2._ms64);
    else
      return Time(0);
  }
}


Time& Time::operator+=(const Interval& i)
{
  // Interval can be negative, but Time cannot be negative
  if (i._ms64 >= 0) {
    this->_ms64 += i._ms64;
  } else {
    // i < 0
    if (this->_ms64 >= -i._ms64)
      this->_ms64 += i._ms64;
    else
      this->_ms64 = 0;
  }
  return *this;
}


// for debug (what's the right way to do this?)
int64_t get_time_ms64(Time& t)
{
  return t._ms64;
}
