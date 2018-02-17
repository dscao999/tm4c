#include "timer_task.h"
#include "tm4c_miscs.h"

static struct ledlit {
	enum led_type color;
	int lit;
} led = { .lit = 0 };

static void led_shut(struct timer_task *slot)
{
	slot->task = 0;
	tm4c_ledlit(led.color, 0);
	led.lit = 0;
}

void led_blink_task(enum led_type color, int csec)
{
	struct timer_task *lb;

	if (led.lit)
		return;
	led.lit = 1;
	led.color = color;
	lb = task_slot_get();
	lb->data = &led;
	tm4c_ledlit(color, 1);
	lb->tick = tm4c_tick_after(csec);
	lb->task = led_shut;
}
