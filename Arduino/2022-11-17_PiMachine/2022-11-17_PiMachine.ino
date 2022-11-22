#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <pidec.h>
#include <rgb.h>
#include <printer.h>

// if 1, wait for serial (usb) console before starting
#define WAIT_CONSOLE 0

// if 1, print to real printer in addition to console
#define PRINT_DIGITS 1

// if 1, check for paper out (can do this without printing digits)
#define CHECK_PAPER 1

// if 1, switch on GPIO simulates paper out
#define CHECK_PAPER_FAKE 0

static const int paper_fake_pin = 10;
static const int paper_fake_yes = 1; // gpio when "paper is present"

// if 1, check power and pause when not present
// (requires divider connected to a/d pin)
#define CHECK_POWER 1

// Minimum number of milliseconds per digit printed.
// 250 seems about right, or slower sometimes for debugging.
const uint32_t print_interval_ms = 500; //250;

// 19200 for the Mini, 9600 for the Nano
const int printer_baud = 19200;

Printer printer(Serial1);

static const int red_pin = 5;
static const int green_pin = 6;
static const int blue_pin = 9;
Rgb led(red_pin, green_pin, blue_pin);


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


// How long it took to calculate the most recent digit
uint32_t digit_ms = 0;


// If paper is detected, return true immediately.
// If paper is not detected, wait for it to be detected continuously
// for 10 seconds, then return false.
bool paper_ok()
{

#if CHECK_PAPER || CHECK_PAPER_FAKE

#if CHECK_PAPER_FAKE
  if (digitalRead(paper_fake_pin) == paper_fake_yes)
    return true;
#else
  if (printer.paper())
    return true;
#endif

  led.set(4); // red

  // Time we started waiting for more paper. This is used to adjust the global
  // start_ms so it doesn't include paper out time.
  uint32_t wait_start_ms = millis();

  const uint32_t wait_ms = 10000; // 10 seconds

  // last time paper was detected as "out"
  uint32_t paper_out_ms = millis();

  while ((millis() - paper_out_ms) < wait_ms) {
#if CHECK_PAPER_FAKE
    if (digitalRead(paper_fake_pin) != paper_fake_yes) {
      paper_out_ms = millis();
      led.set(4); // red
    } else {
      led.set(1); // blue
    }
#else
    if (!printer.paper()) {
      paper_out_ms = millis();
      led.set(4); // red
    } else {
      led.set(1); // blue
    }
#endif
  }

  // Adjust start_ms as if we didn't have to wait
  start_ms += (millis() - wait_start_ms);

  led.set(2); // green

  return false;

#else // CHECK_PAPER(_FAKE)

  return true;

#endif // CHECK_PAPER(_FAKE)

} // check_paper


bool power_ok()
{
  // VUSB is 5.0V when plugged in, and is about 3.3V when not plugged in
  // The pin is half that. Set threshold for halfway between 5.0 and 3.3.
  // Halfway between 3.3 and 5.0 is 4.15V.
  // Pin is half that, or 2.075V.
  // ADC reading is 1024 at 3.3V, so ADC threshold is 644.
  const int v_thresh = 644;
  int v = analogRead(2);

#if CHECK_POWER

  if (v >= v_thresh)
    return true;

  led.set(4); // red

  // Wait to be plugged in for at least 1 sec, then return false.
  // The delay is to let the printer boot up.

  uint32_t wait_start_ms = millis();

  const uint32_t wait_ms = 1000; // 1 second

  // last time power was seen low
  uint32_t power_low_ms = millis();

  while ((millis() - power_low_ms) < wait_ms) {
    if (analogRead(2) < v_thresh)
      power_low_ms = millis();
  }

  // Adjust start_ms as if we didn't have to wait
  start_ms += (millis() - wait_start_ms);

  led.set(2); // green

  return false;

#else // CHECK_POWER

  return true;

#endif // CHECK_POWER

} // check_power


// print a digit (paper and power already checked)
// 
void print_digit(char digit)
{
  // Limit print rate so we don't queue up lines in the receive buffer,
  // which breaks paper-out handling. This is different from worrying
  // about overrunning the receive buffer; we basically want to know a
  // digit is on the paper before we try to print another one.
  static uint32_t next_print_ms = 0;
  while (millis() < next_print_ms)
    delay(1);
  next_print_ms = millis() + print_interval_ms;

  // timestamp that will print with digit
  int h, m, s;
  ms_to_hms(millis() - start_ms, h, m, s);

  // digit number to print
  int32_t pr_digit_num;
  if (digit_num == -2)
    pr_digit_num = 0; // initial '3'
  else
    pr_digit_num = digit_num + 1;

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

  // only print number and timestamp if digit is 0..9
  if ('0' <= digit && digit <= '9')
    sprintf(buf, "%-12ld| %c |%6d:%02d:%02d", pr_digit_num, digit, h, m, s);
  else
    sprintf(buf, "            | %c |            ", digit);
  Serial.print(buf);

  sprintf(buf, "%9d", digit_ms);
  Serial.print(buf);

  Serial.println();

#if PRINT_DIGITS

  // digit number
  if ('0' <= digit && digit <= '9')
    sprintf(buf, "%-12ld", pr_digit_num); // 12 chars left-justified
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

  printer.line_space(0);  // lines as close together as possible
  printer.flush();        // print buffered data and advance paper

  printer.mode(); // defaults

#endif // PRINT_DIGITS

} // print_digit


void setup()
{
  led.begin();

  led.set(1); // blue

  Serial.begin(115200);

#if WAIT_CONSOLE
  while (!Serial)
    ;
  delay(250);
#endif

  led.set(2); // green

#if CHECK_PAPER_FAKE
  // switch from gpio to ground
  // open is paper-yes, closed is paper-no
  pinMode(paper_fake_pin, INPUT_PULLUP);
#endif

  // printer.begin() sets the baud rate and sends a reset command,
  // but does not wait for any responses from the printer
  printer.begin(printer_baud);
  delay(250);

  start_ms = millis();

  digit_num = -4;

} // setup


// Algorithm will only works starting at (approximately) digit 50 after
// the decimal (I don't know why) so before that we cheat and look it up
// This also puts a couple of blank lines at the start, and prints the
// decimal point.
const char *pi50 =
    "  3.1415926535897932384626433832795028841971693993751058209";


void loop()
{
  uint32_t digit_start_ms = millis();

  char digit_char;

  if (digit_num < 50) {

    // digit_num == 0 should get the '1' after the decimal point (pi50[4])
    // digit_num starts at -4

    digit_char = pi50[digit_num + 4];

  } else {

    double x = DigitsOfPi(digit_num);
    double y = x * 1.0e9;
    int digit = int32_t(y) / 100000000;

    digit_char = '0' + digit;

  }

  digit_ms = millis() - digit_start_ms;

  if (paper_ok() && power_ok()) {
    print_digit(digit_char);
    digit_num++;
  } else {
    digit_num -= 3; // repeat two digits
    if (digit_num < -4)
      digit_num = -4;
  }
}
