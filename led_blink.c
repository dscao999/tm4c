#include "timer_task.h"
#include "tm4c_miscs.h"

static struct ledlit {
	enum led_type color;
	int lit;
} led = { .lit = 0 };

static void led_shut(struct timer_task *slot)
{
	struct ledlit *lt = slot->data;

	task_slot_remove(slot);
	tm4c_ledlit(lt->color, 0);
	lt->lit = 0;
}

void led_blink_task(enum led_type color, int csec)
{
	if (led.lit)
		return;
	led.color = color;
	tm4c_ledlit(color, 1);
	led.lit = 1;
	task_slot_setup(&led_shut, &led, csec, 1);
}
