#include "tm4c_dma.h"

static struct dmactl dmacr[32];
uint32_t udmaerr = 0;

void tm4c_dma_enable(void)
{
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
	while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA))
		;
	ROM_uDMAEnable();
	ROM_uDMAControlBaseSet(dmacr);
}
