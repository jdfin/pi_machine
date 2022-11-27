#pragma once

#include <Arduino.h>

class Interval;

// Inspired by std::chrono in that timestamps and time intervals have
// different types and sane use is enforced, but that's about it.
//
// Main objectives are:
// (1) keep track of time with millisecond precision and 64 bit range
// (2) distinguish timestamps and intervals
//
// Lower 32 bits of Time should always be the same as millis().
//
// 32-bit milliseconds rolls over at about 50 days.
// 32-bit (signed) millisecond interval only reaches to +/- 25 days.
// 64-bit milliseconds rolls over at about 584M years.


// Time is an absolute point in time. Valid operations:
// * Get "now"
// * Time = Time + Interval
// * Time = Time - Interval
// * Compare Times
// * Interval = Time - Time
//
// The value of a Time object is meaningless; just its relationship to another
// Time (which is an Interval) is what is meaningful.
class Time {

  public:

    Time() : _ms64(now64()) { }

    // return system time
    static Time now();

    // add Interval to a Time
    Time& operator+=(const Interval& i);

  private:

    // private because one should only construct time by getting "now"
    // and offsetting from that
    Time(int64_t ms64) : _ms64(ms64) { }

    // return system time in milliseconds
    static int64_t now64();

    int64_t _ms64;  // using signed makes it easier to think about times
                    // going back through zero (it stays continuous).

    // Used keep track of system time. If we ever see that millis() is less
    // than _last_ms32, we know to increment _high_ms. That means we must
    // look at the time at least once every few weeks.
    static uint32_t _last_ms32;
    static uint32_t _high_ms;

    friend Interval operator-(const Time& t1, const Time& t2);
    friend Time operator+(const Time& t1, const Interval& i2);

    friend int64_t get_time_ms64(Time& t); // for debug
};


class Interval {

  public:

    Interval(int64_t ms64) : _ms64(ms64) { }

    Interval() : _ms64(0) { }

    int64_t ms() const { return _ms64; }

    // hours, minutes, seconds, milliseconds of the absolute value
    void hmsm(uint32_t& h, uint32_t& m, uint32_t& s, uint32_t& ms) const;

  private:

    int64_t _ms64;

    friend Time;
    friend bool operator<(const Interval& i1, const Interval& i2);
    friend Time operator+(const Time& t1, const Interval& i2);
};
