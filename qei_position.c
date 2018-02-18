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
	qeishot.slot = task_slot_set(qeipos_detect, &qeishot, 2, 0);
	return &qeishot;
}

void qeipos_detect(struct timer_task *slot)
{
	int qeipos;
	struct qeishot *qs = slot->data;

	qeipos = tm4c_qei_getpos(QPORT);
	if (qeipos != qs->qeipos) {
		ssi_display_int(qeipos);
		qs->qeipos = qeipos;
		qs->varied = 1;
		qs->tick = tm4c_tick_after(FLASH_WAIT);
	} else if (qs->varied && time_after(qs->tick))
		qs->paused = 1;
}
