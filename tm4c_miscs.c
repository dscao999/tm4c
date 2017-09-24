#include <stdint.h>
#include <stdbool.h>
#include "tm4c_miscs.h"

extern volatile uint32_t sys_ticks;

void tm4c_ledlit(enum led_type led, int ticks)
{
	uint32_t ledpin;
	int intensity;

	ledpin = GPIO_PIN_1;
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
		while(1)
			;
	}

	ROM_GPIOPinWrite(GPIO_PORTF_BASE, ledpin, intensity);
	tm4c_delay(ticks);
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, ledpin, 0x0);
}
