#include "tm4c_miscs.h"
#include "tm4c_gpio.h"

const uint32_t HZ = 80000000;
const uint32_t CYCLES = 50;
const uint32_t MEMADDR = 0x20000000;
volatile uint32_t sys_ticks;
uint32_t cycles;

void tm4c_setup(void)
{
	ROM_SysCtlClockSet(SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN|SYSCTL_USE_PLL|SYSCTL_SYSDIV_2_5);
	ROM_SysTickPeriodSet(HZ/CYCLES-1);
	sys_ticks = 0;
	cycles = CYCLES/10;
	HWREG(NVIC_ST_CURRENT) = 0;
	ROM_IntMasterEnable();
	ROM_SysTickIntEnable();
	ROM_SysTickEnable();
	tm4c_gpio_setup(GPIOF);
	tm4c_ledlit(RED, 5);
}

void tm4c_ledlit(enum led_type led, int ticks)
{
	uint32_t ledpin;
	int intensity;

	intensity = 10;
	switch(led) {
	case RED:
		ledpin = GPIO_PIN_1;
		break;
	case BLUE:
		ledpin = GPIO_PIN_2;
		intensity = 100;
		break;
	case GREEN:
		ledpin = GPIO_PIN_3;
		break;
	default:
		ledpin = GPIO_PIN_1;
		break;
	}

	ROM_GPIOPinWrite(GPIO_PORTF_BASE, ledpin, intensity);
	tm4c_delay(ticks);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, ledpin, 0x0);
}
