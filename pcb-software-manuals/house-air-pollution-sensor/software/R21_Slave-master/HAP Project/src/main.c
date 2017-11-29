#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "sys.h"
#include "system.h"
#include "phy.h"
#include "nwk.h"
#include "sysTimer.h"
#include "asf.h"
#include "rtc_methods.h"
#include "r21_slave.h"
#include "RadioMessage.h"

#ifdef NWK_ENABLE_SECURITY
#define APP_BUFFER_SIZE (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)
#else
#define APP_BUFFER_SIZE NWK_MAX_PAYLOAD_SIZE
#endif

typedef enum AppState_t {
    APP_STATE_INITIAL,
    APP_STATE_IDLE,
} AppState_t;

void configure_rtc_calendar(void);
void set_rtc_epoch(uint32_t epoch);
uint32_t get_rtc_epoch(void);

static AppState_t appState = APP_STATE_INITIAL;
static NWK_DataReq_t appDataReq;
static bool appDataReqBusy = false;
static uint8_t sendBuffer[APP_BUFFER_SIZE]; // only used when timestamp is sent

struct rtc_module rtc_instance;

// when booting up the master hasn't initialized communication (or the RTC) yet and
// thus communication with the SUM is disabled until at least the RTC is set
// (set_rtc_epoch() is only called by the R21_slave lib)
// That way, it is guaranteed that the SUM will get the correct time on boot-up.
bool isInitializedByMaster = false;

static void appDataConf(NWK_DataReq_t *req)
{
    appDataReqBusy = false;
    (void)req;
}

static bool appDataInd(NWK_DataInd_t *ind)
{
    if (!isInitializedByMaster) {
        return true;
    }
    
    RadioMessage_t radioMessage;
    radioMessage_init(&radioMessage);

    buffer_to_radioMessage(ind->data, &radioMessage);

    // note: status reports have maxTemp and duration equal to 0,
    // but they are otherwise handled the same.

    r21_record_t record;
    record.Timestamp = radioMessage.Timestamp;
    memcpy(record.DeviceId, radioMessage.DeviceId, sizeof(record.DeviceId));
    record.MaxTemperature = radioMessage.MaxTemperature;
    record.Duration = radioMessage.Duration;
    record.BatteryLevel = radioMessage.BatteryLevel;

    r21_slave_add_record(&record);
    
    uint32_t now = get_rtc_epoch();

    memcpy(sendBuffer, &now, sizeof(now)); // sizeof(sendbuffer) > sizeof(now)
    
    appDataReq.dstAddr = ind->srcAddr;
    appDataReq.dstEndpoint = APP_ENDPOINT;
    appDataReq.srcEndpoint = APP_ENDPOINT;
    appDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
    appDataReq.data = sendBuffer;
    appDataReq.size = sizeof(now);
    appDataReq.confirm = appDataConf;
    NWK_DataReq(&appDataReq);

    appDataReqBusy = true;
    
    return true;
}

static void appInit(void)
{
    NWK_SetAddr(APP_ADDR);
    NWK_SetPanId(APP_PANID);
    PHY_SetChannel(APP_CHANNEL);
    PHY_SetRxState(true);

    NWK_OpenEndpoint(APP_ENDPOINT, appDataInd);
}

static void APP_TaskHandler(void)
{
    switch (appState) {
        case APP_STATE_INITIAL:
        {
            appInit();
            appState = APP_STATE_IDLE;
        }
        break;

        case APP_STATE_IDLE:
        break;

        default:
        break;
    }
}

void configure_rtc_calendar(void)
{
    struct rtc_calendar_config config_rtc_calendar;
    rtc_calendar_get_config_defaults(&config_rtc_calendar);
    
    config_rtc_calendar.clock_24h = true;

    rtc_calendar_init(&rtc_instance, RTC, &config_rtc_calendar);
    rtc_calendar_enable(&rtc_instance);
}

void set_rtc_epoch(uint32_t epoch)
{
    struct rtc_calendar_time time;
    rtc_setEpoch(&time, epoch);
    rtc_calendar_set_time(&rtc_instance, &time);
    
    isInitializedByMaster = true; // the time is set by the master, it is safe to talk to the SUM now
}

uint32_t get_rtc_epoch(void)
{
    struct rtc_calendar_time time;
    rtc_calendar_get_time(&rtc_instance, &time);
    return rtc_getEpoch(time, rtc_instance.clock_24h);
}

int main(void)
{
    irq_initialize_vectors();
    system_init();
    delay_init();
    SYS_Init();

    configure_rtc_calendar();
    r21_slave_init();
    r21_slave_set_rtc_callbacks(get_rtc_epoch, set_rtc_epoch);
    cpu_irq_enable();
    
    // when this counter rolls over, r21_slave_update_out_record_buffer_if_needed() is called
    uint8_t buffer_update_counter = 0;

    while (1) {
        SYS_TaskHandler();
        APP_TaskHandler();
        r21_slave_task();

        if (buffer_update_counter == 0) {
            r21_slave_update_out_record_buffer_if_needed();
        }

        buffer_update_counter++;
    }
}
