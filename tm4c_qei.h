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
	uint16_t err;
	uint16_t dir;
};

void qei_setup(int port);

void qei0_isr(void);

#endif /* TM4C_QEI_DSCAO__ */
