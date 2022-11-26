#include <Arduino.h>
#include <cassert>
#include <millis64.h>

extern "C" void SysTick_DefaultHandler(void);


void get_both(uint32_t& ms, uint64_t& ms64)
{
  uint32_t ms1;
  do {
    __asm__ volatile ("nop");
    ms = millis();
    __asm__ volatile ("nop");
    ms64 = millis64();
    __asm__ volatile ("nop");
    ms1 = millis();
    __asm__ volatile ("nop");
  } while (ms1 != ms);
}


void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  uint32_t now_ms;
  uint64_t now_ms64;

  get_both(now_ms, now_ms64);

  Serial.print(now_ms, HEX);
  Serial.print(' ');
  Serial.println(now_ms64, HEX);

  for (int i = 0; i < 1000; i++) {
    get_both(now_ms, now_ms64);
    if ((now_ms64 % 0x100000000ull) != now_ms) {
      Serial.print("ERROR: ");
      Serial.print(now_ms, HEX);
      Serial.print(' ');
      Serial.println(now_ms64, HEX);
      while (1)
        ;
    }
    delay(1);
  }

  get_both(now_ms, now_ms64);

  Serial.print(now_ms, HEX);
  Serial.print(' ');
  Serial.println(now_ms64, HEX);

  // get millis() to where it's about to roll over
  // this loop takes ~18.5 minutes on cortex m4 at 120 MHz
  while (millis() < 4294966796) // 2^32 - 500
    SysTick_DefaultHandler();

  get_both(now_ms, now_ms64);

  Serial.print(now_ms, HEX);
  Serial.print(' ');
  Serial.println(now_ms64, HEX);

  for (int i = 0; i < 1000; i++) {
    get_both(now_ms, now_ms64);
    if ((now_ms64 % 0x100000000ull) != now_ms) {
      Serial.print("ERROR: ");
      Serial.print(now_ms, HEX);
      Serial.print(' ');
      Serial.println(now_ms64, HEX);
      while (1)
        ;
    }
    delay(1);
  }

  get_both(now_ms, now_ms64);

  Serial.print(now_ms, HEX);
  Serial.print(' ');
  Serial.println(now_ms64, HEX);

} // setup


void loop()
{
}
