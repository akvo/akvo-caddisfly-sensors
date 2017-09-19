/*
 * RadioMessageBuffer.h
 *
 * Created: 2/29/2016 10:25:12 PM
 *  Author: alex
 */ 

/*
 * Ring buffer used as message queue for cooking event radio messages.
*/

#ifndef RADIOMESSAGEBUFFER_H_
#define RADIOMESSAGEBUFFER_H_

#include <stdbool.h>
#include <stdint.h>
#include "RadioMessage.h"

#define RADIOMESSAGE_BUFFER_SIZE 16

void radioMessageBuffer_init(void);
bool radioMessageBuffer_isEmpty(void);
void radioMessageBuffer_add(const RadioMessage_t* value);
bool radioMessageBuffer_getCurrent(RadioMessage_t* returnValue);
void radioMessageBuffer_next(void);

#endif /* RADIOMESSAGEBUFFER_H_ */
