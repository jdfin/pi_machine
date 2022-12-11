// Printer Test

#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <printer.h>


// 19200 for the Mini, 9600 for the Nano
static const int printer_baud = 19200;

static Printer printer(Serial1);


void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ;

  // printer.begin() sets the baud rate and sends a reset command,
  // but does not wait for any responses from the printer
  printer.begin(printer_baud);
  delay(250);

  printer.print("Default: 0123456789\n");         delay(200);
  printer.print("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");  delay(200);
  printer.print("abcdefghijklmnopqrstuvwxyz\n");  delay(200);

  printer.mode(Printer::Modes::BoldOn);
  printer.print("BoldOn: 0123456789\n");          delay(200);
  printer.print("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");  delay(200);
  printer.print("abcdefghijklmnopqrstuvwxyz\n");  delay(200);

  printer.mode(Printer::Modes::Height2x);
  printer.print("Height2x: 0123456789\n");        delay(200);
  printer.print("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n");  delay(200);
  printer.print("abcdefghijklmnopqrstuvwxyz\n");  delay(200);

  printer.mode(Printer::Modes::Width2x);
  printer.print("Width2x: 0123456\n");            delay(200);
  printer.print("ABCDEFGHIJKLMNOP\n");            delay(200);
  printer.print("abcdefghijklmnop\n");            delay(200);

  printer.mode(Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print("Height2x|Width2x\n");            delay(200);
  printer.print("ABCDEFGHIJKLMNOP\n");            delay(200);
  printer.print("abcdefghijklmnop\n");            delay(200);

  // some firmwares don't do bold + rotated + 2x size

  printer.mode();
  printer.print("[Rotate on]\n");                 delay(200);
  printer.rotate(true);
  printer.print("Default\n");                     delay(200);
  printer.mode(Printer::Modes::BoldOn);
  printer.print("BoldOn\n");                      delay(200);
  printer.mode(Printer::Modes::Height2x);
  printer.print("Height2x\n");                    delay(200);
  printer.mode(Printer::Modes::Width2x);
  printer.print("Width2x\n");                     delay(200);
  printer.mode(Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print("H2x|W2x\n");                     delay(200);
  printer.mode(Printer::Modes::BoldOn | Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print("Bld|H|W\n");                     delay(200);

  printer.print("\n\n\n");

} // setup


void loop()
{
}
