#include <asf.h>
#include <sys.h>
#include <sysTimer.h>
#include <math.h>
#include <rtc_methods.h>
#include <battery_reading.h>
#include <max31855.h>
#include <inttypes.h>
#include <version.h>
#include "RingBuffer.h"
#include "RadioMessage.h"
#include "RadioMessageBuffer.h"

#define DEBUG

#ifdef NWK_ENABLE_SECURITY
#define APP_BUFFER_SIZE (NWK_MAX_PAYLOAD_SIZE - NWK_SECURITY_MIC_SIZE)
#else
#define APP_BUFFER_SIZE NWK_MAX_PAYLOAD_SIZE
#endif

#define TIMEOUT_TC_MODULE TC4

#define RTC_TOLERANCE 10
#define RESPONSE_TIMEOUT 5
#define STARTUP_DELAY 5 * 1000 // after init is done, wait for this amount of ms before starting app

// if this amount of wake-ups occur without a message, a status report will be sent. Based on a per-minute wake up basis.
#define MAX_ITERATIONS_BETWEEN_MESSAGES (uint16_t)(6 * 60) // every 6 hours

#define DEVICE_ID_FORMAT "%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
#define DEVICE_ID_PARTS(x) x[0],x[1],x[2],x[3],x[4],x[5],x[6],x[7]


typedef enum AppState_t {
    APP_STATE_INITIAL,
    APP_STATE_READING
} AppState_t;

typedef enum StoveState_t {
    StoveCooking,
    StoveIdle
} StoveState_t ;


static void appDataConf(NWK_DataReq_t *req);
static bool appDataInd(NWK_DataInd_t *ind);
static void appInit(void);
static void sendRadioMessage(const RadioMessage_t* message);
static void appMain(void);

static void configure_rtc_callbacks(void);
static void configure_rtc_calendar(void);

static void configure_tcs_callbacks(void);
static void configure_tcs(void);
static void tc_callback_timeout(struct tc_module *const module_inst);

uint32_t get_rtc_epoch(void);
static void appSleep(void);
static void createDeviceId(void);
static void readSensor(void);
static void printRingBuffer(RingBuffer_t* ringBuffer);
static int16_t getAverage(RingBuffer_t* ringBuffer);
static int16_t getAverageTemperature(void);
static int16_t getAverageGradient(void); // per minute
static void updateMaxTemperatureIfNeeded(int16_t temperature);
static int16_t getCurrentCookingStopTemperature(void);
static uint32_t getCurrentCookingDuration(void);
static void detectStoveState(int16_t averageTemperature, int16_t averageGradient);
static void setRadioMessage(RadioMessage_t* message, uint32_t timestamp, uint16_t duration, uint16_t maxTemp);
static void sendStatusReportRadioMessage(void);
static void sendNextPendingMessage(void);


static uint8_t deviceId[8] = { 0, };

static int16_t cookingStartTemperatureThreshold = 75;
static int16_t cookingStartGradientThreshold = 10;

static int16_t cookingStopTemperatureThresholdPercent = 36; // of the maximum temperature
static int16_t cookingStopGradientThreshold = 1;

static int16_t cookingMaxTemperature = 0; // it is being set when cooking start is detected anyway
static uint32_t cookingStartTimestamp = 0;
static uint32_t cookingEndTimestamp = 0;

static StoveState_t stoveState = StoveIdle;
static NWK_DataReq_t appDataReq;

static struct rtc_module rtc_instance;
static struct tc_module tc_instance_timeout;

static bool appDataReqBusy = false;
static bool waitingForAck = false;
static bool cookingEventMessagePendingConfirmation = false; // this is used to handle removing a pending cooking event message from the pending queue once it is sent successfully

static uint8_t sendBuffer[APP_BUFFER_SIZE];
static char displayBuffer[1024];

// counts the wake-ups since last message sent
static uint16_t noMessageSentCounter = MAX_ITERATIONS_BETWEEN_MESSAGES; // set it to max, to trigger a status report on startup

