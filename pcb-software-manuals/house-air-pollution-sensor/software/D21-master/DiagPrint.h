/*
 * DiagPrint.h
 *
 * Created: 17/12/2015
 *  Author: Gabriel Notman
 */

#ifndef DIAG_PRINT_H_
#define DIAG_PRINT_H_

#include <Arduino.h>

extern Stream * diag_stream;
void setDiagStream(Stream * stream);
void setDiagStream(Stream & stream);
#define diagPrint(...) do { if (diag_stream) diag_stream->print(__VA_ARGS__); } while(0)
#define diagPrintLn(...) do {if (diag_stream) diag_stream->println(__VA_ARGS__); } while (0)


void print02X(String prefix, uint8_t* buff, size_t len);
void print02X(uint8_t val);

#endif /* DIAG_PRINT_H_ */
