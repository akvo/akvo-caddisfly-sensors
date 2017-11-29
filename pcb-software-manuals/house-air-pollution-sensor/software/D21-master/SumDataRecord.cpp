#include "SumDataRecord.h"

#define fieldPrintWithSeparator(streamVar, separatorVar, val) { streamVar->print(val, DEC); streamVar->print(separatorVar); }
#define fieldPrintWithSeparatorHEX(streamVar, separatorVar, val) { streamVar->print(val, HEX); streamVar->print(separatorVar); }

// setup the field sizes
static const uint8_t SumDataFieldSizes[] = { SUM_DATA_FIELD_SIZES };
const uint8_t* SumDataRecord::fieldSizes = SumDataFieldSizes;

bool SumDataRecord::isValid() const
{
    return (getStartTimestamp() != 0xFFFFFFFF);
}

void SumDataRecord::printRecordLn(Stream* stream, const char* separator) const
{
    if (stream) {
        fieldPrintWithSeparator(stream, separator, getStartTimestamp());
        
        uint8_t devId[8];
        getDeviceId(devId);
        for (uint8_t i = 0; i < 7; i++) {
            fieldPrintWithSeparatorHEX(stream, ":", devId[i]);
        }
        fieldPrintWithSeparatorHEX(stream, separator, devId[7]);

        fieldPrintWithSeparator(stream, separator, getDuration());
        fieldPrintWithSeparator(stream, separator, getMaxTemperature());
        fieldPrintWithSeparator(stream, "\n", getBatteryLevel());
    }
}

void SumDataRecord::init()
{
    // zero everything
    for (uint16_t i = 0; i < getSize(); i++) {
        buffer[i] = 0;
    }

    // set default value for StartTimestamp (used for validity check)
    setStartTimestamp(0xFFFFFFFF);
}

uint8_t SumDataRecord::getFieldSize(uint8_t fieldIndex) const
{
    if (fieldIndex > getFieldCount()) {
        // TODO sanity check fail
        return 0;
    }

    return fieldSizes[fieldIndex];
}
