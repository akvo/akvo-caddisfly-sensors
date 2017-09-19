/*
 * RadioMessage.h
 *
 * Created: 2/29/2016 2:12:00 PM
 *  Author: alex
 */ 


#ifndef RADIOMESSAGE_H_
#define RADIOMESSAGE_H_

#include <stdint.h>

typedef enum RadioMessageType_t {
    CookingEventMessage,
    StatusReportMessage
} RadioMessageType_t ;

typedef struct RadioMessage_t {
    uint32_t Timestamp;
    uint8_t DeviceId[8];
    uint16_t MaxTemperature;
    uint16_t Duration;
    uint16_t BatteryLevel;
} RadioMessage_t;

void radioMessage_init(RadioMessage_t* record);
uint8_t radioMessage_to_buffer(const RadioMessage_t* record, uint8_t* buffer);
void buffer_to_radioMessage(const uint8_t* buffer, RadioMessage_t* record);

#endif /* RADIOMESSAGE_H_ */