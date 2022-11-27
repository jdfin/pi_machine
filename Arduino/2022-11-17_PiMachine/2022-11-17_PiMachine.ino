// PiMachine

#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <pidec.h>
#include <rgb.h>
#include <printer.h>
#include <chrono.h>

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
// 250 seems to be about the fastest, slower for debugging.
static const Interval print_interval(500);

// 19200 for the Mini, 9600 for the Nano
static const int printer_baud = 19200;

static Printer printer(Serial1);

static const int red_pin = 5;
static const int green_pin = 6;
static const int blue_pin = 9;
static Rgb led(red_pin, green_pin, blue_pin);


// The start time is advanced when waiting for power or paper.
static Time start_time;

// In the program, first digit after the decimal (the 1) is digit zero
// (that is dictated by DigitsOfPi).
//
// In the printout, the initial '3' is digit zero, so the printout uses
// digit_num + 1.
//
// digit_num: -4 and -3 are spaces, -2 is the '3', -1 is the '.', then
//            0.. are the digits after the decimal point
static const int32_t digit_num_start = -4;
static int32_t digit_num = digit_num_start;


// How long it took to calculate the most recent digit.
// loop() sets this and print_digit() uses it.
static Interval digit_interval;


// return true if the printer claims to have paper, false otherwise
static bool check_paper()
{
#if CHECK_PAPER_FAKE
  return (digitalRead(paper_fake_pin) == paper_fake_yes);
#else
  return printer.paper();
#endif
}


// If paper is detected, return true immediately.
// If paper is not detected, wait for it to be detected continuously
// for 10 seconds, then return false.
// Logic is similar to power_wait, but in here we also allow for power
// disappearing (i.e. run out of paper, unplug it to move it or something).
static bool paper_wait()
{

#if CHECK_PAPER || CHECK_PAPER_FAKE

  if (check_paper())
    return true;

  Serial.println("paper out");

  // Time we started waiting for more paper. This is used to adjust the global
  // start_time so it doesn't include paper-out time.
  const Time wait_start;

  // Need to see paper for 10 seconds before continuing.
  const Interval wait_timeout(10000);

  // Last time paper was detected as "out".
  Time paper_out;

  while ((Time::now() - paper_out) < wait_timeout) {

    if (!power_wait()) {
      // power went away, then came back
      paper_out = Time::now();
    }

    if (check_paper()) {
      // solid red means we see paper, waiting for 10 seconds
      led.set(Rgb::Red);
    } else {
      // winking red means we don't see paper
      led.pattern(Rgb::Red, 10, Rgb::Off, 990);
      paper_out = Time::now();
    }

    led.loop(); // handle blinking

    yield(); // could be in this while loop for a long time

  } // while

  // Adjust start_time as if we didn't have to wait.
  start_time += (Time::now() - wait_start);

  // Back up 30 digits after paper out. A nice clean end-of-paper only needs
  // a few digits of backing up (4 or 5 might do), but sometimes the end of
  // the roll is folded back on itself and a couple of inches at the end
  // don't print because it's trying to print on the back of the paper.
  // We're printing about 8.5 lines per inch.
  digit_num -= 30;
  if (digit_num < digit_num_start)
    digit_num = digit_num_start;

  Serial.println("paper okay");

  return false;

#else // CHECK_PAPER(_FAKE)

  return true;

#endif // CHECK_PAPER(_FAKE)

} // paper_wait


// return true if 5V supply is present, false otherwise
static bool check_power()
{

#if CHECK_POWER

  // VUSB is 5.0V when plugged in, and is about 3.3V when not plugged in.
  // The pin is half that. Set threshold for halfway between 5.0 and 3.3.
  // Halfway between 3.3 and 5.0 is 4.15V.
  // Pin is half that, or 2.075V.
  // ADC reading is 1024 at 3.3V, so ADC threshold is 644.
  const int v_thresh = 644;
  int v = analogRead(2);

  return (v >= v_thresh);

#else // CHECK_POWER

  return true;

#endif // CHECK_POWER

} // check_power


