#include "tm4c_miscs.h"
#include "tm4c_qei.h"
#include "ssi_display.h"
#include "uart_laser.h"
#include "display_blink.h"

static struct disp_blink db;

struct disp_blink * blink_init(struct qeishot *qs)
{
	db.qs = qs;
	db.count = 0;
	return &db;
}

struct timer_task * blink_activate(struct disp_blink *blk)
{
	struct timer_task *slot;

	blk->count = 16;
	slot = task_slot_setup(blink_display, blk, 5, 0);
	if (slot)
		task_slot_suspend(blk->qs->slot);
	return slot;
}

void blink_display(struct timer_task *slot)
{
	struct disp_blink *blk;

	blk = slot->data;
	if (blk->count == 0) {
		task_slot_remove(slot);
		return;
	}
	if ((blk->count % 2) == 0) {
		ssi_display_shut();
		if ((blk->count % 4) == 0)
			ssi_display_int(laser_distance());
		else if ((blk->count % 4) == 2)
			ssi_display_int(blk->qs->qeipos);
	} else
		ssi_display_show();
	if (--blk->count == 2) {
		blk->count = 0;
		qeipos_align(laser_distance());
		task_slot_remove(slot);
		task_slot_resume(blk->qs->slot);
	}
}
