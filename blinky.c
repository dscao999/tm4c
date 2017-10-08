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

static const char *RESET = "ReseT";
static char hello[] = "Hello, World! I'm coming soon. This is really wonderful!\n";
static char yell[] = "Hello, World! I'm coming soon. Everything is excellent!\n";
int main(void)
{
	char mesg[128], *buf;
	int count, len, rlen;

	tm4c_setup();
	tm4c_dma_enable();
	uart_open(0);
	tm4c_ledblink(GREEN, 50, 20);
	uart_write(0, hello, strlen(hello));
	uart_write(0, yell, strlen(yell));
	uart_write(0, hello, strlen(hello));
	uart_write(0, yell, strlen(yell));
	buf = mesg;
	len = sizeof(mesg)-1;
	count = 0;
	while(len > 0)
	{
		count = uart_read(0, buf, len, 1);
		for (rlen = 0; rlen < count; rlen++)
			if (*(buf+rlen) == 0)
				*(buf+rlen) = '\\';
		if (count && (*(buf+count-1) == 0x0d || *(buf+count-1) == 0x0a)) {
			*(buf+count) = 0;
			rlen = strlen(mesg);
			if (rlen > 5 && memcmp(mesg, RESET, 5) == 0)
				tm4c_reset();
			uart_write(0, mesg, rlen);
			buf = mesg;
			len = sizeof(mesg)-1;
			count = 0;
		}
		buf += count;
		len -= count;
	}

	uart_close(0);
	tm4c_reset();
}
