#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_qei.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/qei.h"
#include "tm4c_miscs.h"
#include "tm4c_qei.h"

static uint32_t qei0_isr_nums = 0;
static uint32_t qei1_isr_nums = 0;

static struct qei_port qeims[] = {
	{
		.base = QEI0_BASE,
	},
	{
		.base = QEI1_BASE,
	}
};

static void qei_config(uint32_t base, int intr,  uint32_t pos)
{
	uint32_t qeimode, qeictl;

	qeimode = QEI_CONFIG_CAPTURE_A|QEI_CONFIG_NO_RESET|QEI_CONFIG_QUADRATURE;
	ROM_QEIConfigure(base, qeimode, 0xffffffffu);
	qeictl = HWREG(base+QEI_O_CTL) & ~QEI_CTL_FILTCNT_M;
	HWREG(base+QEI_O_CTL) = qeictl|QEI_FILTCNT_12|QEI_CTL_FILTEN;
	ROM_QEIIntEnable(base, QEI_INTERROR);
	ROM_IntPrioritySet(intr, 0xc0);
	HWREG(base+QEI_O_POS) = pos;
	ROM_QEIEnable(base);
	ROM_IntEnable(intr);
}

void tm4c_qei_setup(int port, uint32_t pos, int maxpos, int minpos)
{
	struct qei_port *qei;
	uint32_t sysperip, v;
	int intr;

	qei = qeims+port;
	qei->maxpos = maxpos;
	qei->minpos = minpos;
	switch (port) {
	case 0:
		HWREG(GPIO_PORTD_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
		v = HWREG(GPIO_PORTD_BASE+GPIO_O_CR);
		HWREG(GPIO_PORTD_BASE+GPIO_O_CR) = v|0x0ff;
		ROM_GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7);
		ROM_GPIOPinConfigure(GPIO_PD6_PHA0);
		ROM_GPIOPinConfigure(GPIO_PD7_PHB0);
		sysperip = SYSCTL_PERIPH_QEI0;
		intr = INT_QEI0;
		break;
	case 1:
		ROM_GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_5|GPIO_PIN_6);
		ROM_GPIOPinConfigure(GPIO_PC5_PHA1);
		ROM_GPIOPinConfigure(GPIO_PC6_PHB1);
		sysperip = SYSCTL_PERIPH_QEI1;
		intr = INT_QEI1;
		break;
	default:
		while(1)
			;
	}

	ROM_SysCtlPeripheralEnable(sysperip);
	while(!ROM_SysCtlPeripheralReady(sysperip))
			;
	qei_config(qei->base, intr, pos);
}

static void qei_isr(struct qei_port *qei)
{
	uint32_t isc;

	isc = HWREG(qei->base+QEI_O_ISC);
	if (isc)
		HWREG(qei->base+QEI_O_ISC) = isc;
	if (isc & QEI_INTEN_ERROR)
		qei->err++;
	isc = HWREG(qei->base+QEI_O_ISC);
}

void qei0_isr(void)
{
	qei_isr(qeims);
	qei0_isr_nums++;
}
void qei1_isr(void)
{
	qei_isr(qeims+1);
	qei1_isr_nums++;
}

int tm4c_qei_getpos(int port)
{
	struct qei_port *qei;
	int pos;

	qei = qeims+port;
	pos = HWREG(qei->base+QEI_O_POS);
	if (pos > qei->maxpos)
		pos = qei->maxpos;
	else if (pos < qei->minpos)
		pos = qei->minpos;
	return pos;
}
