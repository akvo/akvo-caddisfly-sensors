/*
 * Based on: Config.h
 *
 *  Created on: Mar 27, 2014
 *      Author: Kees Bakker
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>
#include <Arduino.h>

class ConfigParams
{
public:
  uint16_t      _sm;            // sample interval
  char          _stationName[20];

  char          _apn[50];               // Is this enough for the APN?
  char          _apnuser[50];           // Is this enough for the APN user?
  char          _apnpass[50];           // Is this enough for the APN password?

  char          _mqttsrv[50];           // Is this enough for the server name?
  uint16_t      _mqttport;
  char          _mqttuser[50];
  char          _mqttpass[50];
  char          _mqtttopic[50];

  int16_t       _wap_press_offset;

  uint8_t       _enable_diag;
  uint8_t       _keep_usb;

  char          _magic[20];

  // Always keep this at the end
  uint16_t      _crc;

public:
  void read();
  void commit(bool forced=false);
  void reset();

  bool execCommand(const char * line);

  uint16_t getSampleIntvl() const { return _sm; }

  const char *getStationName() const { return _stationName; }

  const char *getAPN() const { return _apn; }
  const char *getAPNuser() const { return _apnuser; }
  const char *getAPNpassword() const { return _apnpass; }

  const char *getMQTTServer() const { return _mqttsrv; }
  uint16_t getMQTTPort() const { return _mqttport; }
  const char *getMQTTuser() const { return _mqttuser; }
  const char *getMQTTpassword() const { return _mqttpass; }
  const char *getMQTTtopic() const { return _mqtttopic; }

  int16_t getWapPressOffset() const { return _wap_press_offset; }

  bool getEnableDiag() const { return _enable_diag; }
  bool getKeepUSB() const { return _keep_usb; }

  static void addStartupCommands();
  static void showSettings(Stream * stream);
  bool checkConfig();
  void dump(Stream * stream);
private:
  uint16_t computeCrc();
};

extern ConfigParams params;

#endif /* CONFIG_H_ */