static RingBuffer_t temperatureBuffer;
static RingBuffer_t gradientBuffer; // per minute (per wake-up). Value for (temperatureBuffer[i-1], temperatureBuffer[i]) is in gradientBuffer[i]

static bool isAppInitialized = false;
static bool isRtcInitialized = false;

static void appDataConf(NWK_DataReq_t *req)
{
    appDataReqBusy = false;
    (void)req;
}

static bool appDataInd(NWK_DataInd_t *ind)
{
    tc_disable(&tc_instance_timeout);
    
    if (waitingForAck) {
        uint32_t ticks;
        memcpy(&ticks, ind->data, sizeof(ticks));
        
        sprintf(displayBuffer, "Received { %lu }\r\n", ticks);
        udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
        
        // get current time
        struct rtc_calendar_time time;
        rtc_calendar_get_time(&rtc_instance, &time);
        uint32_t old_ticks = rtc_getEpoch(time, rtc_instance.clock_24h);
        
        uint32_t tick_difference = abs(ticks - old_ticks);
        
        if (tick_difference > RTC_TOLERANCE) {
            rtc_setEpoch(&time, ticks);
            rtc_calendar_set_time(&rtc_instance, &time);
        }
        
        if (cookingEventMessagePendingConfirmation) {
            radioMessageBuffer_next();
        }
        
        noMessageSentCounter = 0;
        isRtcInitialized = true;
    }
    
    waitingForAck = false;
    cookingEventMessagePendingConfirmation = false;
    
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

static void setRadioMessage(RadioMessage_t* message, uint32_t timestamp, uint16_t duration, uint16_t maxTemp)
{
    radioMessage_init(message); // everything gets zeroed
    
    memcpy(message->DeviceId, deviceId, sizeof(message->DeviceId));
    message->BatteryLevel = read_battery_mVolts();

    message->Timestamp = timestamp;
    message->Duration = duration;
    message->MaxTemperature = maxTemp;
}

static void sendStatusReportRadioMessage(void)
{
    RadioMessage_t message;
    uint32_t timestamp = 0;
    
    if (isRtcInitialized) {
        timestamp = get_rtc_epoch();
    }
    
    // StatusReportMessage needs 0 in maxTemp and duration
    setRadioMessage(&message, timestamp, 0, 0);

    sendRadioMessage(&message);
}

static void sendNextPendingMessage(void)
{
    RadioMessage_t message;
    if (radioMessageBuffer_getCurrent(&message)) {
        cookingEventMessagePendingConfirmation = true; // this is not set to true in the sending method because it is only for cooking events
        sendRadioMessage(&message);
    }
}

static void sendRadioMessage(const RadioMessage_t* message)
{
    uint8_t bufferSize = radioMessage_to_buffer(message, sendBuffer);

    appDataReq.dstAddr = 0x0000;
    appDataReq.dstEndpoint = APP_ENDPOINT;
    appDataReq.srcEndpoint = APP_ENDPOINT;
    appDataReq.options = NWK_OPT_ACK_REQUEST | NWK_OPT_ENABLE_SECURITY;
    appDataReq.data = sendBuffer;
    appDataReq.size = bufferSize;
    appDataReq.confirm = appDataConf;
    NWK_DataReq(&appDataReq);
    
    #ifdef DEBUG
    sprintf(
        displayBuffer, 
        "Sending message {\r\n\tTimestamp: %lu,\r\n\tDeviceID: " DEVICE_ID_FORMAT ",\r\n\tMax Temp: %u,\r\n\tDuration: %u,\r\n\tBattery Level: %u\r\n}\r\n",
        message->Timestamp, 
        DEVICE_ID_PARTS(message->DeviceId), 
        message->MaxTemperature, 
        message->Duration, 
        message->BatteryLevel);
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    #endif
    
    waitingForAck = true;
    tc_enable(&tc_instance_timeout);
}

static void readSensor(void)
{
    int16_t thermocoupleTemp = 0;
    int16_t internalTemp = 0;
    max31855_read_values(&internalTemp, &thermocoupleTemp);
    
    ringBuffer_add(&temperatureBuffer, thermocoupleTemp);
    uint16_t validItemCount = ringBuffer_getValidItemCount(&temperatureBuffer);
    if (validItemCount >= 2) {
        // add gradient
        int16_t i = validItemCount - 1;
        int16_t x_1 = 0; // i
        int16_t x_0 = 0; // i-1
        ringBuffer_peek(&temperatureBuffer, i, &x_1); // guaranteed to succeed as i can be safely [0, validItemCount) and validItemCount >= 2
        ringBuffer_peek(&temperatureBuffer, i-1, &x_0);  // guaranteed to succeed as i-1 can be safely [0, validItemCount-1) and validItemCount >= 2
        
        ringBuffer_add(&gradientBuffer, x_1-x_0);
    }
}

/*
* Returns the average of the valid items of the given buffer.
*/
static int16_t getAverage(RingBuffer_t* ringBuffer)
{
    uint16_t validItemCount = ringBuffer_getValidItemCount(ringBuffer);
    
    if (validItemCount == 0) {
        return 0;
    }
    
    int32_t sum = 0;
    
    for (uint16_t i = 0; i < validItemCount; i++) {
        int16_t value = 0;
        ringBuffer_peek(ringBuffer, i, &value);
        sum += value;
    }
    
    return (sum / validItemCount);
}

/*
* Prints all the valid items of the given buffer.
*/
static void printRingBuffer(RingBuffer_t* ringBuffer)
{
    uint16_t validItemCount = ringBuffer_getValidItemCount(ringBuffer);
    
    if (validItemCount == 0) {
        sprintf(displayBuffer, "Empty Ring Buffer!");
        udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));

        return;
    }
    
    sprintf(displayBuffer, "[");
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    
    for (uint16_t i = 0; i < validItemCount; i++) {
        int16_t value = 0;
        ringBuffer_peek(ringBuffer, i, &value);

        sprintf(displayBuffer, " %" PRId16, value);
        udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    }

    sprintf(displayBuffer, " ]");
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
}

