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
#include "uart.h"

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

static char hello[] = "Hello, World! I'm coming soon.\n";
int main(void)
{
	char mesg[80], *buf;
	int count, len;

	tm4c_setup();
	tm4c_dma_enable();
	uart_open(&uart0, 0);
	count = 0;
	len = 79;
	buf = mesg;
	tm4c_ledblink(GREEN, 80, 10);
	uart_write(&uart0, hello, strlen(hello));
	while(1)
	{
		tm4c_ledblink(RED, 5, 5);
		count = uart_read(&uart0, buf, len);
		if (count && *(buf+count-1) == 0x0d) {
			*(buf+count) = 0;
			uart_write(&uart0, mesg, strlen(mesg));
			buf = mesg;
			len = 79;
			count = 0;
		}
		buf += count;
		len -= count;
		tm4c_ledblink(BLUE, 5, 5);
	}
	uart_close(&uart0);
}
