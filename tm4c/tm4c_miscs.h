#ifndef TM4C_MISC_UTILS_DSCAO__
#define TM4C_MISC_UTILS_DSCAO__
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ints.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/gpio.h"
#include "driverlib/rom.h"

enum led_type {RED, BLUE, GREEN};
extern volatile uint32_t sys_ticks;
extern const uint32_t HZ;
extern const uint32_t CYCLES;
extern const uint32_t MEMADDR;
extern uint32_t cycles;

#define time_before(tmark) \
	((int32_t)sys_ticks - (int32_t)tmark < 0)

#define time_after(tmark) \
	((int32_t)sys_ticks - (int32_t)tmark > 0)

#define time_at(tmark) \
	((int32_t)sys_ticks - (int32_t)tmark == 0)

#define time_arrived(tmark) \
	((int32_t)sys_ticks - (int32_t)tmark >= 0)

static inline int csec2tick(int csecs)
{
	return cycles*csecs;
}

static inline void tm4c_memsync(void)
{
	__asm__ __volatile__("dsb");
}

static uint32_t tm4c_tick_after(int csecs)
{
	return sys_ticks + csec2tick(csecs);
}

void tm4c_setup(void);
void tm4c_ledlit(enum led_type led, int onoff);
static inline void tm4c_delay(int csecs)
{
	uint32_t mark;

	mark = tm4c_tick_after(csecs);
	while (time_before(mark))
		__asm__ __volatile__("wfi");
}

static inline void tm4c_waitint(void)
{
	__asm__ __volatile__("wfi");
}
static inline void tm4c_reset(void)
{
	HWREG(NVIC_APINT) = NVIC_APINT_VECTKEY|NVIC_APINT_SYSRESETREQ;
	while (1)
		;
}
#endif /* TM4C_MISC_UTILS_DSCAO__ */