static bool power_wait()
{

  if (check_power())
    return true;

  Serial.println("power out");

  // Wait to be plugged in for at least 1 sec, then return false.
  // The delay is to let the printer boot up.
  // Returning false prevents the just-calculated digit from being printed.

  const Time wait_start;

  const Interval wait_timeout(1000); // 1 second

  // last time power was seen low
  Time power_low;

  while ((Time::now() - power_low) < wait_timeout) {

    if (check_power()) {
      // solid blue means we see 5V power, waiting 1 second
      led.set(Rgb::Blue);
    } else {
      // winking blue means we don't see 5V power
      led.pattern(Rgb::Blue, 10, Rgb::Off, 990);
      power_low = Time::now();
    }

    led.loop(); // handle blinking

    yield(); // could be in this while loop for hours

  } // while

  // Adjust start_time as if we didn't have to wait.
  start_time += (Time::now() - wait_start);

  // Back up three digits after power outage; really only the previous one
  // might be chopped off, but maybe there's some other case where more than
  // one might be lost, and losing a digit would be most terrible.
  digit_num -= 3;
  if (digit_num < digit_num_start)
    digit_num = digit_num_start;

  Serial.println("power okay");

  return false;

} // power_wait


// print a digit (paper and power already checked)
// 
static void print_digit(char digit)
{
  // Limit print rate so we don't queue up lines in the receive buffer,
  // which breaks paper-out handling. This is different from worrying
  // about overrunning the receive buffer; we basically want to know a
  // digit is on the paper before we try to print another one.
  static Time last_print_time;
  while ((Time::now() - last_print_time) < print_interval)
    delay(1);
  last_print_time = Time::now();

  // timestamp that will print with digit
  uint32_t h, m, s, ms;
  (last_print_time - start_time).hmsm(h, m, s, ms);

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
    sprintf(buf, "%-12ld| %c |%6lu:%02lu:%02lu", pr_digit_num, digit, h, m, s);
  else
    sprintf(buf, "            | %c |            ", digit);
  Serial.print(buf);

  sprintf(buf, "%9lld", digit_interval.ms());
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

  // Timestamp. This will run up against the digit in 11 years, then
  // do something uglier in 114 years.
  printer.rotate(false);
  printer.mode(Printer::Modes::FontLarge);
  printer.print(' ');
  printer.print(0xb2); // gray box
  if ('0' <= digit && digit <= '9') {
    sprintf(buf, "%6lu:%02lu:%02lu", h, m, s); // 12 chars right-justified
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

  led.set(Rgb::White);

  Serial.begin(115200);

#if WAIT_CONSOLE
  while (!Serial)
    ;
  delay(250);
#endif

#if CHECK_PAPER_FAKE

  // switch from gpio to ground
  // open is paper-yes, closed is paper-no
  pinMode(paper_fake_pin, INPUT_PULLUP);

#endif

  // printer.begin() sets the baud rate and sends a reset command,
  // but does not wait for any responses from the printer
  printer.begin(printer_baud);
  delay(250);

  start_time = Time::now();

  digit_num = digit_num_start;

} // setup


// Algorithm will only works starting at (approximately) digit 50 after
// the decimal (I don't know why) so before that we cheat and look it up
// This also puts a couple of blank lines at the start, and prints the
// decimal point.
static const char *pi50 =
    "  3.1415926535897932384626433832795028841971693993751058209";


void loop()
{
  led.set(Rgb::Green);

  Time digit_start_time;

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

  // how long it took to calculate this digit (only used for serial prints)
  Interval digit_interval(digit_start_time - Time::now());

  // If power_wait() returns false, power was detected as gone, then we
  // waited for it to come back. The last line printed might be faint or
  // chopped off, so digit_num has been backed up and we need to go back
  // and recalculate a previous digit.

  if (!power_wait())
    return;

  // Similar to power_wait(); start over on a previous digit. This backs up
  // farther than power_wait since the end of the paper can cause loss of more
  // digits.

  if (!paper_wait())
    return;

  print_digit(digit_char);
  digit_num++;

} // loop
