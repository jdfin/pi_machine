// TinyPiMachine Power Test

// Turn off power after five seconds.

#include <Arduino.h>
#include <tiny_pi_machine.h>


void setup()
{
  digitalWrite(LED_BUILTIN, 1);
  pinMode(LED_BUILTIN, OUTPUT);
  // LED turns off when power is cut

  digitalWrite(gpio_off, 0);
  pinMode(gpio_off, OUTPUT);

  delay(5000);

  digitalWrite(gpio_off, 1);
}


void loop()
{
}
