#include "tm4c_miscs.h"
#include "tm4c_qei.h"
#include "ssi_display.h"
#include "qei_position.h"

#define FLASH_WAIT 20

static struct qeishot qeishot;

struct qeishot * qeipos_setup(int dist)
{
	qeishot.qeipos = dist;
	tm4c_qei_setpos(QPORT, dist);
	ssi_display_int(dist);
	qeishot.paused = 0;
	qeishot.varied = 0;
	qeishot.tick = tm4c_tick_after(FLASH_WAIT);

	qeishot.slot = task_slot_get();
	qeishot.slot->task = qeipos_detect;
	qeishot.slot->data = 0;
	qeishot.slot->csec = 2;
	qeishot.slot->tick = tm4c_tick_after(qeishot.slot->csec);

	return &qeishot;
}

void qeipos_detect(struct timer_task *slot)
{
	int qeipos;

	qeipos = tm4c_qei_getpos(QPORT);
	if (qeipos != qeishot.qeipos) {
		ssi_display_int(qeipos);
		qeishot.qeipos = qeipos;
		qeishot.tick = tm4c_tick_after(FLASH_WAIT);
		qeishot.varied = 1;
	} else if (qeishot.varied && time_after(qeishot.tick))
		qeishot.paused = 1;
	slot->tick = tm4c_tick_after(slot->csec);
}
