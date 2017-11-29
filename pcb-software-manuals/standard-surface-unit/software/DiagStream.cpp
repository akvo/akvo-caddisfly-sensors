/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * This module takes care of diagnostic output.
 */

#include <unistd.h>
#include <Stream.h>

#include "DiagStream.h"


DiagStream diag_stream;

void DiagStream::setConsoleStream(Stream * stream)
{
  _console_stream = stream;
  setIsEnabled();
}

void DiagStream::setConsoleStream(Stream & stream)
{
  _console_stream = &stream;
  setIsEnabled();
}

void DiagStream::setStream(Stream * stream)
{
  _stream = stream;
  setIsEnabled();
}

void DiagStream::setStream(Stream & stream)
{
  _stream = &stream;
  setIsEnabled();
}

void DiagStream::setIsEnabled()
{
  _isEnabled = false;
  if (_stream || _console_stream) {
    _isEnabled = true;
  }
}

int DiagStream::available()
{
  if (_stream) {
    return _stream->available();
  }
  return -1;
}

int DiagStream::read()
{
  if (_stream) {
    return _stream->read();
  }
  return -1;
}

int DiagStream::peek()
{
  if (_stream) {
    return _stream->peek();
  }
  return -1;
}

void DiagStream::flush()
{
  if (_stream) {
    _stream->flush();
  }
  if (!_disable_console && _console_stream) {
    _console_stream->flush();
  }
}

size_t DiagStream::write(uint8_t b)
{
  if (_stream) {
    _stream->write(b);
  }
  if (!_disable_console && _console_stream) {
    _console_stream->write(b);
  }
  return 1;
}

////////////////////////////////////////////////////////////////////////////////
////////////////   UTILITY FUNCTIONS  //////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void DiagStream::print02x(uint8_t val)
{
    if (val < 0x10) {
        print('0');
    }
    print(val, HEX);
}

void DiagStream::dumpBuffer(const uint8_t * buf, size_t len)
{
    while (len > 0) {
        size_t len1 = len >= 16 ? 16 : len;
        for (size_t j = 0; j < len1; j++) {
            print02x(*buf++);
        }
        println();
        len -= len1;
    }
}
