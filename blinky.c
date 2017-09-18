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

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/rom.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Blinky (blinky)</h1>
//!
//! A very simple example that blinks the on-board LED using direct register
//! access.
//
//*****************************************************************************
volatile uint32_t ticks;
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
	while(1);
}
#endif

static void blink(int led, int intensity);
//*****************************************************************************
//
// Blink the on-board LED.
//
//*****************************************************************************
int
main(void)
{
	ticks = 1;

	ROM_SysCtlClockSet(SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN|SYSCTL_USE_PLL|SYSCTL_SYSDIV_2_5);

	ROM_SysTickPeriodSet(7999999);
	NVIC_ST_CURRENT_R = 0;
	//
	// Enable the GPIO port that is used for the on-board LED.
	//

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	//
	// Check if the peripheral access is enabled.
	//
	while(!ROM_SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
	{
	}

	//
	// Enable the GPIO pin for the LED (PF3).  Set the direction as output, and
	// enable the GPIO pin for digital function.
	//
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1);

	ROM_IntMasterEnable();
	ROM_SysTickIntEnable();
	ROM_SysTickEnable();
	//
	// Loop forever.
	//
	while(1)
	{
		blink(GPIO_PIN_1, 10); 
		blink(GPIO_PIN_2, 100);
		blink(GPIO_PIN_3, 10);
	}
}

static void blink(int led, int intensity)
{
	//
	// Turn on the LED.
	//
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, led, intensity);

	//
	// Delay for a bit.
	//
	while (ticks < 10)
		__asm__ __volatile__("wfi");
	ticks = 1;

	//
	// Turn off the LED.
	//
	ROM_GPIOPinWrite(GPIO_PORTF_BASE, led, 0x0);

	//
	// Delay for a bit.
	//
	while (ticks < 10)
		__asm__ __volatile__("wfi");
	ticks = 1;
}
