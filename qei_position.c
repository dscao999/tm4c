#include "tm4c_miscs.h"
#include "ssi_display.h"
#include "qei_position.h"

#define FLASH_WAIT 20

static struct qeishot qeishot;

static void qeipos_detect(struct timer_task *slot)
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

struct qeishot * qeipos_setup(int dist)
{
	qeipos_align(&qeishot, dist);
	qeishot.paused = 0;
	qeishot.varied = 0;
	qeishot.tick = tm4c_tick_after(FLASH_WAIT);
	qeishot.slot = task_slot_setup(qeipos_detect, &qeishot, 2, 0);
	return &qeishot;
}
