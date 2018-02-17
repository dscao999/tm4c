#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"
#include "tm4c_miscs.h"
#include "tm4c_gpio.h"

uint32_t gpioc_isr_nums = 0;
uint32_t gpiod_isr_nums = 0;

static struct gpio_port gpioms[] = {
	{
		.base = GPIO_PORTA_BASE,
		.mis = 0
	},
	{
		.base = GPIO_PORTB_BASE,
		.mis = 0
	},
	{
		.base = GPIO_PORTC_BASE,
		.mis = 0
	},
	{
		.base = GPIO_PORTD_BASE,
		.mis = 0
	},
	{
		.base = GPIO_PORTE_BASE,
		.mis = 0
	},
	{
		.base = GPIO_PORTF_BASE,
		.mis = 0
	}
};

void tm4c_gpio_setup(enum GPIOPORT port, uint8_t inps, uint8_t outps, uint8_t intrps)
{
	struct gpio_port *gpio = gpioms+port;
	uint32_t intr;
	uint32_t sysperip;

	switch(port) {
	case GPIOA:
		intr = INT_GPIOA;
		sysperip = SYSCTL_PERIPH_GPIOA;
		break;
	case GPIOB:
		intr = INT_GPIOB;
		sysperip = SYSCTL_PERIPH_GPIOB;
		break;
	case GPIOC:
		intr = INT_GPIOC;
		sysperip = SYSCTL_PERIPH_GPIOC;
		break;
	case GPIOD:
		intr = INT_GPIOD;
		sysperip = SYSCTL_PERIPH_GPIOD;
		break;
	case GPIOE:
		intr = INT_GPIOE;
		sysperip = SYSCTL_PERIPH_GPIOE;
		break;
	case GPIOF:
		intr = INT_GPIOF;
		sysperip = SYSCTL_PERIPH_GPIOF;
		break;
	default:
		while (1)
			;
	}
	ROM_SysCtlPeripheralEnable(sysperip);
	while(!ROM_SysCtlPeripheralReady(sysperip))
                        ;
	if (inps) {
		ROM_GPIODirModeSet(gpio->base, inps, GPIO_DIR_MODE_IN);
		HWREG(gpio->base+GPIO_O_DEN) |= inps;
	}
	if (outps) {
		ROM_GPIOPadConfigSet(gpio->base, outps, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
		ROM_GPIODirModeSet(gpio->base, outps, GPIO_DIR_MODE_OUT);
	}

	if (intrps) {
		HWREG(gpio->base+GPIO_O_IM) = 0;
		ROM_GPIOIntTypeSet(gpio->base, intrps, GPIO_FALLING_EDGE);
		HWREG(gpio->base+GPIO_O_DEN) |= intrps;
		HWREG(gpio->base+GPIO_O_ICR) = 0x0ff;
		HWREG(gpio->base+GPIO_O_IM) = intrps;
		ROM_IntPrioritySet(intr, 0x60);
		ROM_IntEnable(intr);
	}
}

static void gpio_isr(struct gpio_port *gpio)
{
	gpio->mis = HWREG(gpio->base+GPIO_O_MIS);
	HWREG(gpio->base+GPIO_O_ICR) = gpio->mis & 0x0ff;
	HWREG(gpio->base+GPIO_O_IM) &= ~gpio->mis;
}

void gpioc_isr(void)
{
	struct gpio_port *gpio = gpioms+GPIOC;

	gpio_isr(gpio);
	gpioc_isr_nums++;
}

void gpiod_isr(void)
{
	struct gpio_port *gpio = gpioms+GPIOD;

	gpio_isr(gpio);
	gpiod_isr_nums++;
}

void tm4c_gpio_write(enum GPIOPORT port, uint8_t pins, int on_off)
{
	struct gpio_port *gpio = gpioms+port;
	uint8_t v;

	v = on_off? 0xff : 0x0;
	ROM_GPIOPinWrite(gpio->base, pins, v);
}
