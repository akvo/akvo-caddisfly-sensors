/*
 * RadioMessageBuffer.c
 *
 * Created: 2/29/2016 10:24:51 PM
 *  Author: alex
 */ 

#include "RadioMessageBuffer.h"

static RadioMessage_t _data[RADIOMESSAGE_BUFFER_SIZE];

static uint16_t _start;
static uint16_t _end;
static uint16_t _validItemCount;


void radioMessageBuffer_init(void)
{
    _validItemCount = 0;
    _start = 0;
    _end = 0;

    for (uint16_t i = 0; i < RADIOMESSAGE_BUFFER_SIZE; i++) {
        radioMessage_init(&_data[i]);
    }
}

bool radioMessageBuffer_isEmpty(void)
{
    return (_validItemCount == 0);
}

void radioMessageBuffer_add(const RadioMessage_t* value)
{
    _data[_end] = *value;
    _end = (_end + 1) % RADIOMESSAGE_BUFFER_SIZE;
        
    _validItemCount++;
        
    // check for roll over
    if (_validItemCount > RADIOMESSAGE_BUFFER_SIZE) {
        _validItemCount = RADIOMESSAGE_BUFFER_SIZE;
            
        _start = (_start + 1) % RADIOMESSAGE_BUFFER_SIZE; // push to next item
    }
}

bool radioMessageBuffer_getCurrent(RadioMessage_t* returnValue)
{
    if (radioMessageBuffer_isEmpty()) {
        return false;
    }
        
    *returnValue = _data[_start];
        
    return true;
}

void radioMessageBuffer_next(void)
{
    _start = (_start + 1) % RADIOMESSAGE_BUFFER_SIZE;
    _validItemCount--;
}