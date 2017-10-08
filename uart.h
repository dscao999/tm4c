#ifndef TM4C_UART_DSCAO__
#define TM4C_UART_DSCAO__
#include <stdint.h>
#include "inc/hw_uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"

struct uart_port {
	uint32_t base;
	uint16_t oerr;
	uint16_t ferr;
	uint8_t tx_dmach;
	volatile uint8_t txdma;
	volatile uint8_t rxhead;
	uint8_t rxtail;
	uint8_t rxbuf[128];
};

void uart_open(int port);
void uart_close(int port);

void uart_write(int port, const char *str, int len);
int uart_read(int port, char *buf, int len, int wait);

void uart0_isr(void);

#endif /* TM4C_UART_DSCAO__ */
