/*
 * Based on: SQ_StartupCommands.cpp by Kees Bakker
 */

/*
 * Reads commands from the serial line (diagport and/or Console)
 * and executes them.
 * This can be used to configure the device (and store parameters
 * in FLASH), or there can be all sorts of debugging commands (if
 * enabled).
 */

#define TIME_FOR_STARTUP_COMMANDS  (20 * 1000)

#include <Arduino.h>

#include "ProjectDefinitions.h"
#include "Config.h"
#include "Command.h"
#include "RTCZero.h"
#include "Utils.h"
#include "HapStorage.h"
#include "PrintSystem.h"

#include "BootupCommands.h"

#define ENABLE_START_COMMANDS_DIAG 1

#define DEBUG_PREFIX "[STC]"

#define ENABLE_DATAFLASH_COMMANDS 1

#if ENABLE_DATAFLASH_COMMANDS

#include "SumDataRecord.h"
#include "SensorDataRecord.h"

static void clearSumRecords(const Command* a, const char* line)
{
    consolePrintLn("Clearing SUM records...");
    hapStorage.clearSumRecords();
    consolePrintLn("Finished clearing SUM records!");
}

static void clearSensorRecords(const Command* a, const char* line)
{
    consolePrintLn("Clearing Sensor records...");
    hapStorage.clearSensorRecords();
    consolePrintLn("Finished clearing Sensor records!");
}

static void dumpRecords(const Command* a, const char* line, DataflashUtils& dflashUtils, DataRecord& record)
{
    consolePrintLn("Dumping records...");
    size_t count = 0;

    int page = dflashUtils.getUploadPage();

    record.printHeaderLn(printSystem.getConsole());

    while (dflashUtils.isValidUploadPage(page)) {
        uint16_t recNum = 0;
        while (dflashUtils.readPageNthRecord(page, recNum, record.getBuffer(), record.getSize())) {
            consolePrintLn("[", page, ",", recNum, "] ");
            record.printRecordLn(printSystem.getConsole());
            recNum++;
            count++;
        }
        page = dflashUtils.getNextPage(page);
    }
    consolePrintLn("Finished dumping ", count, " records!");
}

static void dumpSumRecords(const Command* a, const char* line)
{
    SumDataRecord record;
    dumpRecords(a, line, hapStorage.getSumDataHelper(), record);
}

static void dumpSensorRecords(const Command* a, const char* line)
{
    SensorDataRecord record;
    dumpRecords(a, line, hapStorage.getSensorDataHelper(), record);
}

#endif

static void commitSettings(const Command* a, const char* line)
{
    DFlashSegment& configSegment = hapStorage.getConfigSegment();

    consolePrintLn("Committing settings...");
    params.commit(configSegment);
    consolePrintLn("Finished committing setting!");
}

static const Command args[] = {
#if ENABLE_DATAFLASH_COMMANDS
    { "Dump SUM Records", "DUR", dumpSumRecords, Command::show_string },
    { "Dump Sensor Records", "DSR", dumpSensorRecords, Command::show_string },
    { "Clear SUM Records", "CLRUR", clearSumRecords, Command::show_string },
    { "Clear Sensor Records", "CLRSR", clearSensorRecords, Command::show_string },
#endif
    { "Commit Settings", "CS", commitSettings, Command::show_string }
};

static void showMyCommands(Stream* stream)
{
    size_t nr_cmnds = sizeof(args) / sizeof(args[0]);
    if (nr_cmnds == 0) {
        return;
    }
    stream->println("\r\nCommands:");
    for (size_t i = 0; i < nr_cmnds; ++i) {
        const Command* a = &args[i];
        if (a->show_func) {
            a->show_func(a, stream);
        }
    }
}

/*
 * Execute a command from the commandline
 *
 * Return true if it was a valid command
 */
static bool execCommand(const char* line)
{
    return Command::execCommand(args, sizeof(args) / sizeof(args[0]), line);
}

static int readLine(Stream* stream, char line[], size_t size, uint32_t& ts_max)
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
            ts_waitLF = millis() + 50; // Wait another .05 sec for an optional LF
        }
        else if (c == '\n') {
            stream->write((char)c);
            goto end;
        }
        else if (c == 0x08 || c == 0x7f) {
            // Erase the last character
            if (len > 0) {
                stream->write("\b \b");
                --len;
            }
        }
        else {
            // Any "normal" character is stored in the line buffer
            if (len < size - 1) {
                if (c >= ' ' && c < 0x7f) {
                    stream->write((char)c);
                    line[len++] = c;
                }
            }
        }
    }
    // Timed out. Ignore the input.
    line[0] = '\0';
    return -1;

end:
    line[len] = '\0';
    return len;
}

static void showCommandPrompt(Stream* stream)
{
    showMyCommands(stream);
    ConfigParams::showSettings(stream);
    stream->print("Enter command: ");

    if (printSystem.getDebug() && printSystem.getDebug() != stream) {
        showCommandPrompt(printSystem.getDebug());
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
void startupCommands(Stream& stream)
{
    char buffer[200 + 1];
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
            stream.read(); // Ignore initial NUL bytes on input
        }

        if (stream.available()) {
            size = readLine(&stream, buffer, sizeof(buffer), ts_max);
        }
        if (size <= 0) {
            // Read from diagport too, if diagport is not the same as stream
            if (printSystem.getDebug() && printSystem.getDebug() != &stream) {
                if (printSystem.getDebug()->available()) {
                    size = readLine(printSystem.getDebug(), buffer, sizeof(buffer), ts_max);
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
        if (!seenCommand && execCommand(buffer)) {
            seenCommand = true;
        }
        // Is this a command for EEPROM config?
        if (!seenCommand && params.execCommand(buffer)) {
            seenCommand = true;
        }

        if (seenCommand) {
            ts_max = millis() + TIME_FOR_STARTUP_COMMANDS;
        }
    }
    stream.println();
    if (printSystem.getDebug() && printSystem.getDebug() != &stream) {
        printSystem.getDebug()->println();
    }
}