/*
* Returns the average of the valid items of the temperature buffer.
*/
static int16_t getAverageTemperature(void)
{
    return getAverage(&temperatureBuffer);
}

/*
* Returns the average of the valid items in the gradient buffer (that is, change of temperature per minute).
*/
static int16_t getAverageGradient(void)
{
    return getAverage(&gradientBuffer);
}

static void updateMaxTemperatureIfNeeded(int16_t temperature)
{
    if ((stoveState == StoveCooking) && (temperature > cookingMaxTemperature)) {
        cookingMaxTemperature = temperature;
        
        #ifdef DEBUG
        sprintf(
            displayBuffer, 
            "Max temperature = %u.\r\nCooking stops when temperature falls below %d and gradient below %d.\r\n", 
            cookingMaxTemperature, 
            getCurrentCookingStopTemperature(), 
            cookingStopGradientThreshold);
        udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
        #endif
    }
}

static int16_t getCurrentCookingStopTemperature(void)
{
    return round((float)cookingMaxTemperature * (float)cookingStopTemperatureThresholdPercent / 100.0);
}

static uint32_t getCurrentCookingDuration(void)
{
    if (stoveState == StoveCooking || cookingEndTimestamp == 0) {
        return 0;
    }
    
    return cookingEndTimestamp - cookingStartTimestamp;
}

