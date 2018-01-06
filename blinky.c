//*****************************************************************************
//
// blinky.c - Simple example to blink the on-board LED.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include "miscutils.h"
#include "tm4c_miscs.h"
#include "tm4c_gpio.h"
#include "tm4c_uart.h"
#include "tm4c_dma.h"
#include "tm4c_qei.h"
#include "tm4c_ssi.h"
#include "led_display.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Blinky (blinky)</h1>
//!
//! A very simple example that blinks the on-board LED using direct register
//! access.
//
//*****************************************************************************
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
/*#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
	while(1);
}
#endif*/

//*****************************************************************************
//
// Blink the on-board LED.
//
//*****************************************************************************
static const char *RESET = "ReseT";
static const char hello[] = "Initialization Completed!\n";
static char mesg0[80], mesg1[80];

struct uart_param {
	uint16_t port, rem;
	const char *mesg;
	char *buf;
};

static int uart_op(struct uart_param *p)
{
	int count, rlen, echo;

	echo = 0;
	count = uart_read(p->port, p->buf, p->rem, 0);
	for (rlen = 0; rlen < count; rlen++)
		if (*(p->buf+rlen) == 0)
			*(p->buf+rlen) = '\\';
	if (count && (*(p->buf+count-1) == 0x0d || *(p->buf+count-1) == 0x0a)) {
		*(p->buf+count) = 0;
		rlen = strlen(p->mesg);
		if (rlen > 5 && memcmp(p->mesg, RESET, 5) == 0)
			tm4c_reset();
		else
			echo = 1;
	}
	p->buf += count;
	p->rem -= count;
	return echo;
}

static struct uart_param port0, port1;
int main(void)
{
	int qeipos, len;
	uint16_t *ledat;

	tm4c_setup();
	tm4c_dma_enable();
	tm4c_gpio_setup(GPIOA);
	tm4c_gpio_setup(GPIOB);
	tm4c_gpio_setup(GPIOC);

	port0.mesg = mesg0;
	port0.buf = mesg0;
	port0.port = 0;
	port0.rem = sizeof(mesg0) - 1;
	port1.mesg = mesg1;
	port1.buf = mesg1;
	port1.port = 1;
	port1.rem = sizeof(mesg1) - 1;
	tm4c_qei_setup(1, 23, 30000, -30000);
	len = led_display_init(6, 2);
	uart_open(0);
	uart_write(0, hello, strlen(hello), 1);
	uart_open(1);
	uart_write(1, hello, strlen(hello), 1);

	ledat = (uint16_t *)mesg0;
	len = tm4c_ssi_read(0, ledat, len);
	len = bytes2str_hex((const uint8_t *)ledat, len*2, mesg1);
	mesg1[len] = 0x0d;
	uart_write(0, mesg1, len+1, 1);
	uart_write(1, mesg1, len+1, 1);

	while(1)
	{
		if (uart_op(&port0)) {
			len = strlen(mesg0);
			memcpy(mesg0+len-1, "--Echoed!", 9);
			mesg0[len+8] = 0x0d;
			uart_write(1, mesg0, len+9, 0);
			port0.buf = mesg0;
			port0.rem = sizeof(mesg0) - 1;
			qeipos = str2num_dec(mesg0, len - 1);
			if (qeipos != 0)
				led_display_int(qeipos);
			if (memcmp(mesg0, "BlinK", 5) == 0)
				led_blink(10, 3);
			uart_wait_dma(1);
		}
		if (uart_op(&port1)) {
			len = strlen(mesg1);
			memcpy(mesg1+len-1, "--Echoed!", 9);
			mesg1[len+8] = 0x0d;
			uart_write(0, mesg1, len+9, 0);
			port1.buf = mesg1;
			port1.rem = sizeof(mesg1) - 1;
			qeipos = str2num_dec(mesg1, len - 1);
			if (qeipos != 0)
				led_display_int(qeipos);
			if (memcmp(mesg1, "BlinK", 5) == 0)
				led_blink(10, 3);
			uart_wait_dma(0);
		}
	}

	uart_close(0);
	uart_close(1);
	tm4c_reset();
}
