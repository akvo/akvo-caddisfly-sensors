#include "r21_slave.h"

static uint8_t cmd = 0;
static volatile bool spi_device_selected = false;
static volatile bool spi_receive_pending = false;
static volatile bool spi_transmit_pending = false;

struct spi_module spi_slave_instance;

#define RECORD_BUFFER_SIZE 32 // number of records

r21_record_t in_record_buffer[RECORD_BUFFER_SIZE];
r21_record_t out_record_buffer[RECORD_BUFFER_SIZE];

static uint16_t in_index = 0; // points to the next position to write in in_record_buffer
static uint16_t out_index = 0; // points to the next position to read from out_record_buffer

static uint16_t out_max_index = 0; // points to first position after the last position that holds a valid record (may point outside the array safely)

static get_rtc_epoch_callback_ptr get_rtc_callback;
static set_rtc_epoch_callback_ptr set_rtc_callback;

void configure_spi_slave_callbacks(void);
void configure_spi_slave(void);
void configure_spi_slave_select_pin_interrupt(void);
void handle_spi_command(uint8_t command);
void r21_record_to_buffer(r21_record_t* record, uint8_t* buffer);
uint32_t calculate_crc32(r21_record_t* record);

void r21_record_to_buffer(r21_record_t* record, uint8_t* buffer)
{
    uint8_t index = 0;
    uint8_t size = sizeof(record->Timestamp);
    memcpy(&buffer[index], &record->Timestamp, size);
    
    index += size;
    size = sizeof(record->DeviceId);
    memcpy(&buffer[index], record->DeviceId, size);

    index += size;
    size = sizeof(record->MaxTemperature);
    memcpy(&buffer[index], &record->MaxTemperature, size);

    index += size;
    size = sizeof(record->Duration);
    memcpy(&buffer[index], &record->Duration, size);

    index += size;
    size = sizeof(record->BatteryLevel);
    memcpy(&buffer[index], &record->BatteryLevel, size);

    index += size;
    size = sizeof(record->Crc);
    memcpy(&buffer[index], &record->Crc, size);
}

uint32_t calculate_crc32(r21_record_t* record)
{
    crc32_t crc32;
    uint8_t recordBuffer[R21RECORD_SIZE];

    r21_record_to_buffer(record, recordBuffer);
    crc32_calculate(recordBuffer, sizeof(recordBuffer), &crc32);

    return crc32;
}

uint16_t r21_slave_get_free_record_count(void)
{
    return sizeof(in_record_buffer) / sizeof(in_record_buffer[0]) - in_index;
}

bool r21_slave_add_record(r21_record_t* record)
{
    if (r21_slave_get_free_record_count() > 0) {
        record->Crc = 0; // make sure crc is cleared
        in_record_buffer[in_index] = *record;
        in_record_buffer[in_index].Crc = calculate_crc32(record);
        
        in_index++;
        return true;
    }

    return false;
}

uint16_t r21_slave_get_available_record_count(void)
{
    return out_max_index - out_index;
}

void r21_slave_update_out_record_buffer_if_needed(void)
{
    // if out buffer has been read completely and there are records in the in buffer
    if ((r21_slave_get_available_record_count() == 0) && (in_index > 0)) {
        for (uint16_t i = 0; i < in_index; i++) {
            out_record_buffer[i] = in_record_buffer[i]; // shallow copy
        }

        out_max_index = in_index;
        out_index = 0;
        in_index = 0;
    }
}

void r21_slave_delete_record(void)
{
    if (out_index < out_max_index) {
        out_index++;
    }
}

bool r21_slave_get_record(r21_record_t* record)
{
    if (r21_slave_get_available_record_count() > 0) {
        *record = out_record_buffer[out_index];

        return true;
    }

    return false;
}

void handle_spi_command(uint8_t command)
{
    switch (command)
    {
        case CMD_SET_RTC:
        {
            uint8_t rtcParts[4];

            spi_receive_pending = true;
            spi_read_buffer_job(&spi_slave_instance, rtcParts, 4, 0x00);
            while(spi_device_selected && spi_receive_pending) { }

            if (spi_device_selected && set_rtc_callback) {
                uint32_t newRTC = (uint32_t)(rtcParts[3] << 24) | (uint32_t)(rtcParts[2] << 16) | (uint32_t)(rtcParts[1] << 8) | (uint32_t)(rtcParts[0]);
                set_rtc_callback(newRTC);
            }
        }
        break;

        case CMD_GET_RTC:
        {
            uint32_t rtcValue = 0;

            if (get_rtc_callback) {
                rtcValue = get_rtc_callback();
            }

            uint8_t rtcParts[4];
            memcpy(rtcParts, &rtcValue, sizeof(rtcParts));
            
            spi_transmit_pending = true;
            spi_write_buffer_job(&spi_slave_instance, rtcParts, sizeof(rtcParts));
            while(spi_device_selected && spi_transmit_pending) { }
        }
        break;

        case CMD_GET_AVAILABLE_RECORD_COUNT:
        {
            uint16_t recordCount = r21_slave_get_available_record_count();

            uint8_t parts[2];
            memcpy(parts, &recordCount, sizeof(parts));
            
            spi_transmit_pending = true;
            spi_write_buffer_job(&spi_slave_instance, parts, sizeof(parts));
            while(spi_device_selected && spi_transmit_pending) { }
        }
        break;
        
        case CMD_GET_RECORD:
        {
            r21_record_t record;
            uint8_t recordBuffer[R21RECORD_SIZE];

            r21_slave_get_record(&record);
            r21_record_to_buffer(&record, recordBuffer);

            spi_transmit_pending = true;
            spi_write_buffer_job(&spi_slave_instance, recordBuffer, sizeof(recordBuffer));
            while(spi_device_selected && spi_transmit_pending) { }
        }
        break;

        case CMD_DELETE_RECORD:
        {
            r21_slave_delete_record();
        }
        break;

        default:
        
        break;
    }
}

