#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/gpio.h"
#include "tm4c_miscs.h"
#include "tm4c_dma.h"
#include "uart.h"

static volatile uint32_t uisr = 0;

struct uart_port uart0 = {
	.rxhead = 0,
	.rxtail = 0,
	.txhead = 0,
	.txtail = 0,
};

int uart_open(struct uart_port *uart, int port)
{
	uint32_t imask;

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
		uart->rxdma = 8;
		uart->txdma = 9;
		uart->dma_func = 0;
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
	ROM_UARTConfigSetExpClk(uart->base, HZ, 115200,
		UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE);
	ROM_UARTFIFOLevelSet(uart->base, UART_FIFO_TX2_8, UART_FIFO_RX6_8);
	ROM_UARTFIFOEnable(uart->base);
	uart->dma_burst = 12;
	imask = UART_INT_OE|UART_INT_FE|UART_INT_TX|UART_INT_RX|UART_INT_RT;
	ROM_UARTIntEnable(uart->base, imask);
	ROM_IntPrioritySet(INT_UART0, 0xe0);
	ROM_UARTEnable(uart->base);
	ROM_IntEnable(INT_UART0);
	return port;
}

void uart_write_sync(struct uart_port *uart, const char *str)
{
	const unsigned char *ustr;

	for (ustr = (const unsigned char *)str; *ustr != 0; ustr++) {
/*		if (HWREG(uart->base+UART_O_FR) & UART_FR_TXFF) {
			while((HWREG(uart->base+UART_O_FR) & UART_FR_TXFE) == 0)
				;
		}
		HWREGB(uart->base+UART_O_DR) = *ustr; */
		ROM_UARTCharPut(uart->base, *ustr);
	}
}

void uart_write(struct uart_port *uart, const char *str)
{
	const unsigned char *ustr;

	for (ustr = (const unsigned char *)str; *ustr != 0; ustr++) {
		if(uart->tx) {
			while((HWREG(uart->base+UART_O_FR) & UART_FR_TXFE) == 0)
				;
			uart->tx = 0;
		}
		HWREGB(uart->base+UART_O_DR) = *ustr;
	}
}

int uart_read(struct uart_port *uart, char *buf, int len)
{
	uint8_t *uchar;
	uint8_t head, tail;
	int count;

	head = uart->rxhead;
	tail = uart->rxtail;
	
	count = 0;
	uchar = (uint8_t *)buf;
	while (tail != head && count < len) {
		*uchar++ = uart->rxbuf[tail];
		tail = (tail + 1) & 0x7f;
		count++;
	}
	uart->rxtail = tail;
	return count;
}

static void uart_recv(struct uart_port *uart)
{
	int32_t oh, hd;

	uart->rx++;
	oh = uart->rxhead;
	hd = oh;
	while ((HWREG(uart->base+UART_O_FR) & UART_FR_RXFE) == 0) {
		uart->rxbuf[hd] = HWREG(uart->base+UART_O_DR);
		hd = (hd + 1) & 0x7f;
	}
	uart->rxhead = hd;
}

static void uart_isr(struct uart_port *uart)
{
	uint32_t icr, mis, err, imask;

	err = 0;
	mis = HWREG(uart->base+UART_O_MIS);
	if (mis & UART_INT_TX)
		uart->tx++;
	if ((mis & UART_INT_RX) || (mis & UART_INT_RT))
		uart_recv(uart);
	if (mis & UART_INT_OE) {
		uart->oerr++;
		err = 1;
	}
	if (mis & UART_INT_FE) {
		uart->ferr++;
		err = 1;
	}
	if (err)
		HWREG(uart->base+UART_O_ECR) = 0x0ff;
	imask = HWREG(uart->base+UART_O_IM);
	icr = HWREG(uart->base+UART_O_ICR);
	icr |= (mis & imask);
	HWREG(uart->base+UART_O_ICR) = icr;
}

void uart0_isr(void)
{
	uisr++;
	uart_isr(&uart0);
}
