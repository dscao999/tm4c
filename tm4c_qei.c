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

struct qei_port qeims[] = {
	{
		.base = QEI0_BASE,
		.sysperip = SYSCTL_PERIPH_QEI0,
		.intr = INT_QEI0
	},
	{
		.base = QEI1_BASE,
		.sysperip = SYSCTL_PERIPH_QEI1,
		.intr = INT_QEI1
	}
};

static inline void tm4c_qei_filter(struct qei_port *qei, uint32_t filt)
{
	uint32_t qeictl;

	qeictl = HWREG(qei->base+QEI_O_CTL) & ~QEI_CTL_FILTCNT_M;
	HWREG(qei->base+QEI_O_CTL) = qeictl|filt|QEI_CTL_FILTEN;
}

static void qei_config(struct qei_port *qei, uint32_t pos)
{
	uint32_t qeimode;

	qeimode = QEI_CONFIG_CAPTURE_A|QEI_CONFIG_NO_RESET|QEI_CONFIG_QUADRATURE;
	ROM_QEIConfigure(qei->base, qeimode, 0xffffffffu);
	tm4c_qei_filter(qei, QEI_FILTCNT_12);
	ROM_QEIIntEnable(qei->base, QEI_INTERROR);
	ROM_IntPrioritySet(qei->intr, 0xc0);
	HWREG(qei->base+QEI_O_POS) = pos;
	ROM_QEIEnable(qei->base);
	ROM_IntEnable(qei->intr);
}

void tm4c_qei_config(int port, uint32_t pos)
{
	struct qei_port *qei;

	qei = qeims+port;
	qei_config(qei, pos);
}

void tm4c_qei_setup(int port, uint32_t pos)
{
	struct qei_port *qei;
	uint32_t v;

	qei = qeims+port;
	switch (port) {
	case 0:
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
		while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD))
			;
		HWREG(GPIO_PORTD_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;
		v = HWREG(GPIO_PORTD_BASE+GPIO_O_CR);
		HWREG(GPIO_PORTD_BASE+GPIO_O_CR) = v|0x0ff;
		ROM_GPIOPinTypeQEI(GPIO_PORTD_BASE, GPIO_PIN_6|GPIO_PIN_7);
		ROM_GPIOPinConfigure(GPIO_PD6_PHA0);
		ROM_GPIOPinConfigure(GPIO_PD7_PHB0);
		break;
	case 1:
		ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
		while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC))
			;
		ROM_GPIOPinTypeQEI(GPIO_PORTC_BASE, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6);
		ROM_GPIOPinConfigure(GPIO_PC4_IDX1);
		ROM_GPIOPinConfigure(GPIO_PC5_PHA1);
		ROM_GPIOPinConfigure(GPIO_PC6_PHB1);
		break;
	default:
		while(1)
			;
	}

	ROM_SysCtlPeripheralEnable(qei->sysperip);
	while(!ROM_SysCtlPeripheralReady(qei->sysperip))
			;
	qei_config(qei, pos);
}

static void qei_isr(struct qei_port *qei)
{
	uint32_t isc;

	isc = HWREG(qei->base+QEI_O_ISC);
	if (isc)
		HWREG(qei->base+QEI_O_ISC) = isc;
	if (isc & QEI_INTEN_ERROR)
		qei->err++;
	if (isc & QEI_INTEN_DIR)
		qei->dir++;
	if (isc & QEI_INTEN_INDEX)
		qei->index++;
	qei->mis = isc;
	qei->pos = HWREG(qei->base+QEI_O_POS);
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

void tm4c_qei_reset(int port, uint32_t pos)
{
	struct qei_port *qei;

	qei = qeims+port;
	ROM_SysCtlPeripheralReset(qei->sysperip);
	while(!ROM_SysCtlPeripheralReady(qei->sysperip))
		;
	qei_config(qei, pos);
}

uint32_t tm4c_qei_getpos(int port)
{
	struct qei_port *qei;

	qei = qeims+port;
	return HWREG(qei->base+QEI_O_POS);
}