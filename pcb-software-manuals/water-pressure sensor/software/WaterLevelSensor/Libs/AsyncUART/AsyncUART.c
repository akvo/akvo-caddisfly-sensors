/* Based on Atmel Application Note AVR 306 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <avr/sfr_defs.h>

#include "AsyncUART.h"

struct tx_ring {
    uint8_t buffer[UART_TX_BUFFER_SIZE];
    uint8_t start;
    uint8_t end;
};

static struct tx_ring txBuffer;

void uart_init()
{
    txBuffer.start = 0;
    txBuffer.end   = 0;
    
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;
    
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00); // 8-bit data
    UCSR0B = _BV(TXEN0); // enable TX
}

void uart_putchar(const char c)
{
    // txBuffer.end is the next clear position to write to
    uint8_t nextWritePosition = (txBuffer.end + 1) % UART_TX_BUFFER_SIZE;
    
    if (nextWritePosition != txBuffer.start) {
        txBuffer.buffer[txBuffer.end] = c;
        txBuffer.end = nextWritePosition;
        
        // Data available. Enable the transmit interrupt for serial port 0
        UCSR0B |= _BV(UDRIE0);
    }
}

void uart_printstring(const char* c)
{
    while(*c) {
        uart_putchar(*c);
        c++;
    }
}

ISR(USART0_UDRE_vect)
{
    // txBuffer.start is the next position waiting to be sent
    uint8_t nextReadPosition = (txBuffer.start + 1) % UART_TX_BUFFER_SIZE;
    
    if (txBuffer.start != txBuffer.end) {
        UDR0 = txBuffer.buffer[txBuffer.start];
        txBuffer.start = nextReadPosition;
    } else {
        // Nothing to send. Disable the transmit interrupt for serial port 0
        UCSR0B &= ~_BV(UDRIE0);
    }
}

bool uart_txPending()
{
    // just check the UART interrupt state
    return bit_is_set(UCSR0B, UDRIE0);
}
