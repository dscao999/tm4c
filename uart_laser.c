#include "miscutils.h"
#include "tm4c_uart.h"
#include "uart_op.h"
#include "uart_laser.h"

#include "led_blink.h"

#define LUART_PORT	1

struct lasercmd {
	uint8_t explen, cmplen;
	char cmd;
	char cmprsp[16];
};
static const struct lasercmd  l_open = {7, 5, 'O', "O,OK!"},
	l_sm = {16, 2, 'D', "D:"},
	l_fm = {16, 2, 'F', "F:"},
	l_sn = {16, 2, 'S', "S:"},
	l_close = {7, 5, 'C', "C,OK!"};


static struct laser_beam beam;
static struct uart_param port;

static int laser_measure_sync()
{
/*	char resp[24];
	int len;

	uart_write_cmd_expect(LUART_PORT, l_open.cmd, l_open.explen);
	len = uart_read_expect(LUART_PORT, resp, sizeof(resp));
	if (len != l_open.explen || memcmp(l_open.cmprsp, resp, l_open.cmplen))
		
	uart_write_cmd_expect(LUART_PORT, l_sm.cmd, l_sm.explen);
	len = uart_read_expect(LUART_PORT, resp, sizeof(resp));
	if (len != l_sm.explen || memcmp(l_sm.cmprsp, resp, l_sm.cmplen))

	uart_write_cmd_expect(LUART_PORT, lcmds[5].cmd, lcmds[5].elen);
	len = uart_read_expect(LUART_PORT, resp, 16);
	uart_wait_dma(0);
	uart_write(0, resp, len+1, 0);

	uart_write_cmd_expect(LUART_PORT, l_close.cmd, l_close.explen);
	len = uart_read_expect(LUART_PORT, resp, sizeof(resp));
	if (len != l_close.explen || memcmp(l_close.cmprsp, resp, l_close.cmplen)) */

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
