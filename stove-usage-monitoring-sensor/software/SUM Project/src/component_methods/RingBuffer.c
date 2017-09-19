/*
* RingBuffer.c
*
* Created: 2/28/2016 12:01:45 PM
*  Author: alex
*/

#include "RingBuffer.h"

void ringBuffer_init(RingBuffer_t* ringBuffer)
{
    ringBuffer->_validItemCount = 0;
    ringBuffer->_start = 0;
    ringBuffer->_end = 0;
    ringBuffer->_size = sizeof(ringBuffer->_data)/sizeof(ringBuffer->_data[0]);

    for (uint16_t i = 0; i < ringBuffer->_size; i++) {
        ringBuffer->_data[i] = 0;
    }
}

bool ringBuffer_isEmpty(RingBuffer_t* ringBuffer)
{
    return (ringBuffer->_validItemCount == 0);
}

bool ringBuffer_isFull(RingBuffer_t* ringBuffer)
{
    return (ringBuffer->_validItemCount == ringBuffer->_size);
}

/*
* Adds a new value.
* If the buffer is full, it overwrites the first value.
*/
void ringBuffer_add(RingBuffer_t* ringBuffer, int16_t value)
{
    ringBuffer->_data[ringBuffer->_end] = value;
    ringBuffer->_end = (ringBuffer->_end + 1) % ringBuffer->_size;
    
    ringBuffer->_validItemCount++;
    
    // check for roll over
    if (ringBuffer->_validItemCount > ringBuffer->_size) {
        ringBuffer->_validItemCount = ringBuffer->_size;
        
        ringBuffer->_start = (ringBuffer->_start + 1) % ringBuffer->_size; // push to next item
    }
}

bool ringBuffer_get(RingBuffer_t* ringBuffer, int16_t* returnValue)
{
    if (ringBuffer_isEmpty(ringBuffer)) {
        return false;
    }
    
    *returnValue = ringBuffer->_data[ringBuffer->_start];
    ringBuffer->_start = (ringBuffer->_start + 1) % ringBuffer->_size;
    ringBuffer->_validItemCount--;
    
    return true;
}

/*
* Gets the value from the given offset (start + offset). Returns false if
* there is no such element.
*/
bool ringBuffer_peek(RingBuffer_t* ringBuffer, uint16_t offset, int16_t* returnValue)
{
    if (ringBuffer_isEmpty(ringBuffer) || offset > ringBuffer->_validItemCount) {
        return false;
    }
    
    *returnValue = ringBuffer->_data[(ringBuffer->_start + offset) % ringBuffer->_size];
    return true;
}

uint16_t ringBuffer_getValidItemCount(RingBuffer_t* ringBuffer)
{
    return ringBuffer->_validItemCount;
}
