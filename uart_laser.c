#include "miscutils.h"
#include "tm4c_uart.h"
#include "uart_op.h"
#include "uart_laser.h"

#include "led_blink.h"

#define LUART_PORT	1

struct lasercmd {
	int elen;
	char cmd;
};
static const struct lasercmd  lcmds[] = {
	{ 5, 'O'}, { 14, 'D'}, { 14, 'F'}, { 14, 'S'}, { 5, 'C'}
};

static struct laser_beam beam;
static struct uart_param port;

static int laser_measure_sync()
{
	char resp[16];
	int len;

	uart_write_cmd_expect(LUART_PORT, lcmds[0].cmd, lcmds[0].elen);
	len = uart_read_expect(LUART_PORT, resp, 16);
	resp[len] = 0x0d;
	uart_wait_dma(0);
	uart_write(0, resp, len+1, 0);

	uart_write_cmd_expect(LUART_PORT, lcmds[1].cmd, lcmds[1].elen);
	len = uart_read_expect(LUART_PORT, resp, 16);
	resp[len] = 0x0d;
	uart_wait_dma(0);
	uart_write(0, resp, len+1, 0);

	uart_write_cmd_expect(LUART_PORT, lcmds[5].cmd, lcmds[5].elen);
	len = uart_read_expect(LUART_PORT, resp, 16);
	resp[len] = 0x0d;
	uart_wait_dma(0);
	uart_write(0, resp, len+1, 0);

	uart_write_cmd_expect(LUART_PORT, lcmds[3].cmd, lcmds[3].elen);
	len = uart_read_expect(LUART_PORT, resp, 16);
	resp[len] = 0x0d;
	uart_wait_dma(0);
	uart_write(0, resp, len+1, 0);

	return 0;
}

static void laser_measure(struct timer_task *slot)
{
	struct laser_beam *lb = slot->data;

	if (uart_op(&port)) {
		lb->dist = str2num_dec(port.buf, port.pos);
		port.pos = 0;
	}
}

struct laser_beam * laser_init(int csec)
{
	uart_open(LUART_PORT);
	port.port = LUART_PORT;
	port.pos = 0;
	beam.dist = laser_measure_sync();
	beam.armed = 0;
	beam.slot = task_slot_setup(laser_measure, &beam, csec, 0);
	return &beam;
}
