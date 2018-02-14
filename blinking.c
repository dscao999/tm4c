#include "tm4c_miscs.h"
#include "tm4c_qei.h"
#include "ssi_display.h"
#include "uart_laser.h"
#include "blinking.h"

static struct disp_blink db;

struct disp_blink * blink_init(struct qeishot *qs)
{
	db.qs = qs;
	db.count = 0;
	return &db;
}

struct timer_task * blink_activate(void)
{
	struct timer_task *slot;

	slot = task_slot_get();
	if (slot) {
		db.count = 16;
		slot->task = blink_display;
		slot->csec = 5;
		slot->data = 0;
		slot->tick = tm4c_tick_after(0);
	}
	db.qs->slot->task = 0;
	return slot;
}

void blink_display(struct timer_task *slot)
{
	int dist;

	if (db.count == 0) {
		slot->task = 0;
		return;
	}
	if ((db.count % 2) == 0) {
		ssi_display_shut();
		if ((db.count % 4) == 0)
			ssi_display_int(laser_distance());
		if ((db.count % 4) == 2)
			ssi_display_int(db.qs->qeipos);
	} else
		ssi_display_show();
	if (--db.count == 0) {
		dist = laser_distance();
		tm4c_qei_setpos(QPORT, dist);
		db.qs->qeipos = dist;
		ssi_display_int(dist);
		slot->task = 0;
		db.qs->slot->task = qeipos_detect;
		db.qs->slot->tick = tm4c_tick_after(0);
	}
	slot->tick = tm4c_tick_after(slot->csec);
}
