#include <stdint.h>
#include "tm4c_uart.h"
#include "uart_op.h"

int uart_op(struct uart_param *p)
{
	int count, i, echo, lenrem;
	char *buf;

	echo = 0;
	buf = uart_param_buf(p);
	lenrem = MAX_BUFLEN - p->pos - 1;
	count = uart_read(p->port, buf, lenrem, 0);
	for (i = 0; i < count; i++)
		if (*(buf+i) == 0)
			*(buf+i) = '\\';
	if (count && (*(buf+count-1) == 0x0d || *(buf+count-1) == 0x0a))
		echo = 1;
	p->pos += count;
	return echo;
}
