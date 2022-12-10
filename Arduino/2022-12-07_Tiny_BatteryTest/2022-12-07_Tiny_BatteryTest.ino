// TinyPiMachine Battery Test

// Print battery voltage once per second.

// batt / 5 = adc / 1024
// batt = (adc / 1024) * 5
//      = adc * 5 / 1024
// batt_mv = adc * 5000 / 1024
//         = adc * 625 / 128

#include <Arduino.h>
#include <tiny_pi_machine.h>


void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;
}


void loop()
{
  static uint32_t next_ms = millis();

  if (millis() >= next_ms) {
    uint16_t adc = analogRead(adc_batt);
    uint32_t mv = (adc * 625UL + 64) / 128; // 64 is for rounding

    Serial.print(adc);
    Serial.print(" ");
    Serial.print(mv);
    Serial.println("mV");

    next_ms += 1000;
  }
}
