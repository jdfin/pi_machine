// TinyPiMachine

// While the button is pressed, print digits of pi. When the button has been
// released for 1 second, print some blank lines and power off.

#include <Arduino.h>
#include <string.h>
#include <stdio.h>
#include <printer.h>
#include <tiny_pi_machine.h>
#include "pi.h"

// if 1, wait for serial (usb) console before starting
#define WAIT_CONSOLE 0

// if 1, print to real printer in addition to console
#define USE_PRINTER 1

// Minimum number of milliseconds per digit printed.
// 250 seems to be about the fastest, slower for effect (or debugging).
static const uint32_t print_interval_ms = 500;

// Below this, print recharge message and stop.
// See comments at printer.heat() call in setup().
static const uint32_t battery_min_mv = 3800;

static Printer printer(Serial1);

static int32_t digit_num = 0;

// for snprintf
static const int pbuf_len = 40;
static char pbuf[pbuf_len];


// Check button each call. When it has been seen released for btn_timeout_ms,
// print some lines, delay, then power off (never returns in that case).
static void check_button()
{
  static const uint32_t btn_timeout_ms = 500;
  static uint32_t last_ms = millis();

  int btn = digitalRead(gpio_btn);
  if (btn == 1)
    last_ms = millis();

  if ((millis() - last_ms) >= btn_timeout_ms) {

#if USE_PRINTER
    for (int i = 0; i < 5; i++)
      printer.print('\n');
#endif

    // delay to (maybe) finish printing a digit and scroll the blank lines
    delay(1000);

    // Power off. Is there a race condition where the button is pressed again
    // just as we try to power off? Maybe? Put the power off in a loop in case.
    while (true) {
      digitalWrite(gpio_off, 1);
      delay(1);
    }
  }
}


// Check battery. Called once at startup. If battery is below battery_min_mv,
// print a message and power off.
static void check_battery()
{
  int adc = analogRead(adc_batt);
  int mv = (adc * 625L + 64) / 128; // 64 is for rounding

  if (mv >= battery_min_mv) {

#if 0
    snprintf(pbuf, pbuf_len-1, "Battery: %d mV", mv); // 23 chars max
    Serial.println(pbuf);
#if USE_PRINTER
    printer.print(pbuf);
#endif
#endif

    return;
  }

  // does not return

  snprintf(pbuf, pbuf_len-1, "CHARGE BATTERY (%d mV)", mv); // 31 chars max

  Serial.println(pbuf);

#if USE_PRINTER
  printer.print(pbuf);
  for (int i = 0; i < 5; i++)
    printer.print('\n');
#endif

  // let the printer print, and maybe allow connecting debugger
  delay(2000);
  while (true) {
    digitalWrite(gpio_off, 1);
    delay(1);
  }

} // check_battery


// print a digit
static void print_digit(char digit)
{
  check_button();

  // Limit print rate so we don't queue up lines in the receive buffer,
  // which breaks paper-out handling. This is different from worrying
  // about overrunning the receive buffer; we basically want to know a
  // digit is on the paper before we try to print another one.
  static uint32_t last_print_ms = millis();
  while ((millis() - last_print_ms) < print_interval_ms)
    delay(1);
  last_print_ms = millis();

  // large font is 12 pixels wide x 24 pixels high

  // rotated and doubled digit is 48 pixels wide
  // digit should be centered on 384/2 = 192, so should print at
  // 192 - (48 / 2) = 168

  // line will be:
  // digit number: 12 chars @ 12 pixels = 144 pixels
  // border:        2 chars @ 12 pixels =  24 pixels
  // digit:         1 char  @ 48 pixels =  48 pixels
  // border:        2 chars @ 12 pixels =  24 pixels
  // nothing:      12 chars @ 12 pixels = 144 pixels
  // total:                               384 pixels

  // only print number if digit is 0..9
  if ('0' <= digit && digit <= '9')
    snprintf(pbuf, pbuf_len-1, "%-12ld| %c |", digit_num, digit); // 18 chars
  else
    snprintf(pbuf, pbuf_len-1, "            | %c |", digit); // 18 chars
  pbuf[pbuf_len-1] = '\0';

  Serial.println(pbuf);

#if USE_PRINTER

  // digit number
  if ('0' <= digit && digit <= '9')
    snprintf(pbuf, pbuf_len-1, "%-12ld", digit_num); // 13 chars
  else
    strncpy(pbuf, "            ", pbuf_len-1); // 13 chars
  pbuf[pbuf_len-1] = '\0';

  printer.rotate(false);
  printer.mode(Printer::Modes::FontLarge);
  printer.print(pbuf);
  printer.print(0xb2); // gray box
  printer.print(' ');

  // digit
  printer.rotate(true);
  printer.mode(Printer::Modes::FontLarge | Printer::Modes::BoldOn |
               Printer::Modes::Height2x | Printer::Modes::Width2x);
  printer.print(digit);

  printer.rotate(false);
  printer.mode(Printer::Modes::FontLarge);
  printer.print(' ');
  printer.print(0xb2); // gray box

  printer.line_space(0);  // lines as close together as possible
  printer.flush();        // print buffered data and advance paper

  printer.mode(); // defaults

#endif // USE_PRINTER

} // print_digit


void setup()
{
  Serial.begin(115200);

  pinMode(gpio_btn, INPUT);

  // do not glitch high
  digitalWrite(gpio_off, 0);
  pinMode(gpio_off, OUTPUT);

#if WAIT_CONSOLE
  while (!Serial)
    ;
  delay(250);
#endif

  // printer.begin() sets the baud rate and sends a reset command,
  // but does not wait for any responses from the printer
  printer.begin(printer_baud);
  delay(500);

  // This intertwines with the minimum allowed battery voltage. The more
  // current the printer pulls, the higher the threshold has to be. When the
  // printer prints a line, the voltage from the battery dips (due to IR of
  // the battery) and can hit the battery protection cutoff. With (5, 200, 1),
  // the drain test program works to around a measured voltage of 3.6 V.
  // Presumably, as the battery ages, the IR (and drop) will be greater.
  printer.heat(5, 200, 1);

  check_battery(); // doesn't return if battery too low

} // setup


void loop()
{
  if (digit_num < pi_len) {
    print_digit(pgm_read_byte(pi + digit_num));
    if (digit_num == 0) {
      print_digit('.');
    }
    digit_num++;
  } else if (digit_num == pi_len) {
    Serial.println("No more pi!");
#if USE_PRINTER
    printer.print("No more pi!\n");
#endif
    digit_num++;
  } else {
    // digit_num just sits at pi_len+1 forever
  }
}
