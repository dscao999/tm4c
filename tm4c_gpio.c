#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "tm4c_miscs.h"
#include "tm4c_gpio.h"

static uint32_t gpioc_isr_nums = 0;
static uint32_t gpiod_isr_nums = 0;

struct gpio_port gpioms[] = {
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

static inline void tm4c_gpio_filter(struct gpio_port *gpio, uint32_t filt)
{
	uint32_t gpioctl;

	gpioctl = HWREG(gpio->base+QEI_O_CTL) & ~QEI_CTL_FILTCNT_M;
	HWREG(gpio->base+QEI_O_CTL) = gpioctl|filt|QEI_CTL_FILTEN;
}

static void gpio_config(struct gpio_port *gpio, uint32_t pos)
{
	uint32_t gpiomode;

	gpiomode = QEI_CONFIG_CAPTURE_A|QEI_CONFIG_NO_RESET|QEI_CONFIG_QUADRATURE;
	ROM_QEIConfigure(gpio->base, gpiomode, 0xffffffffu);
	tm4c_gpio_filter(gpio, QEI_FILTCNT_12);
	ROM_QEIIntEnable(gpio->base, QEI_INTERROR);
	ROM_IntPrioritySet(gpio->intr, 0xc0);
	HWREG(gpio->base+QEI_O_POS) = pos;
	ROM_QEIEnable(gpio->base);
	ROM_IntEnable(gpio->intr);
}

void tm4c_gpio_config(int port, uint32_t pos)
{
	struct gpio_port *gpio;

	gpio = gpioms+port;
	gpio_config(gpio, pos);
}

void tm4c_gpio_setup(int port, uint32_t pos)
{
	struct gpio_port *gpio;
	uint32_t v;

	gpio = gpioms+port;
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

	ROM_SysCtlPeripheralEnable(gpio->sysperip);
	while(!ROM_SysCtlPeripheralReady(gpio->sysperip))
			;
	gpio_config(gpio, pos);
}

static void gpio_isr(struct gpio_port *gpio)
{
	uint32_t isc;

	isc = HWREG(gpio->base+QEI_O_ISC);
	if (isc)
		HWREG(gpio->base+QEI_O_ISC) = isc;
	if (isc & QEI_INTEN_ERROR)
		gpio->err++;
	if (isc & QEI_INTEN_DIR)
		gpio->dir++;
	if (isc & QEI_INTEN_INDEX)
		gpio->index++;
	gpio->mis = isc;
	gpio->pos = HWREG(gpio->base+QEI_O_POS);
}

void gpioc_isr(void)
{
	gpio_isr(gpioms);
	gpioc_isr_nums++;
}
void gpiod_isr(void)
{
	gpio_isr(gpioms+1);
	gpiod_isr_nums++;
}

void tm4c_gpio_reset(int port, uint32_t pos)
{
	struct gpio_port *gpio;

	gpio = gpioms+port;
	ROM_SysCtlPeripheralReset(gpio->sysperip);
	while(!ROM_SysCtlPeripheralReady(gpio->sysperip))
		;
	gpio_config(gpio, pos);
}

uint32_t tm4c_gpio_getpos(int port)
{
	struct gpio_port *gpio;

	gpio = gpioms+port;
	return HWREG(gpio->base+QEI_O_POS);
}
