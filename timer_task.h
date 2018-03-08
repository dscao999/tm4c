#ifndef TIMER_TASK_DSCAO__
#define TIMER_TASK_DSCAO__
#include <stdint.h>
#include "tm4c_miscs.h"

struct timer_task {
	uint32_t tick;
	void (*task)(struct timer_task *slot);
	void *data;
	uint16_t csec;
	uint16_t hang;
};
#define MAX_WORKERS	5
typedef void (*task_fun)(struct timer_task *slot);

void task_init(void);
struct timer_task * task_slot_setup(task_fun task, void *data, int csec, int delay);

static inline void task_slot_schedule(struct timer_task *slot)
{
	slot->tick = tm4c_tick_after(slot->csec);
}

static inline void task_slot_immediate(struct timer_task *slot)
{
	slot->tick = tm4c_tick_after(0);
}

static inline void task_slot_suspend(struct timer_task *slot)
{
	slot->hang = 1;
}

static inline void task_slot_resume(struct timer_task *slot)
{
	slot->hang = 0;
	slot->tick = tm4c_tick_after(0);
}

int task_slot_remove(struct timer_task *slot);

static inline int task_slot_suspended(struct timer_task *slot)
{
	return slot->hang;
}

void task_execute(void);

#endif /* TIMER_TASK_DSCAO__ */