static void detectStoveState(int16_t averageTemperature, int16_t averageGradient)
{
    // Transition from IDLE -> COOKING
    if ((stoveState == StoveIdle) 
            && (averageTemperature > cookingStartTemperatureThreshold) 
            && (averageGradient > cookingStartGradientThreshold)) {
        cookingMaxTemperature = averageTemperature;
        cookingStartTimestamp = get_rtc_epoch();
        cookingEndTimestamp = 0;
        
        stoveState = StoveCooking;

        #ifdef DEBUG
        sprintf(displayBuffer, "Detected start of cooking.\r\n");
        udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
        #endif
    }
    
    // Transition from COOKING -> IDLE
    else if ((stoveState == StoveCooking) 
                && (averageTemperature < getCurrentCookingStopTemperature())
                && (averageGradient < cookingStopGradientThreshold)) { // should it be abs(averageGradient) ?
        // stop cooking
        cookingEndTimestamp = get_rtc_epoch();
        stoveState = StoveIdle;

        #ifdef DEBUG
        sprintf(displayBuffer, "Detected end of cooking. Duration = %lu\r\n", cookingEndTimestamp-cookingStartTimestamp);
        udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
        #endif

        // Send cooking event message
        RadioMessage_t message;
        setRadioMessage(&message, cookingStartTimestamp, getCurrentCookingDuration(), cookingMaxTemperature);
        radioMessageBuffer_add(&message);
    }
}

static void appMain(void)
{
    // make sure everything is in order before allowing this to run (this is triggered by RTC alarm)
    if (!isAppInitialized) {
        return;
    }
    
    #ifdef DEBUG
    sprintf(displayBuffer, "Current time: %lu\r\n", get_rtc_epoch());
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    #endif
    
    // in case a message is sent (or rather, verified as successfully
    // sent by receiving a timestamp) this counter is re-set
    // otherwise, it triggers a status report after MAX_ITERATIONS_BETWEEN_MESSAGES
    noMessageSentCounter++;

    cookingEventMessagePendingConfirmation = false; // this should be false anyway, but make sure app wakes up cleanly
    
    // Take reading
    readSensor();

    #ifdef DEBUG
    if (!isRtcInitialized) {
        sprintf(displayBuffer, "RTC is NOT yet Initialized! Cooking Event Detection is disabled.\r\n");
        udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    }
    
    sprintf(displayBuffer, "Temperature: avg(");
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    printRingBuffer(&temperatureBuffer);
    sprintf(displayBuffer, ") = %d\r\n", getAverageTemperature());
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));

    sprintf(displayBuffer, "Gradient: avg(");
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    printRingBuffer(&gradientBuffer);
    sprintf(displayBuffer, ") = %d\r\n", getAverageGradient());
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    #endif

    // start processing only after there is a full ring buffer
    if (isRtcInitialized && ringBuffer_isFull(&temperatureBuffer)) {
        int16_t filteredTemperature = getAverageTemperature();
        
        updateMaxTemperatureIfNeeded(filteredTemperature);
        
        detectStoveState(filteredTemperature, getAverageGradient());
    }

    if (!waitingForAck && !cookingEventMessagePendingConfirmation) {
        sendNextPendingMessage();
    }

    if ((!waitingForAck) && (noMessageSentCounter > MAX_ITERATIONS_BETWEEN_MESSAGES)) {
        sendStatusReportRadioMessage();
        // Note: the counter is re-set only when a timestamp is received
        // (i.e. verified successful message transmission)
        // That means also, that as long as there is no timestamp response,
        // SUM is going to be sending a status report every time it wakes up.
        
        noMessageSentCounter = MAX_ITERATIONS_BETWEEN_MESSAGES; // protect from overflow
    }
}

static void configure_rtc_callbacks(void)
{
    rtc_calendar_register_callback(&rtc_instance, appMain, RTC_CALENDAR_CALLBACK_ALARM_0);
    rtc_calendar_enable_callback(&rtc_instance, RTC_CALENDAR_CALLBACK_ALARM_0);
}

static void configure_rtc_calendar(void)
{
    struct rtc_calendar_config config_rtc_calendar;
    rtc_calendar_get_config_defaults(&config_rtc_calendar);
    
    config_rtc_calendar.clock_24h = true;
    config_rtc_calendar.alarm[0].mask = RTC_CALENDAR_ALARM_MASK_SEC;

    rtc_calendar_init(&rtc_instance, RTC, &config_rtc_calendar);
    rtc_calendar_enable(&rtc_instance);
}

