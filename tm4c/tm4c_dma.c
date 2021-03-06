#include "tm4c_dma.h"
#include "driverlib/udma.h"

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

int tm4c_dma_rem(int ch)
{
	return (dmacr[ch].ctrl & UDMA_CHCTL_XFERSIZE_M) >> UDMA_CHCTL_XFERSIZE_S;
}
