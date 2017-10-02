#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/gpio.h"
#include "driverlib/udma.h"
#include "tm4c_miscs.h"
#include "tm4c_dma.h"
#include "uart.h"

static volatile uint32_t uisr = 0;

struct uart_port uart0 = {
	.rxhead = 0,
	.rxtail = 0,
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
		uart->tx_dmach = UDMA_CHANNEL_UART0TX;
		ROM_uDMAChannelAssign(UDMA_CH9_UART0TX);
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
	imask = UART_INT_OE|UART_INT_FE|UART_INT_TX|UART_INT_RX|UART_INT_RT;
	ROM_UARTIntEnable(uart->base, imask);
	ROM_IntPrioritySet(INT_UART0, 0xe0);

	ROM_uDMAChannelAttributeDisable(uart->tx_dmach, UDMA_ATTR_ALL);
	ROM_uDMAChannelControlSet(uart->tx_dmach|UDMA_PRI_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_8|UDMA_DST_INC_NONE|UDMA_ARB_4);
	ROM_uDMAChannelDisable(uart->tx_dmach);

	ROM_UARTEnable(uart->base);
	ROM_IntEnable(INT_UART0);
	return port;
}

static void uart_write_sync(struct uart_port *uart, const char *str, int len)
{
	const unsigned char *ustr;
	int i;

	for (i = 0, ustr = (const unsigned char *)str; i < len; ustr++, i++)
		ROM_UARTCharPut(uart->base, *ustr);
}

void uart_write(struct uart_port *uart, const char *str, int len)
{
	int dmalen;

	if (str < (char *)MEMADDR) {
		uart_write_sync(uart, str, len);
		return;
	}
	dmalen = len > 1024? 1024 : len;
	while (uart->txdma)
		__asm__ __volatile__("wfi");
	ROM_uDMAChannelTransferSet(uart->tx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, (void *)str, (void *)(uart->base+UART_O_DR), dmalen);
	ROM_UARTDMAEnable(uart->base, UART_DMA_TX);
	ROM_uDMAChannelEnable(uart->tx_dmach);
	uart->txdma = 1;
}

int uart_read(struct uart_port *uart, char *buf, int len, int wait)
{
	uint8_t *uchar;
	int count, tail, head;

	tail = uart->rxtail;
	while (wait && tail == uart->rxhead)
		__asm__ __volatile__("wfi");

	head = uart->rxhead;
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
	uint32_t icr, mis, err, udma_int;

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
	if (mis) {
		icr = HWREG(uart->base+UART_O_ICR);
		HWREG(uart->base+UART_O_ICR) = icr|mis;
	}
	udma_int = HWREG(UDMA_CHIS);
	if (udma_int & (1 << uart->tx_dmach)) {
		HWREG(UDMA_CHIS) = udma_int;
		uart->txdma = 0;
	}
}

void uart0_isr(void)
{
	uisr++;
	uart_isr(&uart0);
}
