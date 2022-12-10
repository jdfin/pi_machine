// TinyPiMachine Button Test

#include <Arduino.h>
#include <tiny_pi_machine.h>


void setup()
{
  Serial.begin(115200);

  while (!Serial)
    ;

  pinMode(gpio_btn, INPUT);

}


void loop()
{
  static int btn_last = -1;

  int btn = digitalRead(gpio_btn);

  if (btn != btn_last)
    Serial.println(btn);

  btn_last = btn;
}
