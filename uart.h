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
	uint8_t rxhead, rxtail;
	uint8_t txhead, txtail;
	uint32_t oerr;
	uint32_t ferr;
	uint8_t txdma, rxdma, dma_func, dma_burst;
	uint8_t rxbuf[128];
	uint8_t txbuf[128];
};

extern struct uart_port uart0;

int uart_open(struct uart_port *uart, int port);
static inline void uart_close(struct uart_port *uart)
{
	ROM_UARTDisable(uart->base);
	ROM_SysCtlPeripheralDisable(uart->sysctl);
}

void uart_write(struct uart_port *uart, const char *str);
void uart_write_sync(struct uart_port *uart, const char *str);
int uart_read(struct uart_port *uart, char *buf, int len);

void uart0_isr(void);

#endif /* TM4C_UART_DSCAO__ */
