/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * The data record contains all the items that we want to record
 */

#ifndef _DATARECORD_H_
#define _DATARECORD_H_

#include <Arduino.h>
#include <stdint.h>

#define DATA_RECORD_FIELD_NAMES         {\
    "ID",\
    "Temp",\
    "OutSidePressure",\
    "WAPPressure",\
    "WAPTemperature",\
    "LipoBattery",\
    }
#define RECORD_SEPARATOR                ","

struct DataRecord_t
{
    // Device ID
    char devid[30];

    //BME280 - TPH2
    int16_t Temp;
    uint16_t OutSidePressure;

    //WAP sensor
    int16_t WAPPressure;
    int16_t WAPTemperature;

    uint16_t lipoBattery;

    bool isValid();
    void init();

    static void getHeaderString(String& header, const char* separator = RECORD_SEPARATOR);
    void getRecordString(String& record, const char* separator = RECORD_SEPARATOR);
    void addRecordString(String& record, const char* separator = RECORD_SEPARATOR);
    void addLatLonString(String& record, const char* separator = RECORD_SEPARATOR);

    static void printHeaderString(Stream & stream, const char* separator = RECORD_SEPARATOR);
    void printRecordString(Stream & stream, const char* separator = RECORD_SEPARATOR);
};
typedef struct DataRecord_t DataRecord;

#endif /* _DATARECORD_H_ */
