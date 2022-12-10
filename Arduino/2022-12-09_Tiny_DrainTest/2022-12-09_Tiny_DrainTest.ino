// TinyPiMachine

// Print the battery voltage once every 5 seconds. The system will power
// off (ungracefully) when the battery gets low enough. This is useful
// for checking the printer.heat() settings.

#include <Arduino.h>
#include <stdio.h>
#include <printer.h>
#include <tiny_pi_machine.h>


static Printer printer(Serial1);


void setup()
{

  // printer.begin() sets the baud rate and sends a reset command,
  // but does not wait for any responses from the printer
  printer.begin(printer_baud);
  delay(500);

  printer.heat(5, 200, 1);

} // setup


void loop()
{
  static uint32_t next_ms = millis();

  if (millis() < next_ms)
    return;

  next_ms += 5000UL; // 5 seconds

  int adc = analogRead(adc_batt);
  int mv = (adc * 625L + 64) / 128; // 64 is for rounding

  static char pbuf[40];
  sprintf(pbuf, "Battery: %d mV", mv); // 23 chars max
  printer.print(pbuf);
  printer.flush(0);
}
