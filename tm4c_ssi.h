#ifndef TM4C_SSI_DSCAO__
#define TM4C_SSI_DSCAO__
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"

struct ssi_port {
	uint32_t base;
	uint8_t tx_dmach;
	volatile uint8_t txdma;
};

void tm4c_ssi_setup(int port);

void ssi0_isr(void);
void ssi1_isr(void);

void tm4c_ssi_write(int port, const char *buf, int len);

#endif /* TM4C_SSI_DSCAO__ */
