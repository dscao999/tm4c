#ifndef BLINKING_DSCAO__
#define BLINKING_DSCAO__
#include "timer_task.h"
#include "qei_position.h"

struct disp_blink {
        struct qeishot *qs;
        volatile uint16_t count;
};

struct disp_blink * blink_init(struct qeishot *qs);
void blink_display(struct timer_task *slot);
struct timer_task * blink_activate(struct disp_blink *blk);

#endif /* BLINKING_DSCAO__ */