static void spi_device_selected_callback(void)
{
    spi_device_selected = !port_pin_get_input_level(PIN_PB03);

    if (spi_device_selected) {
        // cleanup
        cmd = 0;
        spi_receive_pending = false;
        spi_transmit_pending = false;

        spi_abort_job(&spi_slave_instance);

        // start SPI
        spi_enable(&spi_slave_instance);
    }
    else {
        // stop SPI
        spi_disable(&spi_slave_instance);
    }
}

void configure_spi_slave_select_pin_interrupt(void)
{
    struct extint_chan_conf eint_chan_conf;

    extint_chan_get_config_defaults(&eint_chan_conf);

    eint_chan_conf.gpio_pin            = SS_PIN;
    eint_chan_conf.gpio_pin_mux        = SS_PINMUX;
    eint_chan_conf.gpio_pin_pull       = EXTINT_PULL_NONE;
    eint_chan_conf.detection_criteria  = EXTINT_DETECT_BOTH;
    eint_chan_conf.wake_if_sleeping    = true;
    eint_chan_conf.filter_input_signal = false;

    extint_chan_disable_callback(SS_INT_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_chan_set_config(SS_INT_LINE, &eint_chan_conf);
    extint_register_callback(spi_device_selected_callback, SS_INT_LINE, EXTINT_CALLBACK_TYPE_DETECT);
    extint_chan_enable_callback(SS_INT_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}

static void spi_slave_receive_done_callback(struct spi_module *const module)
{
    spi_receive_pending = false;
    
    UNUSED(module);
}

static void spi_slave_transmit_done_callback(struct spi_module *const module)
{
    spi_transmit_pending = false;
    
    UNUSED(module);
}

void configure_spi_slave_callbacks(void)
{
    spi_register_callback(&spi_slave_instance, spi_slave_receive_done_callback,
    SPI_CALLBACK_BUFFER_RECEIVED);
    spi_enable_callback(&spi_slave_instance, SPI_CALLBACK_BUFFER_RECEIVED);

    spi_register_callback(&spi_slave_instance, spi_slave_transmit_done_callback,
    SPI_CALLBACK_BUFFER_TRANSMITTED);
    spi_enable_callback(&spi_slave_instance, SPI_CALLBACK_BUFFER_TRANSMITTED);
}

void configure_spi_slave(void)
{
    struct spi_config config_spi_slave;
    spi_get_config_defaults(&config_spi_slave);
    
    config_spi_slave.mode = SPI_MODE_SLAVE;
    config_spi_slave.mode_specific.slave.preload_enable = true;
    config_spi_slave.mode_specific.slave.frame_format = SPI_FRAME_FORMAT_SPI_FRAME;

    config_spi_slave.mux_setting = EXT1_SPI_SERCOM_MUX_SETTING;
    
    config_spi_slave.pinmux_pad0 = EXT1_SPI_SERCOM_PINMUX_PAD0; // SI
    config_spi_slave.pinmux_pad1 = EXT1_SPI_SERCOM_PINMUX_PAD1; // SS
    config_spi_slave.pinmux_pad2 = EXT1_SPI_SERCOM_PINMUX_PAD2; // SO
    config_spi_slave.pinmux_pad3 = EXT1_SPI_SERCOM_PINMUX_PAD3; // SCK

    config_spi_slave.run_in_standby = true;

    spi_init(&spi_slave_instance, EXT1_SPI_MODULE, &config_spi_slave);

}

void r21_slave_init(void)
{
    configure_spi_slave();
    configure_spi_slave_callbacks();
    configure_spi_slave_select_pin_interrupt();
}

void r21_slave_task(void)
{
    if (spi_device_selected && (!spi_receive_pending) && (!spi_transmit_pending)) {
        spi_transmit_pending = false;
        spi_receive_pending = true;

        spi_read_buffer_job(&spi_slave_instance, &cmd, 1, 0x00);
        while (spi_device_selected && spi_receive_pending) { }
        
        if (spi_device_selected) { // make sure it is still selected
            handle_spi_command(cmd);
        }
    }
}

void r21_slave_set_rtc_callbacks(get_rtc_epoch_callback_ptr getRTC, set_rtc_epoch_callback_ptr setRTC)
{
    get_rtc_callback = getRTC;
    set_rtc_callback = setRTC;
}