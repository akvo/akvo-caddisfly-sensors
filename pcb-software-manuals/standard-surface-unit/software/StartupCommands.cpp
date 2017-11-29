/*
 * Based on: SQ_StartupCommands.cpp
 *
 *  Created on: Mar 27, 2014
 *      Author: Kees Bakker
 */

/*
 * Read commands from the "console" (usually SERIAL_PORT_MONITOR)
 * and optionally from a diagport (usually one of the UARTs)
 * and execute them.
 * This can be used to configure the device (and store parameters
 * in EEPROM), or there can be all sorts of debugging commands.
 */

#include <Arduino.h>

#include "Settings.h"
#include "Command.h"
#include "DiagStream.h"
#include "RTCZero.h"
#include "Utils.h"

#include "StartupCommands.h"

#ifndef TIME_FOR_STARTUP_COMMANDS
#define TIME_FOR_STARTUP_COMMANDS  (20 * 1000)
#endif

#define ENABLE_START_COMMANDS_DIAG     1

#define DEBUG_PREFIX String("[STC]")

static Stream * myDiagPort;
#if ENABLE_START_COMMANDS_DIAG
  #define myDiagPrint(...) diagPrint(__VA_ARGS__)
  #define myDiagPrintLn(...) diagPrintLn(__VA_ARGS__)
#else
  #define myDiagPrint(...)
  #define myDiagPrintLn(...)
#endif

typedef struct {
  const char    * banner;
  uint8_t       nrCmnds;
  const Command * cmds;
  void          (*preShow)(Stream * stream);
  void          (*postExec)(void);
} MyCmdList_t;
MyCmdList_t     myCmdLists[4];
uint8_t         nrMyCmdLists;

static void showMyCommandLists(Stream * stream)
{
  for (uint8_t i = 0; i < nrMyCmdLists; ++i) {
    if (myCmdLists[i].preShow) {
      myCmdLists[i].preShow(stream);
    }
    if (myCmdLists[i].cmds) {
      stream->println();
      stream->println(reinterpret_cast<const __FlashStringHelper *>(myCmdLists[i].banner));
      for (uint8_t j = 0; j < myCmdLists[i].nrCmnds; ++j) {
        const Command * cmd = &myCmdLists[i].cmds[j];
        cmd->show_func(cmd, stream);
      }
    }
  }
}

/*
 * Execute a command from the commandline
 *
 * Return true if it was a valid command
 */
static bool execMyCommand(const char * line)
{
  bool done = false;
  for (uint8_t i = 0; !done && i < nrMyCmdLists; ++i) {
    if (myCmdLists[i].cmds) {
      done = Command::execCommand(myCmdLists[i].cmds, myCmdLists[i].nrCmnds, line);
      if (done) {
        if (myCmdLists[i].postExec) {
          (*myCmdLists[i].postExec)();
        }
      }
    }
  }
  return done;
}

static int readLine(Stream * stream, char line[], size_t size, uint32_t & ts_max)
{
  int c;
  size_t len = 0;
  bool seenCR = false;
  uint32_t ts_waitLF = 0;
  while (!isTimedOut(ts_max)) {
    if (seenCR) {
      c = stream->peek();
      // ts_waitLF is guaranteed to be non-zero
      if ((c == -1 && isTimedOut(ts_waitLF)) || (c != -1 && c != '\n')) {
        goto end;
      }
      // Only \n should fall through
    }

    c = stream->read();
    // Ignore NUL bytes too
    if (c <= 0) {
      continue;
    }

    // There is input, so extend the timeout
    ts_max = millis() + TIME_FOR_STARTUP_COMMANDS;

    seenCR = c == '\r';
    if (c == '\r') {
      stream->write((char)c);
      ts_waitLF = millis() + 50;        // Wait another .05 sec for an optional LF
    } else if (c == '\n') {
      stream->write((char)c);
      goto end;
    } else if (c == 0x08 || c == 0x7f) {
      // Erase the last character
      if (len > 0) {
        stream->write("\b \b");
        --len;
      }
    } else {
      // Any "normal" character is stored in the line buffer
      if (len < size - 1) {
        if (c >= ' ' && c < 0x7f) {
          stream->write((char)c);
          line[len++] = c;
        }
      }
    }
  }
  stream->write("\r\n");            // Make sure the terminal is at the next new line
  // Timed out. Ignore the input.
  line[0] = '\0';
  return -1;

end:
  stream->write('\n');            // Make sure the terminal is at the next new line
  line[len] = '\0';
  return len;
}

static void showCommandPrompt(Stream * stream)
{
  showMyCommandLists(stream);
  stream->println();
  stream->print("Enter command: ");
  if (myDiagPort && stream != myDiagPort) {
    showCommandPrompt(myDiagPort);
  }
}

/*
 * \brief Read commands from a serial device
 *
 * \param stream is the Stream from which to read commands
 *
 * Besides from the standard stream we can also read from the diagport
 * if it is enabled
 */
void startupCommands(Stream & stream)
{
  char buffer[200+1];
  int size;
  uint32_t ts_max = millis() + TIME_FOR_STARTUP_COMMANDS;
  bool needPrompt;
  bool seenCommand;
  uint8_t nrPrompts;

  needPrompt = true;
  nrPrompts = 0;

  while (!isTimedOut(ts_max) && nrPrompts < 250) {
    if (needPrompt) {
      showCommandPrompt(&stream);
      needPrompt = false;
      ++nrPrompts;
    }

    size = -1;
    while (stream.available() && stream.peek() == 0) {
      stream.read();    // Ignore initial NUL bytes on input
    }

    if (stream.available()) {
      size = readLine(&stream, buffer, sizeof(buffer), ts_max);
    }
    if (size <= 0) {
      // Read from diagport too, if diagport is not the same as stream
      if (myDiagPort && &stream != myDiagPort) {
        if (myDiagPort->available()) {
          size = readLine(myDiagPort, buffer, sizeof(buffer), ts_max);
        }
      }
    }

    if (size < 0) {
      continue;
    }

    needPrompt = true;
    if (size == 0) {
      continue;
    }

    if (strcasecmp(buffer, "ok") == 0) {
      break;
    }

    seenCommand = false;

    // Is this a command for us?
    if (!seenCommand && execMyCommand(buffer)) {
      seenCommand = true;
    }

    if (seenCommand) {
      ts_max = millis() + TIME_FOR_STARTUP_COMMANDS;
    }
  }
  stream.println();
  if (myDiagPort && &stream != myDiagPort) {
    myDiagPort->println();
  }
}

void supComRegisterDiagPort(Stream * stream)
{
  myDiagPort = stream;
}

void supComRegisterDiagPort(Stream & stream)
{
  myDiagPort = &stream;
}

void supComRegisterCmnds(const char * banner,
    const Command * cmds, uint8_t nrCmds,
    void (*preShow)(Stream * stream),
    void (*postExec)(void))
{
  if (nrMyCmdLists < sizeof(myCmdLists) / sizeof(myCmdLists[0])) {
    myCmdLists[nrMyCmdLists].banner = banner;
    myCmdLists[nrMyCmdLists].cmds = cmds;
    myCmdLists[nrMyCmdLists].nrCmnds = nrCmds;
    myCmdLists[nrMyCmdLists].preShow = preShow;
    myCmdLists[nrMyCmdLists].postExec = postExec;
    ++nrMyCmdLists;
  }
}
