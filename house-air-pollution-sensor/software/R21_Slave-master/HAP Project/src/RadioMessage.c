/*
 * RadioMessage.c
 *
 * Created: 2/29/2016 2:40:18 PM
 *  Author: alex
 */ 

#include "RadioMessage.h"
#include <string.h>

void radioMessage_init(RadioMessage_t* record)
{
    record->Timestamp = 0;
    record->MaxTemperature = 0;
    record->Duration = 0;
    for (uint8_t i = 0; i < sizeof(record->DeviceId); i++) {
        record->DeviceId[i] = 0;
    }
    record->BatteryLevel = 0;
}

uint8_t radioMessage_to_buffer(const RadioMessage_t* record, uint8_t* buffer)
{
    uint8_t index = 0;
    uint8_t size = sizeof(record->Timestamp);
    memcpy(&buffer[index], &record->Timestamp, size);
    
    index += size;
    size = sizeof(record->DeviceId);
    memcpy(&buffer[index], record->DeviceId, size);

    index += size;
    size = sizeof(record->MaxTemperature);
    memcpy(&buffer[index], &record->MaxTemperature, size);

    index += size;
    size = sizeof(record->Duration);
    memcpy(&buffer[index], &record->Duration, size);

    index += size;
    size = sizeof(record->BatteryLevel);
    memcpy(&buffer[index], &record->BatteryLevel, size);
    
    index += size;

    return index; // return amount written to the buffer
}

void buffer_to_radioMessage(const uint8_t* buffer, RadioMessage_t* record)
{
    uint8_t index = 0;
    uint8_t size = sizeof(record->Timestamp);
    memcpy(&record->Timestamp, &buffer[index], size);

    index += size;
    size = sizeof(record->DeviceId);
    memcpy(record->DeviceId, &buffer[index], size);

    index += size;
    size = sizeof(record->MaxTemperature);
    memcpy(&record->MaxTemperature, &buffer[index], size);

    index += size;
    size = sizeof(record->Duration);
    memcpy(&record->Duration, &buffer[index], size);

    index += size;
    size = sizeof(record->BatteryLevel);
    memcpy(&record->BatteryLevel, &buffer[index], size);
}
