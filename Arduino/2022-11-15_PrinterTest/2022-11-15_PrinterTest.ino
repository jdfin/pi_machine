
#include <printer.h>

const int baud_rate = 19200;

Printer printer(Serial1);


void print_digit(int num, char d, int h, int m, int s)
{
  char buf[24];

  // large font is 12 pixels wide x 24 pixels high

  // rotated and doubled digit is 48 pixels wide
  // digit should be centered on 384/2 = 192, so should print at
  // 192 - (48 / 2) = 168

  // line will be:
  // digit number: 13 chars @ 12 pixels = 156 pixels
  // border:        1 char  @ 12 pixels =  12 pixels
  // digit:         1 char  @ 48 pixels =  48 pixels
  // border:        1 char  @ 12 pixels =  12 pixels
  // timestamp:    13 chars @ 12 pixels = 156 pixels
  // total:                               384 pixels

  printer.rotate(false);
  printer.mode(Printer::Modes::FontLarge);
  if (num >= 0)
    sprintf(buf, "%-12d", num); // 12 chars left-justified
  else
    sprintf(buf, "            "); // 12 chars
  printer.print(buf);
  printer.print(0xb2); // gray box
  printer.print(' ');

  printer.rotate(true);
  // On Mini, one must request doubling in both directions, but the result
  // is the character is taller, not wider (so it still fits on one line)
  printer.mode(Printer::Modes::FontLarge | Printer::Modes::BoldOn |
               Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print(d);

  printer.rotate(false);
  printer.mode(Printer::Modes::FontLarge);
  printer.print(' ');
  printer.print(0xb2); // gray box
  if (num >= 0)
    sprintf(buf, "%6d:%02d:%02d", h, m, s); // 12 chars right-justified
  else
    sprintf(buf, "            "); // 12 chars
  printer.print(buf);

  // print buffered data and advance paper
  printer.flush();

  printer.mode(); // defaults
}


void print_some_pi()
{
  printer.line_space(0); // as close as possible

  const char* pi = "3.1415926535897932";
  int index = -2;

  while (*pi != '\0') {
    print_digit(index, *pi, 0, 0, index);
    pi++;
    index++;
  }

  printer.line_space(); // default
}


void setup()
{
  Serial.begin(115200);
#if 1
  while (!Serial)
    ;
#endif

  delay(250);

  printer.begin(baud_rate);

  printer.print("Printer Test\n");

  if (!printer.paper()) {
    Serial.println("no paper");
    return;
  }

  printer.print("Large Font....32 characters/line\n");

  printer.mode(Printer::Modes::BoldOn);
  printer.print("Bold\n");

  printer.mode(Printer::Modes::Height2x);
  printer.print("Height 2x\n");

  printer.mode(Printer::Modes::Width2x);
  printer.print("Width 2x\n");

  printer.mode(Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print("Both 2x\n");

  printer.mode();

  printer.rotate(true);
  printer.print("Rotate\n");

  printer.mode(Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print("Both 2x\n");

  printer.rotate(false);

  printer.mode(Printer::Modes::FontSmall);
  printer.print("Small Font..............42 characters/line\n");

  print_some_pi();

  printer.print("\n\n\n");
}


void loop()
{
}
