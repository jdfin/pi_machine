#include <Arduino.h>
#include <millis64.h>

// Return 64-bit milliseconds

// 32-bit milliseconds rolls over at about 49.7 days.
// 64-bit milliseconds rolls over at about 584M years.

// This must be called at least once per 32-bit (49.7 day)
// interval in order to detect rollover.

uint64_t millis64()
{
  static uint32_t last_ms = millis();
  static uint32_t high_ms = 0;

  uint32_t low_ms = millis();
  if (low_ms < last_ms)
    high_ms++;

  last_ms = low_ms;

  return (uint64_t(high_ms) << 32) + low_ms;
}
