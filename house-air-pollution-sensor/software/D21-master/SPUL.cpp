/*
* SPUL.cpp
*
* Created: 13/12/2015
*  Author: Gabriel Notman
*/

#include <stdint.h>
#include "Sodaq_GSM_Modem.h"

#include "SPUL.h"

static Sodaq_GSM_Modem * myModem;
void setupSPULModem(Sodaq_GSM_Modem& modem)
{
    myModem = &modem;
}

void setSPULNetworkQuality(SpulFrame_t& frame)
{
    frame.signalQuality = myModem->getLastRSSI();
    frame.minSigQualTime = myModem->getCSQtime();
}

uint32_t getSPULTimeStamp(const char* apn, const char* apnUser,
    const char* apnPass, const char* host, uint16_t port,
    bool switchOff)
{
    uint32_t result = 0;

    if (myModem->openTCP(apn, apnUser, apnPass, host, port)) {
        uint8_t buff[4];
        if (myModem->receiveDataTCP(buff, sizeof(buff))) {
            result = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | (buff[3]);
        }
        myModem->closeTCP(switchOff);
    }

    return result;
}

int8_t openSPULSocket(const char* apn, const char* apnUser,
    const char* apnPass, const char* host, uint16_t port)
{
    int8_t socket = -1;

    if (myModem->openTCP(apn, apnUser, apnPass, host, port)) {
        socket = 0;
    }

    return socket;
}

bool sendSPULFrame(int8_t socket, const SpulFrame_t& frame, size_t payloadLen)
{
    // Socket parameter is unused
    (void)socket;

    return myModem->sendDataTCP((uint8_t*)&frame, payloadLen + offsetof(SpulFrame_t, payload));
}

void closeSPULSocket(int8_t socket)
{
    // Socket parameter is unused
    (void)socket;

    myModem->closeTCP();
}
