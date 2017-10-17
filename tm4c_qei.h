#ifndef TM4C_QEI_DSCAO__
#define TM4C_QEI_DSCAO__
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_qei.h"
#include "driverlib/sysctl.h"
#include "driverlib/qei.h"
#include "driverlib/rom.h"

struct qei_port {
	uint32_t base;
	uint32_t sysperip;
	uint32_t mis;
	uint32_t pos;
	uint16_t intr;
	uint16_t err;
	uint16_t dir;
	uint16_t index;
};

void tm4c_qei_config(int port, uint32_t pos);
void tm4c_qei_setup(int port, uint32_t pos);

void tm4c_qei_reset(int port, uint32_t pos);

void qei0_isr(void);
void qei1_isr(void);

uint32_t tm4c_qei_getpos(int port);

#endif /* TM4C_QEI_DSCAO__ */
