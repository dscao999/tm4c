#ifndef TM4C_GPIO_DSCAO__
#define TM4C_GPIO_DSCAO__
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"

struct gpio_port {
	uint32_t base;
	uint32_t sysperip;
	uint32_t mis;
	uint32_t pos;
	uint16_t intr;
	uint16_t err;
	uint16_t dir;
	uint16_t index;
};

void tm4c_gpio_config(int port, uint32_t pos);
void tm4c_gpio_setup(int port, uint32_t pos);

void tm4c_gpio_reset(int port, uint32_t pos);

void gpio0_isr(void);
void gpio1_isr(void);

uint32_t tm4c_gpio_getpos(int port);

#endif /* TM4C_GPIO_DSCAO__ */
