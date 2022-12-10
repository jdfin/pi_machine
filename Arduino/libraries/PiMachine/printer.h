#pragma once

#include <Arduino.h>


class Printer {

  public:

    Printer(HardwareSerial& port);

    void begin(int baud);

    void reset();

    // returns true if paper present, false if paper not detected
    // (this call takes several msec)
    bool paper();

    void rotate(bool rotate);

    enum Modes {
      FontLarge = 0x00, FontSmall = 0x01,
      BoldOff   = 0x00, BoldOn    = 0x08,
      Height1x  = 0x00, Height2x  = 0x10,
      Width1x   = 0x00, Width2x   = 0x20,
      UnderOff  = 0x00, UnderOn   = 0x80,
    };

    void mode(uint8_t mode=0);

    void line_space(int dots=30);

    void print(char c);
    void print(const char *s);

    void flush(int dots=0);

    uint8_t status(int which);

    void heat(uint8_t n1, uint8_t n2, uint8_t n3);

    // informational
    uint32_t response_us;

  private:

    HardwareSerial& _port;

};
