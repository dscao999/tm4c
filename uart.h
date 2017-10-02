#ifndef TM4C_UART_DSCAO__
#define TM4C_UART_DSCAO__
#include <stdint.h>
#include "inc/hw_uart.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/rom.h"

struct uart_port {
	uint32_t sysctl;
	uint32_t base;
	uint32_t tx;
	uint32_t rx;
	uint32_t oerr;
	uint32_t ferr;
	uint8_t tx_dmach;
	volatile uint8_t txdma;
	volatile uint8_t rxhead;
	uint8_t rxtail;
	uint8_t rxbuf[128];
};

extern struct uart_port uart0;

int uart_open(struct uart_port *uart, int port);
static inline void uart_close(struct uart_port *uart)
{
	ROM_UARTDisable(uart->base);
	ROM_SysCtlPeripheralDisable(uart->sysctl);
}

void uart_write(struct uart_port *uart, const char *str, int len);
int uart_read(struct uart_port *uart, char *buf, int len, int wait);

void uart0_isr(void);

#endif /* TM4C_UART_DSCAO__ */
