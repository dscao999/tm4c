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
static struct uart_param *d_port;
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
		uart_wait_dma(d_port->port);
		memcpy(d_port->buf, laser_fail, count);
		d_port->buf[count-2] = cmd->cmd;
		memcpy(d_port->buf+count, l_port.buf, len);
		uart_write(d_port->port, d_port->buf, count+len, 0);
		len = -1;
	}
	return len;
}

static int laser_measure_sync()
{
	int dist, len;

	dist = -1;

	len = laser_op(&l_open);
	if (len == -1)
		return dist;
		
	len = laser_op(&l_sm);
	if (len == -1)
		goto close_out;
	dist = dist_decode(l_port.buf, len);
	uart_wait_dma(d_port->port);
	len = num2str_dec(dist, d_port->buf, 16);
	d_port->buf[len] = 0x0d;
	uart_write(d_port->port, d_port->buf, len+1, 0);

	len = laser_op(&l_fm);
	if (len == -1)
		goto close_out;
	dist = dist_decode(l_port.buf, len);
	uart_wait_dma(d_port->port);
	len = num2str_dec(dist, d_port->buf, 16);
	d_port->buf[len] = 0x0d;
	uart_write(d_port->port, d_port->buf, len+1, 0);

	len = laser_op(&l_sn);
	if (len == -1)
		goto close_out;
	uart_wait_dma(d_port->port);
	memcpy(d_port->buf, l_port.buf, len);
	uart_write(d_port->port, d_port->buf, len-1, 0);
close_out:
	laser_op(&l_close);
	return dist;
}

static void laser_measure(struct timer_task *slot)
{
	struct laser_beam *lb = slot->data;
	int len, nt;

	nt = 1;
	switch(lb->stage) {
	case 0:
		uart_write_cmd_expect(l_port.port, l_open.cmd, l_open.explen);
		lb->ocnt = 0;
		lb->stage++;
		break;
	case 1:
		len = uart_read_expect(l_port.port, l_port.buf, 32);
		lb->ocnt++;
		if (len == l_open.explen && memcmp(l_port.buf,
					l_open.cmprsp, l_open.cmplen) == 0)
			lb->stage++;
		break;
	case 2:
		uart_write_cmd_expect(l_port.port, l_sm.cmd, l_sm.explen);
		lb->ocnt = 0;
		lb->stage++;
		break;
	case 3:
		len = uart_read_expect(l_port.port, l_port.buf, 32);
		lb->ocnt++;
		if (len == l_sm.explen && memcmp(l_port.buf,
					l_sm.cmprsp, l_sm.cmplen) == 0) {
			lb->dist = dist_decode(l_port.buf, len);
			lb->stage++;
		}
		break;
	case 4:
		uart_write_cmd_expect(l_port.port, l_sn.cmd, l_sn.explen);
		lb->ocnt = 0;
		lb->stage++;
		break;
	case 5:
		len = uart_read_expect(l_port.port, l_port.buf, 32);
		lb->ocnt++;
		if (len == l_sn.explen && memcmp(l_port.buf,
					l_sn.cmprsp, l_sn.cmplen)) {
			lb->stage++;
			uart_wait_dma(d_port->port);
			memcpy(d_port->buf, l_port.buf, len);
			uart_write(d_port->port, d_port->buf, len, 0);
		}
		break;
	case 6:
		uart_write_cmd_expect(l_port.port, l_close.cmd, l_close.explen);
		lb->ocnt = 0;
		lb->stage++;
		break;
	case 7:
		len = uart_read_expect(l_port.port, l_port.buf, 32);
		lb->ocnt++;
		if (len == l_close.explen && memcmp(l_port.buf, l_close.cmprsp,
					l_close.cmplen))
			lb->stage = 0;
		nt = slot->csec;
		break;
	}
	if (lb->ocnt == 20) {
		task_slot_suspend(slot);
	}
	task_slot_schedule_csec(slot, nt);
}

struct laser_beam * laser_init(int csec, struct uart_param *dport)
{
	uart_open(LUART_PORT);
	l_port.port = LUART_PORT;
	l_port.pos = 0;
	d_port = dport;
	beam.dist = laser_measure_sync();
	beam.stage = 0;
	beam.slot = task_slot_setup(laser_measure, &beam, csec, 0);
	return &beam;
}
