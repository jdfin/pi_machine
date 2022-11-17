#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <pidec.h>

// if 1, wait for serial (usb) console before starting
#define WAIT_CONSOLE 0

// if 1, print to real printer in addition to console
#define PRINT_DIGITS 1

#if PRINT_DIGITS

#include <printer.h>

const int printer_baud = 19200;

Printer printer(Serial1);

#endif // PRINT_DIGITS


void ms_to_hms(uint32_t ms, int& h, int& m, int& s)
{
  s = ms / 1000;

  m = s / 60;
  s = s - (m * 60);

  h = m / 60;
  m = m - (h * 60);
}


uint32_t start_ms;

// In the program, first digit after the decimal (the 1) is digit zero
// (that is dictated by DigitsOfPi).
// In the printout, the initial '3' is digit zero, so the printout uses
// digit_num + 1.
int32_t digit_num = 0;


#if PRINT_DIGITS

// If paper is detected, return true immediately.
// If paper is not detected, wait for it to be detected continuously
// for 10 seconds, then return false.
bool paper_ok()
{
  if (printer.paper())
    return true;

  // Time we started waiting for more paper. This is used to adjust the global
  // start_ms so it doesn't include paper out time.
  uint32_t wait_start_ms = millis();

  uint32_t wait_ms = 10000; // 10 seconds

  // last time paper was detected as "out"
  uint32_t paper_out_ms = millis();

  while ((millis() - paper_out_ms) < wait_ms) {
    if (!printer.paper())
      paper_out_ms = millis();
  }

  // Adjust start_ms as if we didn't have to wait for paper
  start_ms += (millis() - wait_start_ms);

  return false;
}

#endif // PRINT_DIGITS


// print a digit (paper already checked)
void print_digit(char digit)
{
  int h, m, s;
  ms_to_hms(millis() - start_ms, h, m, s);

  // large font is 12 pixels wide x 24 pixels high

  // rotated and doubled digit is 48 pixels wide
  // digit should be centered on 384/2 = 192, so should print at
  // 192 - (48 / 2) = 168

  // line will be:
  // digit number: 12 chars @ 12 pixels = 144 pixels
  // border:        2 chars @ 12 pixels =  24 pixels
  // digit:         1 char  @ 48 pixels =  48 pixels
  // border:        2 chars @ 12 pixels =  24 pixels
  // timestamp:    12 chars @ 12 pixels = 144 pixels
  // total:                               384 pixels

  char buf[50];

  if ('0' <= digit && digit <= '9')
    sprintf(buf, "%-12ld| %c |%6d:%02d:%02d", digit_num + 1, digit, h, m, s);
  else
    sprintf(buf, "            | %c |", digit);

  Serial.println(buf);

#if PRINT_DIGITS

  // digit number
  if ('0' <= digit && digit <= '9')
    sprintf(buf, "%-12ld", digit_num + 1); // 12 chars left-justified
  else
    strcpy(buf, "            "); // 12 chars

  printer.rotate(false);
  printer.mode(Printer::Modes::FontLarge);
  printer.print(buf);
  printer.print(0xb2); // gray box
  printer.print(' ');

  // digit
  printer.rotate(true);
  printer.mode(Printer::Modes::FontLarge | Printer::Modes::BoldOn |
               Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print(digit);

  // timestamp
  printer.rotate(false);
  printer.mode(Printer::Modes::FontLarge);
  printer.print(' ');
  printer.print(0xb2); // gray box
  if ('0' <= digit && digit <= '9') {
    sprintf(buf, "%6d:%02d:%02d", h, m, s); // 12 chars right-justified
    printer.print(buf);
  }

  // print buffered data and advance paper
  printer.flush();

  printer.mode(); // defaults

#endif // PRINT_DIGITS

} // print_digit


void setup()
{
  // Algorithm will only calculate from digit 50
  const char *FirstDigits = "1415926535897932384626433832795028841971693993751058209";

  Serial.begin(115200);

#if WAIT_CONSOLE
  while (!Serial)
    ;
  delay(250);
#endif

#if PRINT_DIGITS

  printer.begin(printer_baud);
  delay(250);

  // lines as close together as possible
  printer.line_space(0);

#endif // PRINT_DIGITS

  start_ms = millis();

  digit_num = -1; // makes the '3' print as digit zero
  print_digit('3');
  print_digit('.');
  for (digit_num = 0; digit_num < 50; digit_num++) {
    if (!paper_ok()) {
      digit_num -= 3; // repeat two digits
      if (digit_num < -1)
        digit_num = -1;
      continue;
    }
    print_digit(FirstDigits[digit_num]);
    // Delay needed here to prevent overruns at start: 20 msec is not enough,
    // 25 msec seems to be enough. When we start calculating digits, the first
    // ones take long enough. With 50 msec, the paper-out detection doesn't
    // work (as if there is data buffered when paper out is noticed.) 200 msec
    // seems to be enough for paper-out to work.
    delay(250);
  }

} // setup


void loop()
{
  double x = DigitsOfPi(digit_num);
  double pow = 1.e9;
  double y = x * pow;
  int digit = int32_t(y) / 100000000;
  if (!paper_ok()) {
    digit_num -= 2; // repeat two digits
    return;
  }
  print_digit('0' + digit);
  digit_num++;
}
