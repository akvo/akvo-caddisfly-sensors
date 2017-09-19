/*
 * \file MyMQTT.h
 *
 * This module forms the layer between the application (sketch) and the MQTT library
 */

#ifndef _MYMQTT_H
#define _MYMQTT_H

#include <WString.h>
#include <Stream.h>
#include "Sodaq_GSM_Modem.h"

void setupMQTT(Sodaq_GSM_Modem& modem, const String & clientId);
bool publishMQTT(const String & topic, const String & msg, bool do_close = true);
bool publishMQTT(const char * topic, const char * msg, bool do_close = true);
bool subscribeMQTT(const String & topic);
bool subscribeMQTT(const char * topic);
void testMQTT();

#endif // _MYMQTT_H
