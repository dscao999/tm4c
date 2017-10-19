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

void tm4c_gpio_setup(enum GPIOPORT port)
{
	struct gpio_port *gpio = gpioms+port;
	uint32_t in_pins, out_pins, int_pins, intr;
	uint32_t sysperip;

	switch(port) {
	case GPIOA:
		intr = 0;
		in_pins = 0;
		out_pins = 0;
		int_pins = 0;
		sysperip = SYSCTL_PERIPH_GPIOA;
		break;
	case GPIOB:
		intr = 0;
		in_pins = 0;
		out_pins = 0;
		int_pins = 0;
		sysperip = SYSCTL_PERIPH_GPIOB;
		break;
	case GPIOC:
		intr = INT_GPIOC_TM4C123;
		in_pins = GPIO_PIN_4;
		out_pins = 0;
		int_pins = GPIO_INT_PIN_4;
		sysperip = SYSCTL_PERIPH_GPIOC;
		break;
	case GPIOD:
		intr = INT_GPIOD_TM4C123;
		in_pins = GPIO_PIN_3;
		out_pins = 0;
		int_pins = GPIO_INT_PIN_3;
		sysperip = SYSCTL_PERIPH_GPIOD;
		break;
	case GPIOE:
		intr = 0;
		in_pins = 0;
		out_pins = 0;
		int_pins = 0;
		sysperip = SYSCTL_PERIPH_GPIOE;
		break;
	case GPIOF:
		intr = 0;
		in_pins = 0;
		out_pins = GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1;
		int_pins = 0;
		sysperip = SYSCTL_PERIPH_GPIOF;
		break;
	default:
		while (1)
			;
	}
	ROM_SysCtlPeripheralEnable(sysperip);
	while(!ROM_SysCtlPeripheralReady(sysperip))
                        ;
	if (in_pins) {
		ROM_GPIOPinTypeGPIOInput(gpio->base, in_pins);
		HWREG(gpio->base+GPIO_O_PDR) = in_pins;
	}
	if (out_pins) {
		ROM_GPIOPinTypeGPIOOutput(gpio->base, out_pins);
		HWREG(gpio->base+GPIO_O_PDR) = out_pins;
	}
	if (int_pins && intr) {
		HWREG(gpio->base+GPIO_O_IM) = 0;
		ROM_GPIOIntTypeSet(gpio->base, int_pins, GPIO_FALLING_EDGE);
		HWREG(gpio->base+GPIO_O_DEN) |= int_pins;
		HWREG(gpio->base+GPIO_O_ICR) = 0x0ff;
		HWREG(gpio->base+GPIO_O_IM) = int_pins;
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

int tm4c_gpio_intpin(enum GPIOPORT port, uint32_t pins)
{
	struct gpio_port *gpio = gpioms + port;
	
	int retv;

	retv = 0;
	if (gpio->mis & pins) {
		gpio->mis &= ~pins;
		HWREG(gpio->base+GPIO_O_ICR) = pins;
		HWREG(gpio->base+GPIO_O_IM) |= pins;
		retv = 1;
	}
	return retv;
}
