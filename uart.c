#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/gpio.h"
#include "tm4c_miscs.h"
#include "uart.h"

uint8_t uart_rbuf[128];
uint8_t uart_xbuf[128];

int uart_open(struct uart_port *uart, int port)
{
	if (port < 0 || port > 7)
		return -1;
	switch(port) {
	case 0:
		uart->sysctl = SYSCTL_PERIPH_UART0;
		uart->base = UART0_BASE;
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
		while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
			;
		ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0|GPIO_PIN_1);
		ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
		ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
		break;
	case 1:
		uart->sysctl = SYSCTL_PERIPH_UART1;
		uart->base = UART1_BASE;
		break;
	case 2:
		uart->sysctl = SYSCTL_PERIPH_UART2;
		uart->base = UART2_BASE;
		break;
	case 3:
		uart->sysctl = SYSCTL_PERIPH_UART3;
		uart->base = UART3_BASE;
		break;
	case 4:
		uart->sysctl = SYSCTL_PERIPH_UART4;
		uart->base = UART4_BASE;
		break;
	case 5:
		uart->sysctl = SYSCTL_PERIPH_UART5;
		uart->base = UART5_BASE;
		break;
	case 6:
		uart->sysctl = SYSCTL_PERIPH_UART6;
		uart->base = UART6_BASE;
		break;
	case 7:
		uart->sysctl = SYSCTL_PERIPH_UART7;
		uart->base = UART7_BASE;
		break;
	}
	ROM_SysCtlPeripheralEnable(uart->sysctl);
	while (!ROM_SysCtlPeripheralReady(uart->sysctl))
		;
	ROM_UARTClockSourceSet(uart->base, UART_CLOCK_SYSTEM);
	ROM_UARTConfigSetExpClk(uart->base, 80000000, 115200,
		UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE);
	ROM_UARTEnable(uart->base);
	return port;
}

void uart_write(struct uart_port *uart, const char *str)
{
	const unsigned char *ustr;

	for (ustr = (const unsigned char *)str; *ustr != 0; ustr++) {
		while (ROM_UARTBusy(uart->base))
			tm4c_ledblink(BLUE, 5, 5);
		if (*ustr == '\n')
			ROM_UARTCharPut(uart->base, 0x0d);
		ROM_UARTCharPut(uart->base, *ustr);
	}
}
