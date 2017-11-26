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

void tm4c_dma_set(int ch, const char *src, char *dst, int len)
{
	if ((dmacr[ch].ctrl & UDMA_CHCTL_SRCINC_M) != UDMA_SRC_INC_NONE)
		dmacr[ch].src = src + len - 1;
	else
		dmacr[ch].src = src;
	if ((dmacr[ch].ctrl & UDMA_CHCTL_DSTINC_M) != UDMA_DST_INC_NONE)
		dmacr[ch].dst = dst + len - 1;
	else
		dmacr[ch].dst = dst;
	dmacr[ch].ctrl &= ~(UDMA_CHCTL_XFERSIZE_M|UDMA_CHCTL_XFERMODE_M);
	dmacr[ch].ctrl |= (((len -1) << UDMA_CHCTL_XFERSIZE_S)|UDMA_CHCTL_XFERMODE_BASIC);
}
