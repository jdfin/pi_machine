// ChronoTest

#include <Arduino.h>
#include <chrono.h>


// wait for millis() to tick over
// (don't know if delay(1) would work for this)
static void wait_tick()
{
  uint32_t ms = millis();
  while (millis() == ms)
    ;
}


static void result(bool pass, const char* msg)
{
  if (pass)
    Serial.print("PASS: ");
  else
    Serial.print("FAIL: ");
  Serial.println(msg);
}


void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  delay(100);

  Serial.println("Chrono Test");

  uint32_t h, m, s, ms;

  {
    wait_tick();
    Time t;
    result(millis() == get_time_ms64(t), "Time and millis() in sync");
  }

  {
    wait_tick();
    Time t1;
    Time t2;
    Interval i1(t2 - t1);
    i1.hmsm(h, m, s, ms);
    result(h == 0 && m == 0 && s == 0 && ms == 0, "hmsm() zero");
  }

  {
    wait_tick();
    Time t1;
    wait_tick();
    Time t2;
    Interval i(t2 - t1);
    i.hmsm(h, m, s, ms);
    result(h == 0 && m == 0 && s == 0 && ms == 1, "hmsm() 1 msec");
  }

  {
    wait_tick();
    Time t1;
    for (int i = 0; i < 1000; i++)
      wait_tick();
    Time t2;
    Interval i(t2 - t1);
    i.hmsm(h, m, s, ms);
    result(h == 0 && m == 0 && s == 1 && ms == 0, "hmsm() 1 sec");
  }

  {
    Interval i0(0);
    Interval i1(1);
    Interval i2(2);
    result(!(i0 < i0), "< false");
    result(i0 < i1,    "< true");
    result(i0 < i2,    "< true");
    result(!(i1 < i0), "< false");
    result(!(i1 < i1), "< false");
    result(i1 < i2,    "< true");
    result(!(i2 < i0), "< false");
    result(!(i2 < i1), "< false");
    result(!(i2 < i2), "< false");
  }

  {
    wait_tick();
    Time t0;
    wait_tick();
    Time t1;
    Interval i0(t0 - t1);
    Interval i1(t1 - t0);
    i0.hmsm(h, m, s, ms);
    result(h == 0 && m == 0 && s == 0 && ms == 1, "hmsm() 1 msec");
    i1.hmsm(h, m, s, ms);
    result(h == 0 && m == 0 && s == 0 && ms == 1, "hmsm() 1 msec");
    result(i0 < i1,    "< true");
    result(!(i1 < i0), "< false");
  }

} // setup


void loop()
{
}
