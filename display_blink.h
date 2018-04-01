#ifndef BLINKING_DSCAO__
#define BLINKING_DSCAO__
#include "timer_task.h"
#include "qei_position.h"
#include "uart_laser.h"

struct disp_blink {
	volatile const int *q_pos, *l_pos;
        volatile uint16_t count;
};

struct disp_blink * blink_init(void);
struct timer_task * blink_activate(struct disp_blink *blk);
static inline int blink_ing(const struct disp_blink *blk)
{
	return blk->count;
}
static inline void blink_enlong(struct disp_blink *blk, int stride)
{
	blk->count += stride;
}
static inline void blink_taxing(struct disp_blink *blk)
{
	blk->count = 8 + (blk->count % 4);
}

#endif /* BLINKING_DSCAO__ */
