#include "tm4c_miscs.h"
#include "timer_task.h"

static struct timer_task workers[MAX_WORKERS];
static uint8_t nows = 0;

void task_init(void)
{
	int i;
	struct timer_task *w;

	for (i = 0, w = workers; i < MAX_WORKERS; i++, w++) {
		w->task = 0;
		w->hang = 1;
		w->data = 0;
	}
}

struct timer_task *task_slot_setup(task_fun_p task, void *data, int csec, int delay)
{
	int i;
	struct timer_task *w;

	for (i = 0, w = workers; i < MAX_WORKERS && w->task != 0; i++, w++)
		;
	if (i < MAX_WORKERS) {
		nows++;
		w->task = task;
		w->data = data;
		w->csec = csec;
		if (delay)
			w->tick = tm4c_tick_after(csec);
		else
			w->tick = tm4c_tick_after(0);
		w->hang = 0;
	} else
		w = 0;
	return w;
}

int task_execute(void)
{
	int i, ex;
	struct timer_task *w;

	ex = 0;
	for (i = 0, w = workers; i < MAX_WORKERS; i++, w++) {
		if (w->task == 0 || w->hang)
			continue;
		if (time_arrived(w->tick)) {
			w->task(w);
			task_slot_schedule(w);
			ex++;
		}
	}
	return ex;
}

int task_slot_remove(struct timer_task *slot)
{
	slot->task = 0;
	slot->data = 0;
	slot->hang = 1;
	return --nows;
}
