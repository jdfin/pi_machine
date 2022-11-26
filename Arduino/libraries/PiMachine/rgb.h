#pragma once

#include <Arduino.h>

// Common-Anode RGB LED (three GPIOs, low is on)

class Rgb {

  public:

    Rgb(int red_pin, int green_pin, int blue_pin) :
      _red_pin(red_pin),
      _green_pin(green_pin),
      _blue_pin(blue_pin),
      _pat_num(0),
      _rgb1(0),
      _ms1(0),
      _rgb2(0),
      _ms2(0),
      _next_ms(0)
    {
    }

    ~Rgb()
    {
      set(Off);
      pinMode(_red_pin, INPUT);
      pinMode(_green_pin, INPUT);
      pinMode(_blue_pin, INPUT);
    }
      
    void begin()
    {
      set(Off);
      pinMode(_red_pin, OUTPUT);
      pinMode(_green_pin, OUTPUT);
      pinMode(_blue_pin, OUTPUT);
    }

    enum {
      White = 7,
      Red = 4,
      Green= 2,
      Blue = 1,
      Off = 0,
    };

    void set(int rgb)
    {
      _set(rgb);
      _pat_num = 0;
    }

    void pattern(int rgb1, int ms1, int rgb2, int ms2)
    {
      if (rgb1 == _rgb1 && ms1 == _ms1 &&
          rgb2 == _rgb2 && ms2 == _ms2 &&
          _pat_num != 0)
        return;

      _rgb1 = rgb1;
      _ms1 = ms1;

      _rgb2 = rgb2;
      _ms2 = ms2;

      _pat_num = 1;
      _set(_rgb1);
      _next_ms = millis() + _ms1;
    }

    void loop()
    {
      if (_pat_num == 1) {
        if (millis() < _next_ms)
          return;
        _pat_num = 2;
        _set(_rgb2);
        _next_ms += _ms2;
      } else if (_pat_num == 2) {
        if (millis() < _next_ms)
          return;
        _pat_num = 1;
        _set(_rgb1);
        _next_ms += _ms1;
      }
    }

  private:

    int _red_pin;
    int _green_pin;
    int _blue_pin;

    // pattern support
    int _pat_num; // 1 or 2
    int _rgb1;
    int _ms1;
    int _rgb2;
    int _ms2;
    uint32_t _next_ms;

    void _set(int rgb)
    {
      // first, all off...
      digitalWrite(_red_pin, 1);
      digitalWrite(_green_pin, 1);
      digitalWrite(_blue_pin, 1);
      // ...then turn on as requested
      if (rgb & Red) digitalWrite(_red_pin, 0);
      if (rgb & Green) digitalWrite(_green_pin, 0);
      if (rgb & Blue) digitalWrite(_blue_pin, 0);
    }

};
