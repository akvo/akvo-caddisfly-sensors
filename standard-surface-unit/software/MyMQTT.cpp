/*
 * \file MyMQTT.cpp
 *
 * This module forms the layer between the application (sketch) and the MQTT library
 */

#include <WString.h>
#include <Stream.h>

#include "Config.h"
#include "DiagStream.h"
#include "Sodaq_GSM_Modem.h"
#include "Sodaq_MQTT.h"
#include "Sodaq_wdt.h"

#include "MyMQTT.h"

#define ENABLE_MQTT_DIAG             1

#define DEBUG_PREFIX String("[MQTT] ")

#if ENABLE_MQTT_DIAG
  #define myDiagPrint(...) diagPrint(__VA_ARGS__)
  #define myDiagPrintLn(...) diagPrintLn(__VA_ARGS__)
#else
  #define myDiagPrint(...)
  #define myDiagPrintLn(...)
#endif

/*!
 * Setup the MQTT client
 *
 * Notice that we are totally relying on the fact that params
 * is statically allocated. Its members are used to pass down
 * to the MQTT library.
 */
void setupMQTT(Sodaq_GSM_Modem& modem, const String & clientId)
{
    // Set the MQTT server hostname, and the port number
    mqtt.setServer(params.getMQTTServer(), params.getMQTTPort());

    // OPTIONAL. Set the user name and password
    mqtt.setAuth(params.getMQTTuser(), params.getMQTTpassword());

    // Set the MQTT client ID
    mqtt.setClientId(clientId.c_str());

    mqtt.setTransport(&modem);
}

bool publishMQTT(const String & topic, const String & msg, bool do_close)
{
    return publishMQTT(topic.c_str(), msg.c_str(), do_close);
}

bool publishMQTT(const char * topic, const char * msg, bool do_close)
{
    bool retval = mqtt.publish(topic, msg);
    if (do_close) {
        mqtt.close();
    }
    return retval;
}

bool subscribeMQTT(const String & topic)
{
    return subscribeMQTT(topic.c_str());
}

bool subscribeMQTT(const char * topic)
{
    bool retval = mqtt.subscribe(topic);
    // It never makes sense to close the connection
    return retval;
}

////////////////////////////////////////////////////////////////////////////////
////////////////   TESTING SODAQ_MQTT  /////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

static int counter;
static void testMQTT_loop()
{
    const char * topic = "SODAQ/demo/text";
    String msg = "Our message, number " + String(counter);
    // PUBLISH something
    if (!mqtt.publish(topic, msg.c_str())) {
      Serial.println("publish failed");
      while (true) {}
    }

    ++counter;
    if (counter >= 2) {
      mqtt.close();
      // End of the demo. Wait here for ever
      sodaq_wdt_disable();
      while (true) {}
    }
}

void testMQTT()
{
    testMQTT_loop();
    sodaq_wdt_reset();
    testMQTT_loop();
}
