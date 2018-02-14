#include "tm4c_miscs.h"
#include "timer_task.h"

static struct timer_task workers[MAX_WORKERS];

void task_init(void)
{
	int i;
	struct timer_task *w;

	for (i = 0, w = workers; i < MAX_WORKERS; i++, w++)
		w->task = 0;
}

struct timer_task *task_slot_get(void)
{
	int i;
	for (i = 0; i < MAX_WORKERS; i++)
		if (workers[i].task == 0)
			return workers+i;
	return 0;
}

void task_execute(void)
{
	int i;
	struct timer_task *w;

	for (i = 0, w = workers; i < MAX_WORKERS; i++, w++) {
		if (w->task == 0)
			continue;
		if (time_after(w->tick))
			w->task(w);
	}
}
