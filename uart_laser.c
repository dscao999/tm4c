#include "miscutils.h"
#include "tm4c_uart.h"
#include "uart_op.h"
#include "uart_laser.h"

#include "led_blink.h"

#define LUART_PORT	1

static struct laser_beam beam;
static struct uart_param port;

static int laser_measure_sync()
{
	uart_write(LUART_PORT, "Distance now: 0\n", 16, 0);
	return 0;
}

void laser_init(void)
{
	uart_open(LUART_PORT);
	port.port = LUART_PORT;
	port.pos = 0;
	beam.dist = laser_measure_sync();
	beam.armed = 0;
}

static void laser_measure(struct timer_task *slot)
{
	if (uart_op(&port)) {
		beam.dist = str2num_dec(port.buf, port.pos);
		port.pos = 0;
	}
}

void laser_start(int csec)
{
	beam.slot = task_slot_setup(laser_measure, &beam, csec, 0);
}

void laser_stop(void)
{
	task_slot_remove(beam.slot);
}

int laser_distance(void)
{
	return beam.dist;
}
