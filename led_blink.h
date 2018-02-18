#ifndef LED_BLINK_DSCAO__
#define LED_BLINK_DSCAO__
#include "tm4c_miscs.h"

static inline void led_blink_sync(enum led_type color, int csec)
{
	tm4c_ledlit(color, 1);
	tm4c_delay(csec);
	tm4c_ledlit(color, 0);
}

void led_blink_task(enum led_type color, int csec);
#endif /* LED_BLINK_DSCAO__ */

