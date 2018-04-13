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
static struct uart_param *debug_port;
struct uart_param l_port;

static int dist_decode(const char *buf, int len)
{
	const char *db = buf + 2, *dp, *de;
	int dint, dfrac;

	for (dp = db; dp < db + len && *dp != '.'; dp++)
		;
	for (de = dp; de < db + len && *de != 'm'; de++)
		;
	dint = str2num_dec(db, dp - db);
	dfrac = str2num_dec(dp+1, (de - dp) - 1);
	return dint*1000 + dfrac;
}

static int laser_op(const struct lasercmd *cmd)
{
	int len, count;
	static const char *laser_fail = "UART Laser failed, command:  :";

	uart_write_cmd_expect(l_port.port, cmd->cmd, cmd->explen);
	count = 0;
	len = 0;
	do {
		tm4c_delay(2);
		len = uart_read_expect(l_port.port, l_port.buf, UART_BUFSIZ);
		count++;
	} while (len == 0 && count < 20);
	if (len != cmd->explen || memcmp(cmd->cmprsp, l_port.buf, cmd->cmplen)) {
		count = strlen(laser_fail);
		uart_wait_dma(debug_port->port);
		memcpy(debug_port->buf, laser_fail, count);
		debug_port->buf[count-2] = cmd->cmd;
		memcpy(debug_port->buf+count, l_port.buf, len);
		debug_port->buf[count+len] = 0x0d;
		uart_write(debug_port->port, debug_port->buf, count+len+1, 0);
		len = -1;
	}
	return len;
}

static int laser_measure_sync()
{
	int dist, len;

	dist = 0;

	len = laser_op(&l_open);
	if (len == -1)
		return dist;
		
	len = laser_op(&l_sm);
	if (len == -1)
		goto close_out;
	dist = dist_decode(l_port.buf, len);
	uart_wait_dma(debug_port->port);
	len = num2str_dec(dist, debug_port->buf, 16);
	debug_port->buf[len] = 0x0d;
	uart_write(debug_port->port, debug_port->buf, len+1, 0);

	len = laser_op(&l_fm);
	if (len == -1)
		goto close_out;
	dist = dist_decode(l_port.buf, len);
	uart_wait_dma(debug_port->port);
	len = num2str_dec(dist, debug_port->buf, 16);
	debug_port->buf[len] = 0x0d;
	uart_write(debug_port->port, debug_port->buf, len+1, 0);

	len = laser_op(&l_sn);
	if (len == -1)
		goto close_out;
	uart_wait_dma(debug_port->port);
	memcpy(debug_port->buf, l_port.buf, len);
	uart_write(debug_port->port, debug_port->buf, len-1, 0);
close_out:
	laser_op(&l_close);
	return dist;
}

static void laser_measure(struct timer_task *slot)
{
/*	struct laser_beam *lb = slot->data;

	if (uart_op(&port)) {
		lb->dist = str2num_dec(port.buf, port.pos);
		port.pos = 0;
	}*/
}

struct laser_beam * laser_init(int csec, struct uart_param *dport)
{
	uart_open(LUART_PORT);
	l_port.port = LUART_PORT;
	l_port.pos = 0;
	debug_port = dport;
	beam.dist = laser_measure_sync();
	beam.armed = 0;
	beam.slot = task_slot_setup(laser_measure, &beam, csec, 0);
	return &beam;
}
