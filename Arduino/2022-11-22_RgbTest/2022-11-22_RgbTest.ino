#include <Arduino.h>
#include <rgb.h>

static const int red_pin = 5;
static const int green_pin = 6;
static const int blue_pin = 9;
Rgb led(red_pin, green_pin, blue_pin);


void setup()
{
  led.begin();

  Serial.begin(115200);

#if 0
  while (!Serial)
    ;
#endif

  uint32_t next_ms = millis() + 1000;

  led.set(Rgb::Red);
  while (millis() < next_ms)
    led.loop();

  next_ms += 1000;

  led.set(Rgb::Green);
  while (millis() < next_ms)
    led.loop();

  next_ms += 1000;

  led.set(Rgb::Blue);
  while (millis() < next_ms)
    led.loop();

  // "paper out" - flashing red, 10 seconds

  next_ms += 10000;

  led.pattern(Rgb::Red, 10, Rgb::Off, 990);
  while (millis() < next_ms)
    led.loop();

  // "ok" - green, 2 seconds

  next_ms += 2000;

  led.set(Rgb::Green);
  while (millis() < next_ms)
    led.loop();

  // "power fail" - flashing blue, forever

  led.pattern(Rgb::Blue, 10, Rgb::Off, 990);

} // setup


void loop()
{
  led.loop();
}
