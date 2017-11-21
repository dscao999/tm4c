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
#include "tm4c_dma.h"
#include "tm4c_uart.h"
#include "tm4c_qei.h"
#include "tm4c_gpio.h"
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
int main(void)
{
	char mesg[96], *buf, mesg1[12];
	uint32_t ticks;
	int16_t tleap, qeipos, prev_qeipos;
	int8_t count, len, rlen;

	tm4c_setup();
	tm4c_dma_enable();
	tm4c_gpio_setup(GPIOA);
	tm4c_gpio_setup(GPIOB);
	tm4c_gpio_setup(GPIOC);
	tm4c_gpio_setup(GPIOD);
	tm4c_qei_setup(0, 0, 999, 0);
	uart_open(0);
	uart_write(0, hello, strlen(hello));
	uart_open(1);
	tm4c_ledlit(GREEN, 10);

	tleap = csec2tick(20);
	tm4c_qei_velconf(0, HZ / 20);
	buf = mesg;
	len = sizeof(mesg)-1;
	count = 0;
	ticks = sys_ticks;
	prev_qeipos = 0;
	tm4c_delay(10);
	led_display_init();
	while(1)
	{
		count = uart_read(0, buf, len, 0);
		for (rlen = 0; rlen < count; rlen++)
			if (*(buf+rlen) == 0)
				*(buf+rlen) = '\\';
		if (count && (*(buf+count-1) == 0x0d || *(buf+count-1) == 0x0a)) {
			*(buf+count) = 0;
			rlen = strlen(mesg);
			if (rlen > 5 && memcmp(mesg, RESET, 5) == 0)
				tm4c_reset();
			else
				uart_write(0, mesg, rlen);
			buf = mesg;
			len = sizeof(mesg)-1;
			count = 0;
		}
		if (sys_ticks - ticks >= tleap) {
			qeipos = tm4c_qei_getpos(0);
			if (qeipos != prev_qeipos) {
				if (!tm4c_qei_velproc(0))
					tm4c_qei_velstart(0);
				else {
					rlen = num2str_hex(tm4c_qei_velget(0), mesg1);
					mesg1[rlen] = '\n';
					uart_write(0, mesg1, rlen+1);
				}
				led_display_int(qeipos);
				prev_qeipos = qeipos;
			} else
				tm4c_qei_velstop(0);
			led_display_int(qeipos);
			ticks = sys_ticks;
		}
		buf += count;
		len -= count;
		if (tm4c_gpio_intpin(GPIOD, GPIO_PIN_3))
			uart_write(0, "Button Pressed!\n", 16);
		if (len <= 0) {
			buf = mesg;
			len = sizeof(mesg)-1;
			count = 0;
		}
	}

	uart_close(0);
	tm4c_reset();
}
