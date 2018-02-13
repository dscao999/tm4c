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
#include "ssi_display.h"

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
struct timer_task {
	uint32_t tick;
	int csec;
	void (*task)(struct timer_task *slot);
	void *data;
};
#define MAX_WORKERS	4
static struct timer_task workers[MAX_WORKERS];

static inline struct timer_task *task_slot(void)
{
	int i;
	for (i = 0; i < MAX_WORKERS; i++)
		if (workers[i].task == 0)
			return workers+i;
	return 0;
}

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

static void display_position(struct timer_task *slot)
{
	int *pos = slot->data, qeipos;

	qeipos = tm4c_qei_getpos(0);
	if (qeipos != *pos) {
		ssi_display_int(qeipos);
		*pos = qeipos;
	}
	if (slot->csec != 0)
		slot->tick = tm4c_tick_after(slot->csec);
	else
		slot->task = 0;
}

struct dispblink {
	uint16_t altnum;
	uint16_t count;
};
static struct dispblink dblink = { .count = 0, .altnum = 0 };

static void display_blink(struct timer_task *slot)
{
	struct dispblink *bl = slot->data;

	if (bl->count == 0) {
		slot->task = 0;
		return;
	}
	if ((bl->count % 2) == 0)
		ssi_display_shut();
	else
		ssi_display_show();
	bl->count--;
	if (bl->count == 0)
		slot->task = 0;

	if (slot->csec != 0)
		slot->tick = tm4c_tick_after(slot->csec);
	else
		slot->task = 0;
}

static struct uart_param port0, port1;
int main(void)
{
	int qeipos, len, len0, i;
	char qeipos_str[16];
	struct timer_task *slot;

	tm4c_setup();
	for (i = 0; i < MAX_WORKERS; i++)
		workers[i].task = 0;

	tm4c_dma_enable();
	tm4c_gpio_setup(GPIOA);
	tm4c_gpio_setup(GPIOB);
	tm4c_gpio_setup(GPIOC);
	tm4c_gpio_setup(GPIOD);

	port0.mesg = mesg0;
	port0.buf = mesg0;
	port0.port = 0;
	port0.rem = sizeof(mesg0) - 1;
	port1.mesg = mesg1;
	port1.buf = mesg1;
	port1.port = 1;
	port1.rem = sizeof(mesg1) - 1;
	tm4c_qei_setup(0, 0, 999, 0);
	len = ssi_display_init(3, 2);
	uart_open(0);
	uart_write(0, hello, strlen(hello), 1);
	uart_open(1);
	uart_write(1, hello, strlen(hello), 1);
	tm4c_ledlit(RED, 10);
	tm4c_ledlit(GREEN, 10);

	qeipos = tm4c_qei_getpos(0);
	ssi_display_int(qeipos);
	workers[0].task = display_position;
	workers[0].data = &qeipos;
	workers[0].csec = 2;
	workers[0].tick = tm4c_tick_after(workers[0].csec);
	while(1) {
		for (i = 0; i < MAX_WORKERS; i++) {
			if (workers[i].task == 0)
				continue;
			if (time_after(workers[i].tick))
				workers[i].task(workers+i);
		}
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
			if (memcmp(mesg0, "BlinK", 5) == 0 && dblink.count == 0) {
				slot = task_slot();
				if (slot) {
					dblink.altnum = 101;
					dblink.count = 6;
					slot->task = display_blink;
					slot->csec = 10;
					slot->data = &dblink;
					slot->tick = tm4c_tick_after(0);
				}
			}
			uart_wait_dma(1);
			uart_wait_dma(0);
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
			if (memcmp(mesg1, "BlinK", 5) == 0 && dblink.count == 0) {
				slot = task_slot();
				if (slot) {
					dblink.altnum = 102;
					dblink.count = 6;
					slot->task = display_blink;
					slot->csec = 10;
					slot->data = &dblink;
					slot->tick = tm4c_tick_after(0);
				}
			}
			uart_wait_dma(0);
			uart_wait_dma(1);
		}
	}

	uart_close(0);
	uart_close(1);
	tm4c_reset();
}
