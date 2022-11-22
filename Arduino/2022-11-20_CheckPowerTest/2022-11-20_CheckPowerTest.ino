#include <Arduino.h>


// VUSB is 5.0V when plugged in, and is about 3.3V when not plugged in
// The pin is half that. Set threshold for halfway between 5.0 and 3.3.
// Halfway between 3.3 and 5.0 is 4.15V.
// Pin is half that, or 2.075V.
// ADC reading is 1024 at 3.3V, so ADC threshold is 644.


void setup()
{

  Serial.begin(115200);
  while (!Serial)
    ;

}


void loop()
{
  static uint32_t next_ms = millis();

  if (millis() < next_ms)
    return;

  next_ms += 1000;

  int adc = analogRead(2);

  // mV = 2 * 3300 * adc / 1024
  //   reduce:
  // mv = 825 * adc / 128

  int mv = 825 * adc / 128;

  Serial.print("adc="); Serial.print(adc);
  Serial.print("    mv="); Serial.print(mv);
  Serial.println();
}
