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
};

int uart_open(struct uart_port *uart, int port);
static inline void uart_close(struct uart_port *uart)
{
	ROM_UARTDisable(uart->base);
	ROM_SysCtlPeripheralDisable(uart->sysctl);
}

void uart_write(struct uart_port *uart, const char *str);

#endif /* TM4C_UART_DSCAO__ */
