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
#include "tm4c_qei.h"
#include "tm4c_gpio.h"

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
static const char strled[] = "$001,1.48#";
int main(void)
{
	char mesg[96], *buf, uart1_mesg[64], digit;
	int count, len, rlen, cmdlen;

	tm4c_setup();
	tm4c_dma_enable();
	tm4c_gpio_setup(GPIOF);
	tm4c_gpio_setup(GPIOA);
	tm4c_gpio_setup(GPIOB);
	tm4c_ledblink(GREEN, 10, 5);
	tm4c_gpio_setup(GPIOC);
	tm4c_gpio_setup(GPIOD);
	tm4c_qei_setup(0, 0);
	tm4c_qei_setup(1, 0);
	uart_open(0);
	uart_write(0, hello, strlen(hello));
	uart_open(1);
	cmdlen = strlen(strled);
	memcpy(uart1_mesg, strled, cmdlen);
	uart_write(0, uart1_mesg, cmdlen);
	uart_write(1, uart1_mesg, cmdlen);

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
			if (memcmp(mesg, "PoS", 3) == 0) {
				hex2ascii(tm4c_qei_getpos(0), mesg);
				mesg[8] = '-';
				hex2ascii(gpioc_isr_nums, mesg+9);
				mesg[17] = '\n';
				uart_write(0, mesg, 18);
				hex2ascii(tm4c_qei_getpos(1), mesg+20);
				mesg[28] = '-';
				hex2ascii(gpiod_isr_nums, mesg+29);
				mesg[37] = '\n';
				uart_write(0, mesg+20, 18);
				digit = uart1_mesg[7] + 1;
				if (digit > '9')
					digit = '0';
				uart1_mesg[7] = digit;
				uart_write(1, uart1_mesg, cmdlen);
			} else
				uart_write(0, mesg, rlen);
			buf = mesg;
			len = sizeof(mesg)-1;
			count = 0;
		}
		buf += count;
		len -= count;
		if (tm4c_gpio_intpin(GPIOD, GPIO_PIN_3))
			uart_write(0, "Button Pressed!\n", 16);
	}

	uart_close(0);
	tm4c_reset();
}
