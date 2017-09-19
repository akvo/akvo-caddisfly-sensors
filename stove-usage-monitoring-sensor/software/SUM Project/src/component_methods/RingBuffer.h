/*
* RingBuffer.h
*
* Created: 2/28/2016 6:10:00 PM
*  Author: alex
*/

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

#include <stdbool.h>
#include <stdint.h>

#define RING_BUFFER_SIZE 5

typedef struct RingBuffer_t
{
    int16_t _data[RING_BUFFER_SIZE];

    uint16_t _start;
    uint16_t _end;
    uint16_t _size;
    uint16_t _validItemCount;
    
} RingBuffer_t;

void ringBuffer_init(RingBuffer_t* ringBuffer);
bool ringBuffer_isEmpty(RingBuffer_t* ringBuffer);
bool ringBuffer_isFull(RingBuffer_t* ringBuffer);
void ringBuffer_add(RingBuffer_t* ringBuffer, int16_t value);
bool ringBuffer_get(RingBuffer_t* ringBuffer, int16_t* returnValue);
bool ringBuffer_peek(RingBuffer_t* ringBuffer, uint16_t offset, int16_t* returnValue);
uint16_t ringBuffer_getValidItemCount(RingBuffer_t* ringBuffer);

#endif /* RINGBUFFER_H_ */