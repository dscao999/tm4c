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
#include "tm4c_ssi.h"

static uint32_t ssi0_isr_nums = 0;

static struct ssi_port ssims[] = {
	{
		.base = SSI0_BASE,
		.tx_dmach = UDMA_CHANNEL_SSI0TX,
		.txdma = 0
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
		GPIOPadConfigSet(ssi->base, GPIO_PIN_PA2, GPIO_STRENGTH_2MA,
			GPIO_PIN_TYPE_STD_WPU);
		GPIOPadConfigSet(ssi->base, GPIO_PIN_PA3, GPIO_STRENGTH_2MA,
			GPIO_PIN_TYPE_STD_WPU);
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
		SSI_MODE_MASTER, 2000000, 8);

	ROM_uDMAChannelDisable(ssi->tx_dmach);
	ROM_uDMAChannelAttributeDisable(ssi->tx_dmach, UDMA_ATTR_ALL);
	ROM_uDMAChannelControlSet(ssi->tx_dmach|UDMA_PRI_SELECT,
		UDMA_SIZE_8|UDMA_SRC_INC_8|UDMA_DST_INC_NONE|UDMA_ARB_8);
	ROM_SSIDMAEnable(ssi->base, SSI_DMA_TX);
	ROM_SSIIntEnable(ssi->base, SSI_TXFF);
	ROM_IntPrioritySet(intr, 0xc0);

	HWREG(ssi->base + SSI_O_CR1) |= SSI_CR1_SSE;
	ROM_IntEnable(intr);
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
	ROM_uDMAChannelTransferSet(ssi->tx_dmach|UDMA_PRI_SELECT,
		UDMA_MODE_BASIC, (void *)str, (void *)(ssi->base+SSI_O_DR), dmalen);
	while((HWREG(ssi->base+SSI_O_SR) & SSI_SR_TFE) == 0)
		;
	ssi->txdma = 1;
	ROM_uDMAChannelEnable(ssi->tx_dmach);
}

int tm4c_ssi_waitdma(int port)
{
	struct ssi_port *ssi = ssims + port;
	while (ssi->txdma)
		tm4c_waitint();
}
	
static void ssi_isr(struct ssi_port *ssi)
{
	uint32_t mis, udma_int;

	mis = HWREG(ssi->base+SSI_O_MIS);
	udma_int = HWREG(UDMA_CHIS);
	if (mis & 0x03)
		HWREG(ssi->base+SSI_O_ICR) = mis & 0x03;
	if (udma_int & (1 << ssi->tx_dmach)) {
		HWREG(UDMA_CHIS) = (1 << ssi->tx_dmach);
		ssi->txdma = 0;
	}
	tm4c_memsync();
	HWREG(ssi->base+SSI_O_ICR);
	HWREG(UDMA_CHIS);
}

void ssi0_isr(void)
{
	ssi_isr(ssims);
	ssi0_isr_nums++;
}
