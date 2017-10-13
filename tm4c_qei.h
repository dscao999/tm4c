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
	uint16_t intr;
	uint16_t err;
	uint16_t dir;
};

void tm4c_qei_config(struct qei_port *qei, uint32_t pos);
void qei_setup(int port, uint32_t pos);

static inline void qei_reset(struct qei_port *qei, uint32_t pos)
{
	ROM_SysCtlPeripheralReset(qei->sysperip);
	while(!ROM_SysCtlPeripheralReady(qei->sysperip))
		;
	tm4c_qei_config(qei, pos);
}

void qei0_isr(void);

static inline int32_t qei_getpos(struct qei_port *qei)
{
	return HWREG(qei->base+QEI_O_POS);
}
static inline void qei_setpos(struct qei_port *qei, uint32_t pos)
{
	HWREG(qei->base+QEI_O_POS) = pos;
}

#endif /* TM4C_QEI_DSCAO__ */
