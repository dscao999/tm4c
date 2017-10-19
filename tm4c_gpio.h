#ifndef TM4C_GPIO_DSCAO__
#define TM4C_GPIO_DSCAO__
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"

enum GPIOPORT { GPIOA = 0, GPIOB = 1, GPIOC = 2, GPIOD = 3, GPIOE = 4, GPIOF = 5};
struct gpio_port {
	uint32_t base;
	uint32_t mis;
};


void gpioc_isr(void);
void gpiod_isr(void);

void tm4c_gpio_setup(enum GPIOPORT port);
int tm4c_gpio_intpin(enum GPIOPORT port, uint32_t pins);

extern uint32_t gpioc_isr_nums;
extern uint32_t gpiod_isr_nums;
#endif /* TM4C_GPIO_DSCAO__ */
