#ifndef TM4C_UART_DSCAO__
#define TM4C_UART_DSCAO__
#include <stdint.h>

#define UART_BUFSIZ	128
struct uart_port {
	uint32_t base;
	uint16_t oerr;
	uint16_t ferr;
	uint8_t tx_dmach;
	uint8_t rx_dmach;
	volatile uint8_t txdma;
	volatile uint8_t rxhead;
	uint8_t rxtail;
	uint8_t rxbuf[UART_BUFSIZ];
};

void uart_open(int port);
void uart_close(int port);

void uart_write(int port, const char *str, int len, int wait);
int uart_read(int port, char *buf, int len, int wait);
void uart_wait_dma(int port);

void uart0_isr(void);
void uart1_isr(void);

#endif /* TM4C_UART_DSCAO__ */
