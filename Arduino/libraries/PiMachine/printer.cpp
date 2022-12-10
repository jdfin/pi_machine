
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
  // XXX: Should check for no-response condition, either by
  //      changing status() to return error, or checking fixed
  //      bits in returned status byte.
  return (status(4) & 0x40) == 0; // bit 0x40 set means paper out
}


void Printer::rotate(bool rotate)
{
  uint8_t cmd[] = { 0x1b, 0x56, uint8_t(rotate ? 0x01 : 0x00) };
  _port.write(cmd, sizeof(cmd));
}


void Printer::mode(uint8_t mode)
{
  uint8_t cmd[] = { 0x1b, 0x21, mode };
  _port.write(cmd, sizeof(cmd));
}


void Printer::line_space(int dots)
{
  uint8_t cmd[] = { 0x1b, 0x33, uint8_t(dots) };
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
  uint8_t cmd[] = { 0x1b, 0x4a, uint8_t(dots) };
  _port.write(cmd, sizeof(cmd));
}


// Read and return one of the "real-time transmission status" bytes.
//
// Mini Thermal Printer:
//   First time after reset has been observed to take up to 18.7 msec
//   (usually 13.5 msec). After that, it's about 8.3 msec.
//   Update: with a 30 msec timeout, sometimes it still times out; have
//   not verified that the response came back after 30 msec, but a 100
//   msec timeout works better.
//
// Nano Thermal Printer:
//   First time after reset has been observed to take up to 8.2 msec
//   (usually 6.2 msec). After that, it's about 4.2 msec.
//
// Sending a reset, then immediately sending this command, seems to be okay.
//
uint8_t Printer::status(int which)
{
  const uint32_t timeout_us = 100000; // 100 msec

  // read and discard any old data (usually none)
  while (_port.read() != -1)
    ;

  int b;
  uint8_t cmd[] = { 0x10, 0x04, uint8_t(which) };

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


void Printer::heat(uint8_t n1, uint8_t n2, uint8_t n3)
{
  uint8_t cmd[] = { 0x1b, 0x37, n1, n2, n3 };
  _port.write(cmd, sizeof(cmd));
}
