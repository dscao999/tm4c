#ifndef TM4C_SSI_DSCAO__
#define TM4C_SSI_DSCAO__
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/ssi.h"
#include "driverlib/rom.h"

#define SSI_BUFSIZ	64
struct ssi_port {
	uint32_t base;
	uint16_t numr;
	uint16_t numw;
	uint16_t numrr;
	uint16_t overrun;
	uint8_t tx_dmach;
	uint8_t rx_dmach;
	uint8_t eot;
	volatile uint8_t txdma;
	volatile uint8_t head;
	uint8_t tail;
	uint16_t buf[SSI_BUFSIZ];
};

void tm4c_ssi_setup(int port);

void ssi0_isr(void);

void tm4c_ssi_write(int port, const uint16_t *buf, int len);
void tm4c_ssi_waitdma(int port);

#endif /* TM4C_SSI_DSCAO__ */
