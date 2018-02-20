#ifndef QEI_POSITION_DSCAO__
#define QEI_POSITION_DSCAO__

#include "tm4c_qei.h"
#include "timer_task.h"

#define QPORT   0

struct qeishot {
	struct timer_task *slot;
        volatile int qeipos;
        uint32_t tick;
        volatile int8_t paused;
        volatile int8_t varied;
};

static inline int qeipos_position(struct qeishot *qs)
{
	return qs->qeipos;
}

static inline int qeipos_in_window(struct qeishot *qs)
{
	return qs->paused;
}

static inline void qeipos_reset_window(struct qeishot *qs)
{
	qs->paused = 0;
	qs->varied = 0;
}

static inline void qeipos_suspend(struct qeishot *qs)
{
	task_slot_suspend(qs->slot);
}

static inline void qeipos_resume(struct qeishot *qs)
{
	task_slot_resume(qs->slot);
}

static inline int qeipos_suspended(struct qeishot *qs)
{
	return task_slot_suspended(qs->slot);
}

struct qeishot * qeipos_setup(int dist);

static inline void qeipos_align(struct qeishot *qs, int dist)
{       
        qs->qeipos = dist;
        tm4c_qei_setpos(QPORT, dist);
        ssi_display_int(dist);
}

#endif  /* QEI_POSITION_DSCAO__ */
