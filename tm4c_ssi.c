#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ssi.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "inc/hw_udma.h"
#include "driverlib/ssi.h"
#include "driverlib/udma.h"
#include "tm4c_miscs.h"
#include "tm4c_dma.h"
#include "tm4c_ssi.h"

static uint32_t ssi0_isr_nums = 0;

static struct ssi_port ssims[] = {
	{
		.base = SSI0_BASE,
		.tx_dmach = UDMA_CHANNEL_SSI0TX,
		.rx_dmach = UDMA_CHANNEL_SSI0RX,
		.txdma = 0,
		.numr = 0,
		.numw = 0
	}
};

void tm4c_ssi_setup(int port)
{
	struct ssi_port *ssi;
	uint32_t sysperip;
	int intr;

	ssi = ssims+port;
	switch (port) {
	case 0:
		ROM_GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_2|GPIO_PIN_3,
			GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
		ROM_GPIOPinTypeSSI(GPIO_PORTA_BASE,
			GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
		ROM_GPIOPinConfigure(GPIO_PA2_SSI0CLK);
		ROM_GPIOPinConfigure(GPIO_PA3_SSI0FSS);
		ROM_GPIOPinConfigure(GPIO_PA4_SSI0RX);
		ROM_GPIOPinConfigure(GPIO_PA5_SSI0TX);
		sysperip = SYSCTL_PERIPH_SSI0;
		intr = INT_SSI0_TM4C123;
		break;
	default:
		while(1)
			;
	}

	ROM_SysCtlPeripheralEnable(sysperip);
	while(!ROM_SysCtlPeripheralReady(sysperip))
			;
	ROM_SSIConfigSetExpClk(ssi->base, HZ, SSI_FRF_MOTO_MODE_3,
		SSI_MODE_MASTER, 1000000, 8);

	ROM_uDMAChannelDisable(ssi->tx_dmach);
	ROM_uDMAChannelAttributeDisable(ssi->tx_dmach, UDMA_ATTR_ALL);
	ROM_uDMAChannelControlSet(ssi->tx_dmach|UDMA_PRI_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_8|UDMA_DST_INC_NONE|UDMA_ARB_8);
	ROM_uDMAChannelTransferSet(ssi->tx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, 0, (void *)(ssi->base+SSI_O_DR), 0);
	ROM_SSIDMAEnable(ssi->base, SSI_DMA_TX);

	while (HWREG(ssi->base+SSI_O_SR) & SSI_SR_RNE)
		HWREG(ssi->base+SSI_O_DR);
	ROM_uDMAChannelDisable(ssi->rx_dmach);
	ROM_uDMAChannelAttributeDisable(ssi->rx_dmach, UDMA_ATTR_ALL);
	ROM_uDMAChannelControlSet(ssi->rx_dmach|UDMA_PRI_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_NONE|UDMA_DST_INC_8|UDMA_ARB_8);
	ROM_uDMAChannelTransferSet(ssi->rx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, (void *)(ssi->base+SSI_O_DR), (void *)ssi->buf, SSI_BUFSIZ);
	ROM_SSIDMAEnable(ssi->base, SSI_DMA_RX);

	HWREG(ssi->base+SSI_O_IM) = SSI_IM_RXIM|SSI_IM_RTIM|SSI_IM_RORIM;
	ROM_IntPrioritySet(intr, 0xc0);
	ROM_IntEnable(intr);
	HWREG(ssi->base + SSI_O_CR1) |= SSI_CR1_SSE;
}

static void tm4c_ssi_write_sync(struct ssi_port *ssi, const char *str, int len)
{
	const unsigned char *ustr;
	int i;

	for (i = 0, ustr = (const unsigned char *)str; i < len; ustr++, i++) {
		while((HWREG(ssi->base+SSI_O_SR) & SSI_SR_TNF) == 0)
			;
		HWREG(ssi->base+SSI_O_DR) = *ustr;
	}
}

void tm4c_ssi_write(int port, const char *str, int len)
{
	int dmalen;
	struct ssi_port *ssi = ssims + port;

	while (ssi->txdma)
		tm4c_waitint();
	if (str < (char *)MEMADDR) {
		tm4c_ssi_write_sync(ssi, str, len);
		return;
	}

	dmalen = len > 512? 512 : len;
	ssi->numw += dmalen;
	tm4c_dma_set(ssi->tx_dmach, str, (void *)(ssi->base+SSI_O_DR), dmalen);
	tm4c_dma_set(ssi->rx_dmach, (void *)(ssi->base+SSI_O_DR), ssi->buf, SSI_BUFSIZ);
	ssi->txdma = 1;
	HWREG(UDMA_ENASET) = (1 << ssi->rx_dmach)|(1 << ssi->tx_dmach);
}

void tm4c_ssi_waitdma(int port)
{
	struct ssi_port *ssi = ssims + port;
	while (ssi->txdma)
		tm4c_waitint();
}
	
static void tm4c_ssi_recv(struct ssi_port *ssi)
{
	int head;

	head = ssi->head;
	while (HWREG(ssi->base+SSI_O_SR) & SSI_SR_RNE) {
		ssi->buf[head] = HWREG(ssi->base+SSI_O_DR);
		head = (head + 1) & 0x3f;
		ssi->numrr++;
	}
	ssi->head = head;
}

static void ssi_isr(struct ssi_port *ssi)
{
	uint32_t mis, udma_int;

	mis = HWREG(ssi->base+SSI_O_MIS);
	if (mis & (SSI_MIS_RTMIS|SSI_MIS_RORMIS))
		HWREG(ssi->base+SSI_O_ICR) = mis & (SSI_MIS_RTMIS|SSI_MIS_RORMIS);
	udma_int = HWREG(UDMA_CHIS);
	if (udma_int & (1 << ssi->rx_dmach)) {
		HWREG(UDMA_CHIS) = (1 << ssi->rx_dmach);
		ssi->numr += SSI_BUFSIZ;
		ROM_uDMAChannelTransferSet(ssi->rx_dmach|UDMA_PRI_SELECT,
			UDMA_MODE_BASIC, (void *)(ssi->base+SSI_O_DR), (void *)ssi->buf, SSI_BUFSIZ);
		HWREG(UDMA_ENASET) = (1 << ssi->rx_dmach);
	}
	if (udma_int & (1 << ssi->tx_dmach)) {
		HWREG(UDMA_CHIS) = (1 << ssi->tx_dmach);
		ssi->txdma = 0;
		ssi->numr += SSI_BUFSIZ - tm4c_dma_rem(ssi->rx_dmach) - 1;
		HWREG(UDMA_ENACLR) = 1 << ssi->rx_dmach;
	}
	if (mis & (SSI_MIS_RXMIS|SSI_MIS_RTMIS))
		tm4c_ssi_recv(ssi);
	if (mis & SSI_MIS_RORMIS)
		ssi->overrun++;
}

void ssi0_isr(void)
{
	ssi_isr(ssims);
	ssi0_isr_nums++;
}
