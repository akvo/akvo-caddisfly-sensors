/*
 * Based on: Config.cpp
 *
 *  Created on: Mar 27, 2014
 *      Author: Kees Bakker
 */

/*
 * This module is responsible for the configuration of the SODAQ
 * device.
 */

#include <SPI.h>
#include <stdint.h>
#include "ProjectDefinitions.h"
#include "Command.h"
#include "Config.h"
#include "DiagPrint.h"
#include "Sodaq_segmented_df.h"
#include "Utils.h"

#define ENABLE_CONFIG_DIAG             1

#define DEBUG_PREFIX String("[CONF]")

#if ENABLE_CONFIG_DIAG
#define myDiagPrint(...) diagPrint(__VA_ARGS__)
#define myDiagPrintLn(...) diagPrintLn(__VA_ARGS__)
#else
  #define myDiagPrint(...)
  #define myDiagPrintLn(...)
#endif

static const char magic[] = "SODAQ HAP";

ConfigParams params;
static bool needCommit;

/*
 * Read all of the config parameters from the EEPROM
 *
 * There are some sanity checks. If they fail then
 * it will call reset() instead.
 */
void ConfigParams::read(DFlashSegment& configSeg)
{
    const size_t crc_size = sizeof(uint16_t);
    const size_t magic_len = sizeof(magic);
    size_t size = magic_len + sizeof(*this) + crc_size;
    uint8_t buffer[size];

    myDiagPrint(DEBUG_PREFIX + "Magic_len=");
    myDiagPrintLn(magic_len);
    myDiagPrint(DEBUG_PREFIX + "Size=");
    myDiagPrintLn(size);
    myDiagPrint(DEBUG_PREFIX + "crc_size=");
    myDiagPrintLn(crc_size);

    configSeg.readPageToBuf(CONFIG_DF_PAGE);
    configSeg.readStr(CONFIG_DF_ADDRESS, buffer, size);
#if ENABLE_CONFIG_DIAG
    print02X(DEBUG_PREFIX, buffer, size);
    myDiagPrintLn();
#endif

    uint16_t crc = *(uint16_t *)(buffer + size - crc_size);
    uint16_t crc1 = crc16ccitt(buffer, size - crc_size);

    if (strncmp_P((const char *)buffer, magic, magic_len) != 0) {
        myDiagPrintLn(DEBUG_PREFIX + "ConfigParms::read - magic wrong");
        goto do_reset;
    }
    myDiagPrintLn(DEBUG_PREFIX + "ConfigParms::read - magic OK");
    myDiagPrint(DEBUG_PREFIX + "crc=");
    myDiagPrintLn(crc);
    myDiagPrint(DEBUG_PREFIX + "crc1=");
    myDiagPrintLn(crc1);
    if (crc != crc1) {
        goto do_reset;
    }
    memcpy((uint8_t *)this, buffer + magic_len, sizeof(*this));
    return;

do_reset:
    reset();
}

void ConfigParams::reset()
{
    myDiagPrintLn(DEBUG_PREFIX + "ConfigParms::reset");
    memset(this, 0, sizeof(*this));

    _aggregationInterval = PARAM_AGGREGATION_INTERVAL;
    _uploadInterval = PARAM_UPLOAD_INTERVAL;
    _rtcSyncInterval = PARAM_RTCSYNC_INTERVAL;

    needCommit = true;
}

/*
 * Write the configuration parameters to NVM / Dataflash
 */
void ConfigParams::commit(DFlashSegment& configSeg, bool forced)
{
    if (!forced && !needCommit) {
        return;
    }

    myDiagPrintLn(DEBUG_PREFIX + "ConfigParams::commit");
    // Fill in the magic and CRC, and write to EEPROM
    const size_t crc_size = sizeof(uint16_t);
    const size_t magic_len = sizeof(magic);
    size_t size = magic_len + sizeof(*this) + crc_size;
    uint8_t buffer[size];

    strncpy_P((char *)buffer, magic, magic_len);
    memcpy(buffer + magic_len, (uint8_t *)this, sizeof(*this));
    uint16_t crc = crc16ccitt(buffer, size - crc_size);
    *(uint16_t *)(buffer + size - crc_size) = crc;
    print02X(DEBUG_PREFIX, buffer, size);
    myDiagPrintLn();

    configSeg.writeStr(CONFIG_DF_ADDRESS, buffer, size);
    configSeg.writeBufToPage(CONFIG_DF_PAGE);

    needCommit = false;
}

static const Command args[] = {
//    {"Sample         ", "sm=", Command::set_uint16, Command::show_uint16, &params._sm},
    {"Aggregate      ", "ag=", Command::set_uint16, Command::show_uint16, &params._aggregationInterval},
    {"Upload         ", "ul=", Command::set_uint16, Command::show_uint16, &params._uploadInterval},
    {"Sync RTC       ", "rtc=", Command::set_uint32, Command::show_uint32, &params._rtcSyncInterval},
    {"APN            ", "apn=", Command::set_string, Command::show_string, params._apn, sizeof(params._apn)},
    {"APN user       ", "apnusr=", Command::set_string, Command::show_string, params._apnusr, sizeof(params._apnusr)},
    {"APN password   ", "apnpw=", Command::set_string, Command::show_string, params._apnpw, sizeof(params._apnpw)},
    {"SPUL server    ", "srv=", Command::set_string, Command::show_string, params._spulsrv, sizeof(params._spulsrv)},
    {"Timestamp port ", "tport=", Command::set_uint16, Command::show_uint16, &params._timeport},
    {"SPUL port      ", "sport=", Command::set_uint16, Command::show_uint16, &params._spulport}
};

void ConfigParams::showSettings(Stream* stream)
{
    stream->println();
    stream->println("Settings (terms in minutes):");
    for (size_t i = 0; i < sizeof(args) / sizeof(args[0]); ++i) {
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
bool ConfigParams::execCommand(const char* line)
{
    bool done = Command::execCommand(args, sizeof(args) / sizeof(args[0]), line);
    if (done) {
        needCommit = true;
    }

    return done;
}

/*
 * Check if all required config parameters are filled in
 */
bool ConfigParams::checkConfig()
{
    // The APN cannot be empty
    if (_apn[0] == 0xFF || _apn[0] == '\0') {
        return false;
    }

    // The SPUL server cannot be empty
    if (_spulsrv[0] == 0xFF || _spulsrv[0] == '\0') {
        return false;
    }

    return true;
}

void ConfigParams::dump()
{
#ifdef DEBUG_STREAM
    showSettings(&DEBUG_STREAM);
#endif
}

