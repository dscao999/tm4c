#include "tm4c_uart.h"
#include "uart_op.h"
#include "uart_laser.h"

#define LUART_PORT	1

static struct laser_beam beam;
static struct uart_param port;

static int laser_beam_echo()
{
	uart_write(LUART_PORT, "Distance now: 0\n", 16, 0);
	return 0;
}

void laser_init(void)
{
	uart_open(LUART_PORT);
	port.port = LUART_PORT;
	port.pos = 0;
	beam.dist = laser_beam_echo();
}

void laser_start(int csec)
{
}

int laser_distance(void)
{
	return beam.dist;
}
