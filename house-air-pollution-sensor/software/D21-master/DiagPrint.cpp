/*
 * DiagPrint.cpp
 *
 * Created: 17/12/2015
 *  Author: Gabriel Notman
 */

#include <Arduino.h>

#include "DiagPrint.h"

Stream * diag_stream;

void setDiagStream(Stream * stream)
{
  diag_stream = stream;
}

void setDiagStream(Stream & stream)
{
  diag_stream = &stream;
}

void print02X(String prefix, uint8_t* buff, size_t len)
{
  if (!diag_stream) {
    return;
  }
  size_t index = 0;
  while (index < len) {
    if ((index % 16) == 0) {
      if (index != 0) {
        diag_stream->println();
      }
      diag_stream->print(prefix);
    }
    print02X(buff[index]);
    index++;
  }
}

void print02X(uint8_t val)
{
  if (!diag_stream) {
    return;
  }
  uint8_t highVal = (val >> 4);
  uint8_t lowVal = (val & 0b00001111);

  char highNibble = (highVal > 9) ? (char)highVal + 'A' - 10 : (char)highVal + '0';
  char lowNibble = (lowVal > 9) ? (char)lowVal + 'A' - 10 : (char)lowVal + '0';

  diag_stream->print(highNibble);
  diag_stream->print(lowNibble);
  diag_stream->print(" ");
}
