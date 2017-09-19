/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * ...
 */

#ifndef _DIAGSTREAM_H
#define _DIAGSTREAM_H

#include <Stream.h>

class DiagStream : public Stream
{
public:
  virtual ~DiagStream() {}

  bool isEnabled() { return _isEnabled; }
  void setStream(Stream * stream);
  void setStream(Stream & stream);

  void enableConsole() { _disable_console = false; }
  void disableConsole() { _disable_console = true; }
  void setConsoleStream(Stream * stream);
  void setConsoleStream(Stream & stream);

  // A few utility functions
  void print02x(uint8_t val);
  void dumpBuffer(const uint8_t * buf, size_t len);

  // virtual functions in Stream
  // These only read from the "main" diag stream.
  int available();
  int read();
  int peek();

  // Flush all available output streams
  void flush();
  // virtual functions in Print
  // Write the byte to all available output streams
  size_t write(uint8_t);

private:
  void setIsEnabled();

  bool _isEnabled;
  Stream * _stream;

  bool  _disable_console;
  Stream * _console_stream;
};
extern DiagStream diag_stream;

#endif
