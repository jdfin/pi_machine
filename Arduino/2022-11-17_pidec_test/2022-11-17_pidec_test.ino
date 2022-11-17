#include <Arduino.h>
#include <stdio.h>
#include <pidec.h>

#include "pi16k.h"


void ms_to_hms(uint32_t ms, uint32_t& h, uint32_t& m, uint32_t& s)
{
  s = ms / 1000;

  m = s / 60;
  s = s - (m * 60);

  h = m / 60;
  m = m - (h * 60);
}


uint32_t start_ms;

// first digit after the decimal (1) is digit zero
uint32_t digNum = 0;

// chars per line of block output
// if zero, one char per line with index and timestamp
const int digits_per_line = 64;
int digits_this_line = 0;


void emit(int digit)
{
  bool err = (digNum > 0 && digNum < 16382 && digit != (pi16k[digNum+1] - '0'));

  if (digits_per_line > 0) {

    if (err)
      Serial.print('/');

    Serial.print(digit);
    if (++digits_this_line >= digits_per_line) {
      Serial.println();
      digits_this_line = 0;
    }

  } else {

    // similar to what will go to the printer
    uint32_t h, m, s;
    ms_to_hms(millis() - start_ms, h, m, s);

    char buf[50];
    sprintf(buf, "%-18lu%d%13lu:%02lu:%02lu", digNum, digit, h, m, s);
    Serial.print(buf);

    if (err)
      Serial.print("  ERROR");

    Serial.println();

  }

} // emit


void setup()
{
  const char *FirstDigits = "1415926535897932384626433832795028841971693993751058209";

  Serial.begin(115200);
  while (!Serial)
    ;

  delay(250);

  start_ms = millis();

  emit(3);
  for (digNum = 0; digNum < 50; digNum++) {
    int digit = FirstDigits[digNum] - '0';
    emit(digit);
  }

}


void loop()
{
  double x = DigitsOfPi(digNum);
  double pow = 1.e9;
  double y = x * pow;
  int digit = int32_t(y) / 100000000;
  emit(digit);
  digNum++;
}
