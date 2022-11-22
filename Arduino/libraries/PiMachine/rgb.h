#pragma once

#include <Arduino.h>

// Common-Anode RGB LED (three GPIOs, low is on)

class Rgb {

  public:

    Rgb(int red_pin, int green_pin, int blue_pin) :
      _red_pin(red_pin),
      _green_pin(green_pin),
      _blue_pin(blue_pin)
    {
    }

    ~Rgb()
    {
      off();
      pinMode(_red_pin, INPUT);
      pinMode(_green_pin, INPUT);
      pinMode(_blue_pin, INPUT);
    }
      
    void begin()
    {
      off();
      pinMode(_red_pin, OUTPUT);
      pinMode(_green_pin, OUTPUT);
      pinMode(_blue_pin, OUTPUT);
    }

    void off() { set(false, false, false); }
    void red() { set(true, false, false); }
    void green() { set(false, true, false); }
    void blue() { set(false, false, true); }
    void white() { set(true, true, true); }

    void set(bool red, bool green, bool blue)
    {
      digitalWrite(_red_pin, 1);
      digitalWrite(_green_pin, 1);
      digitalWrite(_blue_pin, 1);
      if (red) digitalWrite(_red_pin, 0);
      if (green) digitalWrite(_green_pin, 0);
      if (blue) digitalWrite(_blue_pin, 0);
    }

    void set(int rgb)
    {
      set((rgb & 4) != 0, (rgb & 2) != 0, (rgb & 1) != 0);
    }

  private:

    int _red_pin;
    int _green_pin;
    int _blue_pin;
};
