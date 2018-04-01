#ifndef UART_LASER_DSCAO__
#define UART_LASER_DSCAO__

#include "timer_task.h"

struct laser_beam {
	struct timer_task *slot;
	volatile int dist;
	int8_t armed;
};

struct laser_beam * laser_init(int csec);

static inline int laser_distance(const struct laser_beam *lb)
{
	return lb->dist;
}

static inline int laser_recali(struct laser_beam *lb, int csec)
{
	int lastc;

	lastc = lb->slot->csec;
	lb->slot->csec = csec;
	task_slot_immediate(lb->slot);
	return lastc;
}

#endif  /* UART_LASER_DSCAO__ */
