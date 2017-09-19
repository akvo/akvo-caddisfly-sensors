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

#include <stdint.h>
#include "Command.h"
#include "Config.h"
#include "DiagStream.h"
#include "FlashStorage.h"
#include "Settings.h"
#include "StartupCommands.h"
#include "Utils.h"

// #define ENABLE_CONFIG_DIAG             1

#define DEBUG_PREFIX String("[CONF] ")

#if ENABLE_CONFIG_DIAG
  #define myDiagPrint(...) do { if (diag_stream.isEnabled()) diag_stream.print(__VA_ARGS__); } while(0)
  #define myDiagPrintLn(...) do { if (diag_stream.isEnabled()) diag_stream.println(__VA_ARGS__); } while(0)
#else
  #define myDiagPrint(...)
  #define myDiagPrintLn(...)
#endif

/*
 * Some default values of config parameters
 */
static const char stationName_Default[] = "SSU_WAP";
static const char mqttServer_Default[] = "vps01.m2m4all.com";
static const uint16_t mqttPort_Default = 1883;
static const char mqttUser_Default[] = "itay";
static const char mqttPass_Default[] = "sodaq";
static const char mqttTopic_Default[] = "ITAY/SSU";
static const char magic[] = "SODAQ";

ConfigParams params;
static bool needCommit;
FlashStorage(my_flash_store, ConfigParams);

/*
 * Read all of the config parameters from the EEPROM
 *
 * There are some sanity checks. If they fail then
 * it will call reset() instead.
 */
void ConfigParams::read()
{
  uint16_t crc;
  uint16_t crc1;

  *this = my_flash_store.read();
#if ENABLE_CONFIG_DIAG
  //dumpBuffer(DEBUG_PREFIX, *this, sizeof(*this));
#endif

  if (strcmp(magic, _magic) != 0) {
      goto do_reset;
  }

  crc = _crc;
  crc1 = computeCrc();

  myDiagPrint(DEBUG_PREFIX + "crc="); myDiagPrintLn(crc);
  myDiagPrint(DEBUG_PREFIX + "crc1="); myDiagPrintLn(crc1);
  if (crc != crc1) {
    goto do_reset;
  }
  return;

do_reset:
  reset();
}

/*
 * Reset all config parameters to their default
 *
 * Some of the defaults are defined in Settings.h
 * Other defaults can be found at the top of this file.
 */
void ConfigParams::reset()
{
  myDiagPrintLn(DEBUG_PREFIX + "ConfigParms::reset");
  memset(this, 0, sizeof(*this));
  _sm = PARAM_Sm;

#if 1
  strncpy(_stationName, stationName_Default, sizeof(_stationName) - 1);
#endif

  strncpy(_apn, DEFAULT_APN, sizeof(_apn) - 1);
  //strncpy(_apnusr, DEFAULT_APNUSR, sizeof(_apnusr) - 1);
  //strncpy(_apnpw, DEFAULT_APNPW, sizeof(_apnpw) - 1);

  strncpy(_mqttsrv, mqttServer_Default, sizeof(_mqttsrv) - 1);
  _mqttport = mqttPort_Default;
  strncpy(_mqttuser, mqttUser_Default, sizeof(_mqttuser) - 1);
  strncpy(_mqttpass, mqttPass_Default, sizeof(_mqttpass) - 1);
  strncpy(_mqtttopic, mqttTopic_Default, sizeof(_mqtttopic) - 1);

  _enable_diag = true;
  _keep_usb = true;

  strncpy(_magic, magic, sizeof(_magic) - 1);

  needCommit = true;
}

/*
 * Write the configuration parameters to NVM / Dataflash
 */
void ConfigParams::commit(bool forced)
{
  if (_stationName[0] == '\0') {
    // It was probably not initialized.
    return;
  }
  if (!forced && !needCommit) {
    return;
  }

  myDiagPrintLn(DEBUG_PREFIX + "ConfigParams::commit");
  // Fill in the CRC, and write to EEPROM
  _crc = computeCrc();

  my_flash_store.write(*this);
  
  needCommit = false;
}

static const char txtCommands[] PROGMEM = "Configuration:";
static const Command cmds[] = {
    {"Sample         ", "sm=",      Command::set_uint16, Command::show_uint16,  &params._sm},
    {"station name   ", "nm=",      Command::set_string, Command::show_string,  params._stationName, sizeof(params._stationName)},

    {"APN            ", "apn=",     Command::set_string, Command::show_string,  params._apn, sizeof(params._apn)},
    {"APN user       ", "apnusr=",  Command::set_string, Command::show_string,  params._apnuser, sizeof(params._apnuser) },
    {"APN password   ", "apnpw=",   Command::set_string, Command::show_string,  params._apnpass, sizeof(params._apnpass) },

    {"MQTT server    ", "srv=",     Command::set_string, Command::show_string,  params._mqttsrv, sizeof(params._mqttsrv)},
    {"MQTT port      ", "port=",    Command::set_uint16, Command::show_uint16,  &params._mqttport},
    {"MQTT user      ", "mqusr=",   Command::set_string, Command::show_string,  params._mqttuser, sizeof(params._mqttuser)},
    {"MQTT password  ", "mqpw=",    Command::set_string, Command::show_string,  params._mqttpass, sizeof(params._mqttpass)},
    {"MQTT topic prefix", "mqtop=", Command::set_string, Command::show_string,  params._mqtttopic, sizeof(params._mqtttopic)},

    {"WAP press offset", "pres=",   Command::set_int16,  Command::show_int16,   &params._wap_press_offset},

    {"Enable Diag    ", "ed=",      Command::set_uint8,  Command::show_uint8,   &params._enable_diag},
    {"Keep USB on    ", "usb=",     Command::set_uint8,  Command::show_uint8,   &params._keep_usb},
};

static void setNeedCommit()
{
  needCommit = true;
}

void ConfigParams::addStartupCommands()
{
  supComRegisterCmnds(txtCommands,
      cmds, sizeof(cmds) / sizeof(cmds[0]),
      NULL,
      setNeedCommit
      );
}

void ConfigParams::showSettings(Stream * stream)
{
  if (!stream) {
    return;
  }
  stream->println();
  stream->println("Settings (terms in minutes):");
  for (size_t i = 0; i < sizeof(cmds) / sizeof(cmds[0]); ++i) {
    const Command *a = &cmds[i];
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
bool ConfigParams::execCommand(const char * line)
{
  bool done = Command::execCommand(cmds, sizeof(cmds) / sizeof(cmds[0]), line);
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
    if (_stationName[0] == '\0') {
        myDiagPrintLn(DEBUG_PREFIX + "ERROR: station name not set");
        // It was probably not initialized.
        return false;
    }
    if (_apn[0] == '\0') {
        myDiagPrintLn(DEBUG_PREFIX + "ERROR: apn not set");
        return false;
    }
    if (_mqttsrv[0] == '\0') {
        myDiagPrintLn(DEBUG_PREFIX + "ERROR: MQTT server not set");
        return false;
    }
    if (_mqttport == 0) {
        myDiagPrintLn(DEBUG_PREFIX + "ERROR: MQTT port not set");
        return false;
    }
    if (_mqtttopic[0] == '\0') {
        myDiagPrintLn(DEBUG_PREFIX + "ERROR: MQTT topic");
        return false;
    }

    return true;
}

void ConfigParams::dump(Stream * stream)
{
  showSettings(stream);
}

uint16_t ConfigParams::computeCrc()
{
    return crc16ccitt((uint8_t *)this, offsetof(ConfigParams, _crc));
}
