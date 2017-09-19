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
#include "Sodaq_segmented_df.h"

#define CONFIG_DF_PAGE 0 // relative page in segment
#define CONFIG_DF_ADDRESS 0 // relative address in page

class ConfigParams
{
public:
    uint16_t _sm; // sample interval
    uint16_t _aggregationInterval; // aggregate interval
    uint16_t _uploadInterval; // upload interval
    uint32_t _rtcSyncInterval; // RTC sync interval
    char _apn[25];
    char _apnusr[25];
    char _apnpw[25];
    char _spulsrv[25];
    uint16_t _timeport;
    uint16_t _spulport;

public:
    void read(DFlashSegment& configSeg);
    void commit(DFlashSegment& configSeg, bool forced = false);
    void reset();

    bool execCommand(const char* line);

    uint16_t getSampleIntvl() const { return _sm; }
    uint16_t getAggregationInterval() const { return _aggregationInterval; }
    uint16_t getUploadInterval() const { return _uploadInterval; }
    uint32_t getRtcSyncInterval() const { return _rtcSyncInterval; }
    const char* getAPN() const { return _apn; }
    const char* getAPNuser() const { return _apnusr; }
    const char* getAPNpassword() const { return _apnpw; }
    const char* getSPULserver() const { return _spulsrv; }
    uint16_t getTimePort() const { return _timeport; }
    uint16_t getSPULport() const { return _spulport; }

    static void showSettings(Stream* stream);
    bool checkConfig();
    void dump();
};

extern ConfigParams params;

#endif /* CONFIG_H_ */

