#ifndef UART_LASER_DSCAO__
#define UART_LASER_DSCAO__

#include "timer_task.h"

struct laser_beam {
	struct timer_task *slot;
	volatile int dist;
};

void laser_init(void);

int laser_distance(void);
void laser_start(int csec);
void laser_stop(void);

#endif  /* UART_LASER_DSCAO__ */
