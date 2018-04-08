#include "miscutils.h"
#include "tm4c_uart.h"
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
static struct uart_param port, *debug_port;

static int dist_decode(const struct uart_param *port)
{
	const char *db = port.buf + 2, *dp, *de;
	int dint, dfrac;

	for (dp = db; dp < db + port.pos && *dp != '.'; dp++)
		;
	for (de = dp; de < db + port.pos && *de != 'm'; de++)
		;
	dint = str2num_dec(db, dp - db);
	dfrac = str2num_dec(dp+1, (de - dp) - 1);
	return dint + dfrac*1000;
}

static int laser_measure_sync()
{
	int dist, msglen, olen;
	char buff[16];
	static const char *laser_fail = "UART Laser failed:  \n";

	dist = 0;
	msglen = strlen(laser_fail);
	memcpy(debug_port.buf, laser_fail, msglen + 1);

	debug_port.buf[msglen-2] = 'O';
	uart_write_cmd_expect(port.port, l_open.cmd, l_open.explen);
	len = uart_read_expect(port.port, port.buf, sizeof(resp));
	if (len != l_open.explen || memcmp(l_open.cmprsp, port.buf, l_open.cmplen)) {
		uart_write(debug_port.port, debug_port.buf, msglen, 1);
		goto exit_out;
	}
		
	debug_port.buf[msglen-2] = 'D';
	uart_write_cmd_expect(port.port, l_sm.cmd, l_sm.explen);
	port.pos = uart_read_expect(port.port, port.buf, sizeof(resp));
	if (len != l_sm.explen || memcmp(l_sm.cmprsp, port.buf, l_sm.cmplen)) {
		uart_write(debug_port.port, debug_port.buf, msglen, 1);
		goto close_out;
	}
	dist = dist_decode(&port);
	olen = num2str_dec(dist, buff, 16);
	buff[olen] = 0x0d;
	uart_write(debug_port.port, debug_port.buf, olen+1, 0);

close_out:
	debug_port.buf[msglen-2] = 'C';
	uart_write_cmd_expect(LUART_PORT, l_close.cmd, l_close.explen);
	len = uart_read_expect(LUART_PORT, resp, sizeof(resp));
	if (len != l_close.explen || memcmp(l_close.cmprsp, resp, l_close.cmplen))
		uart_write(debug_port.port, debug_port.buf, msglen, 1);

exit_out:
	return dist;
}

static void laser_measure(struct timer_task *slot)
{
	struct laser_beam *lb = slot->data;

	if (uart_op(&port)) {
		lb->dist = str2num_dec(port.buf, port.pos);
		port.pos = 0;
	}
}

struct laser_beam * laser_init(int csec, struct uart_param *dport)
{
	uart_open(LUART_PORT);
	port.port = LUART_PORT;
	port.pos = 0;
	debug_port = dport;
	beam.dist = laser_measure_sync();
	beam.armed = 0;
	beam.slot = task_slot_setup(laser_measure, &beam, csec, 0);
	return &beam;
}
