#include "SumNoIdDataRecord.h"

#define fieldPrintWithSeparator(streamVar, separatorVar, val) { streamVar->print(val, DEC); streamVar->print(separatorVar); }
#define fieldPrintWithSeparatorHEX(streamVar, separatorVar, val) { streamVar->print(val, HEX); streamVar->print(separatorVar); }

// setup the field sizes
static const uint8_t SumNoIdDataFieldSizes[] = { SUM_NO_ID_DATA_FIELD_SIZES };
const uint8_t* SumNoIdDataRecord::fieldSizes = SumNoIdDataFieldSizes;

bool SumNoIdDataRecord::isValid() const
{
    return (getStartTimestamp() != 0xFFFFFFFF);
}

void SumNoIdDataRecord::printRecordLn(Stream* stream, const char* separator) const
{
    if (stream) {
        fieldPrintWithSeparator(stream, separator, getStartTimestamp());
        fieldPrintWithSeparator(stream, separator, getDuration());
        fieldPrintWithSeparator(stream, separator, getMaxTemperature());
        fieldPrintWithSeparator(stream, "\n", getBatteryLevel());
    }
}

void SumNoIdDataRecord::init()
{
    // zero everything
    for (uint16_t i = 0; i < getSize(); i++) {
        buffer[i] = 0;
    }

    // set default value for StartTimestamp (used for validity check)
    setStartTimestamp(0xFFFFFFFF);
}

uint8_t SumNoIdDataRecord::getFieldSize(uint8_t fieldIndex) const
{
    if (fieldIndex > getFieldCount()) {
        // TODO sanity check fail
        return 0;
    }

    return fieldSizes[fieldIndex];
}
