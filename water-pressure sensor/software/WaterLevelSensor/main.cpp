#include <util/delay.h>
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/sfr_defs.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <avr/sleep.h>

#include <MS5803_14BA.h>
#include <InputOutputMacros.h>

extern "C" {
    #include <AsyncUART.h>
    #include <MinimalSPI.h>
}

#define CS 0,B // 0
#define RS485_TRANSMITTER_DATA_ENABLE 2,B

#define enableRS485Transmitter(x) on(RS485_TRANSMITTER_DATA_ENABLE)
#define disableRS485Transmitter(x) off(RS485_TRANSMITTER_DATA_ENABLE)

#define WDT_WAKE_UP_SECONDS 2
#define WAKE_UP_EVERY_X_SECONDS 2
#define WDT_WAKE_UP_COUNTER_MAX WAKE_UP_EVERY_X_SECONDS / WDT_WAKE_UP_SECONDS

#define NIBBLE_TO_HEX_CHAR(i) ((i <= 9) ? ('0' + i) : ('A' - 10 + i))
#define HIGH_NIBBLE(i) ((i >> 4) & 0x0F)
#define LOW_NIBBLE(i) (i & 0x0F)

void int32ToStr(int32_t value, char* buffer);
void addToCRC(uint16_t* crc, const char* c);

MS5803_14BA sensor = MS5803_14BA(0);
char conversionBuffer[10 + 1 + 1]; // "1000000000" + sign + \0

volatile bool wdtWakeUp = true;
volatile uint8_t wdtWakeUpCounter = 0;

void int32ToStr(int32_t value, char* buffer)
{
    static uint32_t powers[] = { 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

    uint32_t unsignedValue = 0;
    
    if (value < 0) {
        *buffer++ = '-';
        unsignedValue = (uint32_t) (-1 * value);
    } else {
        *buffer++ = '+';
        unsignedValue = (uint32_t) value;
    }

    for (int8_t i = 8; i >= 0; i--) {
        char digit = '0';
        uint32_t power = powers[i];
        
        while (unsignedValue >= power) {
            unsignedValue -= power;
            digit++;
        }
        
        *buffer++ = digit;
    }
    
    *buffer++ = '0' + unsignedValue;
    *buffer = '\0';
}

void addToCRC(uint8_t* crc, const char* c) {
    while (*c) {
        *crc += *c;
        c++;
    }
}

int main()
{
    cli();
    
    REMAP |= _BV(U0MAP); // redirect the UART pins to (RX,TX)=(PB2,PA7)
    uart_init();

    REMAP |= _BV(SPIMAP); // redirect SPI pins to (MISO,MOSI,SCK)=(PA0,PA1,PA3)
    spi_init();
    out(SS); // SS is not used, so make sure it is set to output for master SPI
    off(SS); // then it can be kept low (otherwise, if it set to input, it needs to be high for master SPI)
    
    out(RS485_TRANSMITTER_DATA_ENABLE);
    disableRS485Transmitter();
        
    wdt_enable(WDTO_2S);
    WDTCSR |= _BV(WDIE);
    
    sei();

    // if sensor init fails, send failure message and reset
    if (!sensor.init()) {
        uart_printstring("failure\r\n");
        for (;;); // cause a reset
    }
    
    for(;;) {
        wdt_reset();
        WDTCSR |= _BV(WDIE);

        if (wdtWakeUp) {
            wdtWakeUp = false;
        
            sensor.read();

            int32_t pressure = sensor.getPressure();
            int32_t temperature = sensor.getTemperature();
            uint8_t crc = 0;

            enableRS485Transmitter();
            uart_printstring("$");
            
            int32ToStr(pressure, conversionBuffer);
            uart_printstring(conversionBuffer);
            addToCRC(&crc, conversionBuffer);
            
            uart_printstring(",");
            addToCRC(&crc, ",");
            
            int32ToStr(temperature, conversionBuffer);
            uart_printstring(conversionBuffer);
            addToCRC(&crc, conversionBuffer);
            
            uart_printstring("*");
            
            uart_putchar(NIBBLE_TO_HEX_CHAR(HIGH_NIBBLE(crc)));
            uart_putchar(NIBBLE_TO_HEX_CHAR(LOW_NIBBLE(crc)));
            
            uart_printstring("\r\n");
            
            // wait for UART to be done sending before going to sleep
            while (uart_txPending()) { }
            _delay_ms(5); // last char
            
            disableRS485Transmitter();
        }
        
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        cli();
        if (!wdtWakeUp)
        {
            sleep_enable();
            sei();
            sleep_cpu();
            sleep_disable();
        }
        sei();
    }
}

ISR(WDT_vect)
{
    wdtWakeUpCounter++;
    if (wdtWakeUpCounter >= WDT_WAKE_UP_COUNTER_MAX){
        wdtWakeUpCounter = 0;
        wdtWakeUp = true;
    }        
}
