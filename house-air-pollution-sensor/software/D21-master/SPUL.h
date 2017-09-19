/*
* SPUL.h
*
* Created: 13/12/2015
*  Author: Gabriel Notman
*/

#ifndef SPUL_H_
#define SPUL_H_

#include "Sodaq_GSM_Modem.h"

struct SpulFrame_t
{
    /* header */
    uint8_t deviceID[8];
    uint8_t recordCount;
    uint8_t recordLength;
    int8_t signalQuality;
    uint8_t minSigQualTime;

    /* payload */
    uint8_t payload[500];       // Note. This struct member determines the header size!!
} __attribute__((packed));

typedef struct SpulFrame_t SpulFrame_t;

void setupSPULModem(Sodaq_GSM_Modem& modem);

void setSPULNetworkQuality(SpulFrame_t& frame);

uint32_t getSPULTimeStamp(const char* apn, const char* apnUser,
    const char* apnPass, const char* host, uint16_t port,
    bool switchOff = true);

int8_t openSPULSocket(const char* apn, const char* apnUser,
    const char* apnPass, const char* host, uint16_t port);

bool sendSPULFrame(int8_t socket, const SpulFrame_t& frame, size_t payloadLen);

void closeSPULSocket(int8_t socket = 0);

#endif /* SPUL_H_ */