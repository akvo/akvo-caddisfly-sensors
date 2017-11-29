#include "SensorDataRecord.h"

#define fieldPrintWithSeparator(streamVar, separatorVar, val) { streamVar->print(val, DEC); streamVar->print(separatorVar); }

// setup the field sizes
static const uint8_t SensorDataFieldSizes[] = { SENSOR_DATA_FIELD_SIZES };
const uint8_t* SensorDataRecord::fieldSizes = SensorDataFieldSizes;

bool SensorDataRecord::isValid() const
{
    return (getTimestamp() != 0xFFFFFFFF);
}

void SensorDataRecord::printRecordLn(Stream* stream, const char* separator) const
{
    if (stream) {
        fieldPrintWithSeparator(stream, separator, getTimestamp());
        fieldPrintWithSeparator(stream, separator, getCO());
        fieldPrintWithSeparator(stream, separator, getCO2());
        fieldPrintWithSeparator(stream, separator, getP1());
        fieldPrintWithSeparator(stream, separator, getP2());
        fieldPrintWithSeparator(stream, "\n", getBatteryLevel());
    }
}

void SensorDataRecord::init()
{
    // zero everything
    for (uint16_t i = 0; i < getSize(); i++) {
        buffer[i] = 0;
    }
    
    // set default value for Timestamp (used for validity check)
    setTimestamp(0xFFFFFFFF);
}

uint8_t SensorDataRecord::getFieldSize(uint8_t fieldIndex) const
{
    if (fieldIndex > getFieldCount()) {
        // TODO sanity check fail
        return 0;
    }

    return fieldSizes[fieldIndex];
}
