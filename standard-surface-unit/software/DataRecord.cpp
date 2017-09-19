/*
 * Copyright 2016, M2M4ALL
 *
 * This is part of the SSU WAP.
 * The data record contains all the items that we want to record
 */

#include <Arduino.h>
#include "DataRecord.h"

void DataRecord_t::init()
{
    memset(this, 0, sizeof(*this));
}

void DataRecord_t::getHeaderString(String& header, const char* separator)
{
    const char * fields[] = DATA_RECORD_FIELD_NAMES;
    header.reserve(80);
    header = "";
    for (size_t ix = 0; ix < sizeof(fields) / sizeof(fields[0]); ++ix) {
        if (ix != 0) {
            header += separator;
        }
        header += fields[ix];
    }
}

static void addField(String & str, const String & fld, const char* separator)
{
    if (str.length() > 0) {
        str += separator;
    }
    str += fld;
}

void DataRecord_t::addRecordString(String& str, const char* separator)
{
    addField(str, String(devid), separator);

    addField(str, String(Temp), separator);
    addField(str, String(OutSidePressure), separator);

    addField(str, String(WAPPressure), separator);
    addField(str, String(WAPTemperature), separator);

    addField(str, String(lipoBattery), separator);
}

void DataRecord_t::getRecordString(String& str, const char* separator)
{
    str.reserve(100);
    str = "";

    addRecordString(str, separator);
}

void DataRecord_t::printHeaderString(Stream & stream, const char* separator)
{
    String header;
    getHeaderString(header);
    stream.println(header);
}

void DataRecord_t::printRecordString(Stream & stream, const char* separator)
{
    String str;
    getRecordString(str, separator);
    stream.println(str);
}
