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
// This is the program to control a lifter in garage.
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
#include "ssi_display.h"
#include "uart_laser.h"
#include "blinking.h"

//*****************************************************************************
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

static void start_motor(int start, int stop)
{
}

static struct uart_param port0, port1;

static int check_key_press(void)
{
	return port0.rem < sizeof(mesg0 - 1) || port1.rem < sizeof(mesg1) - 1;
}

static struct qeishot *qs;
static struct disp_blink *db;

int main(void)
{
	int len, len0, motor_on = 0;
	char qeipos_str[16];

	tm4c_gpio_setup(GPIOA);
	tm4c_gpio_setup(GPIOB);
	tm4c_gpio_setup(GPIOC);
	tm4c_gpio_setup(GPIOD);
	tm4c_gpio_setup(GPIOF);
	tm4c_setup();
	task_init();
	tm4c_dma_enable();
	tm4c_qei_setup(0, 0, 999, 0);
	ssi_display_init(3, 2);
	uart_open(0);
	uart_open(1);

	port0.mesg = mesg0;
	port0.buf = mesg0;
	port0.port = 0;
	port0.rem = sizeof(mesg0) - 1;
	port1.mesg = mesg1;
	port1.buf = mesg1;
	port1.port = 1;
	port1.rem = sizeof(mesg1) - 1;
	uart_write(0, hello, strlen(hello), 1);
	uart_write(1, hello, strlen(hello), 1);
	tm4c_ledlit(GREEN, 10);

	qs = qeipos_setup(laser_distance());
	db = blink_init(qs);
	while(1) {
		if (qs->paused) {
			if (qs->qeipos != laser_distance())
				blink_activate();
			qs->paused = 0;
			qs->varied = 0;
		}
		task_execute();
		if (uart_op(&port0)) {
			len = strlen(mesg0);
			memcpy(mesg0+len-1, "--Echoed!", 9);
			mesg0[len+8] = 0x0d;
			uart_write(1, mesg0, len+9, 0);
			len0 = num2str_dec(tm4c_qei_getpos(0), qeipos_str, 14);
			qeipos_str[len0] = 0x0d;
			uart_write(0, qeipos_str, len0+1, 0);
			port0.buf = mesg0;
			port0.rem = sizeof(mesg0) - 1;
		}
		if (uart_op(&port1)) {
			len = strlen(mesg1);
			memcpy(mesg1+len-1, "--Echoed!", 9);
			mesg1[len+8] = 0x0d;
			uart_write(0, mesg1, len+9, 0);
			len0 = num2str_dec(tm4c_qei_getpos(0), qeipos_str, 14);
			qeipos_str[len0] = 0x0d;
			uart_write(1, qeipos_str, len0+1, 0);
			port1.buf = mesg1;
			port1.rem = sizeof(mesg1) - 1;
			uart_wait_dma(0);
			uart_wait_dma(1);
		}
		if (db->count && !motor_on && check_key_press()) {
			motor_on = 1;
			db->count += 600*4;
			start_motor(laser_distance(), tm4c_qei_getpos(0));
		}
		uart_wait_dma(1);
		uart_wait_dma(0);
	}

	uart_close(0);
	uart_close(1);
	tm4c_reset();
}
