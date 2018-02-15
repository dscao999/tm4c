#ifndef QEI_POSITION_DSCAO__
#define QEI_POSITION_DSCAO__

#include "timer_task.h"

#define QPORT   0

struct qeishot {
	struct timer_task *slot;
        int qeipos;
        uint32_t tick;
        volatile int8_t paused;
        volatile int8_t varied;
};

struct qeishot * qeipos_setup(int dist);
void qeipos_detect(struct timer_task *slot);

#endif  /* QEI_POSITION_DSCAO__ */
