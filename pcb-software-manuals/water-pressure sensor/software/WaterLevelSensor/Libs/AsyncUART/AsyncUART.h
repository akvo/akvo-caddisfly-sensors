#ifndef ASYNCUART_H_
#define ASYNCUART_H_

#ifndef BAUD
#define BAUD 4800
#endif

#include <util/setbaud.h>
#include <stdbool.h>

#ifndef UART_TX_BUFFER_SIZE
#define UART_TX_BUFFER_SIZE 64
#endif

struct tx_ring;

void uart_init();
void uart_putchar(const char c);
void uart_printstring(const char* c);
bool uart_txPending();


#endif /* ASYNCUART_H_ */