static void configure_tcs_callbacks(void)
{
    tc_register_callback(&tc_instance_timeout, tc_callback_timeout, TC_CALLBACK_CC_CHANNEL0);
    tc_enable_callback(&tc_instance_timeout, TC_CALLBACK_CC_CHANNEL0);
}

static void configure_tcs(void)
{
    struct tc_config config_tc;
    
    tc_get_config_defaults(&config_tc);
    config_tc.counter_size = TC_COUNTER_SIZE_16BIT;
    config_tc.clock_source = GCLK_GENERATOR_2;
    config_tc.clock_prescaler = TC_CLOCK_PRESCALER_DIV1;
    config_tc.counter_16_bit.compare_capture_channel[0] = RESPONSE_TIMEOUT * 1024;
    config_tc.oneshot = true;
    config_tc.run_in_standby = true;
    
    tc_init(&tc_instance_timeout, TIMEOUT_TC_MODULE, &config_tc);
}

static void tc_callback_timeout(struct tc_module *const module_inst)
{
    tc_disable(&tc_instance_timeout);
    waitingForAck = false;
    cookingEventMessagePendingConfirmation = false;
    
    UNUSED(module_inst);
}

static void appSleep(void)
{
    // Sleep, if the network is idle, no response is pending, and USB is not connected
    if ((!NWK_Busy()) && (!waitingForAck) && (!usb_serial_status()))
    {
        NWK_SleepReq();
        sleepmgr_enter_sleep();
        NWK_WakeupReq();
    }
}

uint32_t get_rtc_epoch(void)
{
    struct rtc_calendar_time time;
    rtc_calendar_get_time(&rtc_instance, &time);
    
    return rtc_getEpoch(time, rtc_instance.clock_24h);
}

/*
* Creates a unique device ID
*
* This is done by using the 4 words (32 bits) of the SAMD Serial number.
* Word 1 and 3 are XOR-ed, and word 2 and 4 are XOR-ed.
* Then the two words are placed in a 8 byte array. LS byte first (little
* endian).
*/
static void createDeviceId(void)
{
    const uint32_t *sernum1 = (uint32_t *)0x0080A00C;
    const uint32_t *sernum2 = (uint32_t *)0x0080A040;
    const uint32_t *sernum3 = (uint32_t *)0x0080A044;
    const uint32_t *sernum4 = (uint32_t *)0x0080A048;
    
    uint32_t word1 = *sernum1 ^ *sernum3;
    uint32_t word2 = *sernum2 ^ *sernum4;
    
    memcpy(&deviceId[0], &word1, sizeof(word1));
    memcpy(&deviceId[4], &word2, sizeof(word2));
}

int main(void)
{
    irq_initialize_vectors();
    
    system_init();
    delay_init();
    SYS_Init();
    
    configure_rtc_calendar();
    configure_rtc_callbacks();
    
    configure_tcs();
    configure_tcs_callbacks();
    
    usb_vbus_config();
    max31855_init();
    
    system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
    sleepmgr_init();
    
    cpu_irq_enable();
    
    createDeviceId();
    appInit();
    
    ringBuffer_init(&temperatureBuffer);
    ringBuffer_init(&gradientBuffer);
    
    radioMessageBuffer_init();
    
    #ifdef DEBUG
    delay_ms(STARTUP_DELAY);
    
    sprintf(
        displayBuffer, 
        "** Stove Utilization Module (SUM) - %s **\r\n -> DeviceID: " DEVICE_ID_FORMAT "\r\n", 
        VERSION, 
        DEVICE_ID_PARTS(deviceId));
    udi_cdc_multi_write_buf(0, displayBuffer, strlen(displayBuffer));
    #endif

    isAppInitialized = true;

    while (1) {
        SYS_TaskHandler();
        appSleep();
    }
}
