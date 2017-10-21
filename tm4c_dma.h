#ifndef TM4C_DMA_DSCAO__
#define TM4C_DMA_DSCAO__
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_udma.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"

struct dmactl {
	void *src;
	void *dst;
	uint32_t xfer_mode:3;
	uint32_t nxt_use_burst:1;
	uint32_t xfer_size:10;
	uint32_t arb_size:4;
	uint32_t reserv0:6;
	uint32_t src_size:2;
	uint32_t src_inc:2;
	uint32_t dst_size:2;
	uint32_t dst_inc:2;
	uint32_t reserv1;
};

void tm4c_dma_enable(void);
	
#endif /* TM4C_DMA_DSCAO__ */
