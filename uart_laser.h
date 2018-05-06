#ifndef UART_LASER_DSCAO__
#define UART_LASER_DSCAO__

#include "timer_task.h"
#include "uart_op.h"

struct laser_beam {
	struct timer_task *slot;
	volatile int dist;
	uint8_t stage, ocnt, csec;
};

struct laser_beam * laser_init(int csec);

static inline int laser_dist(const struct laser_beam *lb)
{
	return lb->dist;
}

static inline int laser_speedup(struct laser_beam *lb, int up)
{
	int lastc;

	lastc = lb->slot->csec;
	if (up > 0)
		lb->slot->csec = 20;
	else
		lb->slot->csec = 2;
	task_slot_immediate(lb->slot);
	return lastc;
}

int laser_quick(struct laser_beam *lb);
int laser_normal(struct laser_beam *lb);

#endif  /* UART_LASER_DSCAO__ */
