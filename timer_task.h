#ifndef TIMER_TASK_DSCAO__
#define TIMER_TASK_DSCAO__
#include <stdint.h>
struct timer_task {
	uint32_t tick;
	int csec;
	void (*task)(struct timer_task *slot);
	void *data;
};
#define MAX_WORKERS	4

void task_init(void);
struct timer_task *task_slot_get(void);

void task_execute(void);

#endif /* TIMER_TASK_DSCAO__ */
