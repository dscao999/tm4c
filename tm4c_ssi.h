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
	uint32_t numr;
	uint32_t numw;
	uint16_t numrr;
	uint16_t overrun;
	uint8_t tx_dmach;
	uint8_t rx_dmach;
	volatile uint8_t txdma;
	volatile uint8_t head;
	uint8_t tail;
	uint8_t buf[64];
};

void tm4c_ssi_setup(int port);

void ssi0_isr(void);
void ssi1_isr(void);

void tm4c_ssi_write(int port, const char *buf, int len);
void tm4c_ssi_waitdma(int port);

#endif /* TM4C_SSI_DSCAO__ */
