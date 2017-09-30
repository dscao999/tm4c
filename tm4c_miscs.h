#ifndef TM4C_MISC_UTILS_DSCAO__
#define TM4C_MISC_UTILS_DSCAO__
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"

enum led_type {RED, BLUE, GREEN};
extern volatile uint32_t sys_ticks;
extern const uint32_t HZ;
extern const uint32_t PERIOD;
extern const uint32_t MEMADDR;

#define time_before(cur, tmark) \
	((int32_t)cur - (int32_t)tmark < 0)

#define time_after(cur, tmark) \
	((int32_t)cur - (int32_t)tmark > 0)

void tm4c_setup(void);
void tm4c_ledlit(enum led_type led, int ticks);
static inline void tm4c_delay(int ticks)
{
	uint32_t mark;

	mark = sys_ticks + ticks;
	while (time_before(sys_ticks, mark))
		__asm__ __volatile__("wfi");
}
static inline void tm4c_ledblink(enum led_type led, int on, int off)
{
	tm4c_ledlit(led, on);
	tm4c_delay(off);
}
#endif /* TM4C_MISC_UTILS_DSCAO__ */
