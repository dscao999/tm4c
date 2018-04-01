#ifndef UART_OP_DSCAO__
#define UART_OP_DSCAO__

#define MAX_BUFLEN	80
struct uart_param {
	uint16_t port;
	uint16_t pos;
	char buf[MAX_BUFLEN];
};
static inline char *uart_param_buf(struct uart_param *up)
{
	return up->buf + up->pos;
}

int uart_op(struct uart_param *p);

#endif /* UART_OP_DSCAO__ */
