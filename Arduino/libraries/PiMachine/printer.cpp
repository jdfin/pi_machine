
#include <Arduino.h>
#include <printer.h>


Printer::Printer(HardwareSerial& port) :
  _port(port)
{
}


void Printer::begin(int baud)
{
  _port.begin(baud);
  while (!_port)
    ;
  reset();
}


void Printer::reset()
{
  const uint8_t cmd[] = { 0x1b, 0x40 };
  _port.write(cmd, sizeof(cmd));
}


bool Printer::paper()
{
  return (status(4) & 0x40) == 0; // bit 0x40 set means paper out
}


void Printer::rotate(bool rotate)
{
  uint8_t cmd[] = { 0x1b, 0x56, rotate ? 0x01 : 0x00 };
  _port.write(cmd, sizeof(cmd));
}


void Printer::mode(uint8_t mode)
{
  uint8_t cmd[] = { 0x1b, 0x21, mode };
  _port.write(cmd, sizeof(cmd));
}


void Printer::line_space(int dots)
{
  uint8_t cmd[] = { 0x1b, 0x33, dots };
  _port.write(cmd, sizeof(cmd));
}


void Printer::print(char c)
{
  _port.write(c);
}


void Printer::print(const char *s)
{
  _port.write(s);
}


void Printer::flush(int dots)
{
  // print and advance a number of pixels, but always advance at least
  // the height of what's in the print buffer
  uint8_t cmd[] = { 0x1b, 0x4a, dots };
  _port.write(cmd, sizeof(cmd));
}


// Read and return one of the "real-time transmission status" bytes.
//
// Mini Thermal Printer:
//   First time after reset has been observed to take up to 18.7 msec
//   (usually 13.5 msec). After that, it's about 8.3 msec.
//
// Nano Thermal Printer:
//   First time after reset has been observed to take up to 8.2 msec
//   (usually 6.2 msec). After that, it's about 4.2 msec.
//
// Sending a reset, then immediately sending this command, seems to be okay.
//
uint8_t Printer::status(int which)
{
  const uint32_t timeout_us = 30000; // 30 msec

  // read and discard any old data (usually none)
  while (_port.read() != -1)
    ;

  int b;
  uint32_t now_us;
  uint8_t cmd[] = { 0x10, 0x04, which };

  _port.write(cmd, sizeof(cmd));
  uint32_t start_us = micros();
  do {
    // response_us is public member so we can see how long it took
    response_us = micros() - start_us;
    b = _port.read();
  } while (b == -1 && response_us <= timeout_us);

  if (b == -1)
    return 0; // no response

  return uint8_t(b);
}
