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
#include "uart_op.h"
#include "display_blink.h"
#include "led_blink.h"

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

struct global_control {
	int target_pos;
	volatile int cur_pos;
	uint8_t in_motion;
	volatile uint8_t btn_pressed;
};

static void motor_start(struct global_control *g_ctrl)
{
	tm4c_gpio_write(GPIOC, GPIO_PIN_4, 1);
	g_ctrl->in_motion = 1;
}

static struct uart_param debug_port;
static int check_key_press(struct global_control *g_ctrl)
{
	if (g_ctrl->in_motion)
		return 0;
	if (debug_port.pos > 0) {
		g_ctrl->btn_pressed = 1;
		debug_port.pos = 0;
	}
	return g_ctrl->btn_pressed;
}

static struct qeishot *qs;
static struct disp_blink *db;

static struct global_control g_ctrl = {0, 0};

void __attribute__((noreturn)) main(void)
{
	int len0;

	tm4c_gpio_setup(GPIOA, 0, 0, 0);
	tm4c_gpio_setup(GPIOB, 0, 0, 0);
	tm4c_gpio_setup(GPIOC, 0, GPIO_PIN_4, 0);
	tm4c_gpio_setup(GPIOD, 0, 0, 0);
	tm4c_gpio_setup(GPIOF, 0, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1, 0);
	tm4c_setup();
	task_init();
	tm4c_dma_enable();
	tm4c_qei_setup(0, 0, 999, 0);
	ssi_display_init(3, 2);
	uart_open(0);
	laser_init();

	debug_port.port = 0;
	debug_port.pos= 0;
	uart_write(0, hello, strlen(hello), 1);

	qs = qeipos_setup(laser_distance());
	db = blink_init(qs);
	led_blink_task(BLUE, 10);
	while(1) {
		if (qs->paused) {
			if (qs->qeipos != laser_distance())
				blink_activate(db);
			qs->paused = 0;
			qs->varied = 0;
		}
		task_execute();
		if (uart_op(&debug_port)) {
			memcpy(uart_param_buf(&debug_port) - 1, " QEI Position: ", 15);
			debug_port.pos += 14;
			len0 = num2str_dec(tm4c_qei_getpos(QPORT), uart_param_buf(&debug_port), 14);
			debug_port.pos += len0;
			*uart_param_buf(&debug_port) = 0x0d;
			uart_wait_dma(0);
			uart_write(0, debug_port.buf, debug_port.pos+1, 0);
			if (memcmp(debug_port.buf, RESET, 5) == 0) {
				uart_wait_dma(0);
				tm4c_reset();
			}
			debug_port.pos = 0;
		}
		if (db->count && check_key_press(&g_ctrl)) {
			g_ctrl.btn_pressed = 0;
			db->count += 600;
			g_ctrl.target_pos = tm4c_qei_getpos(QPORT);
			g_ctrl.cur_pos = laser_distance();
			motor_start(&g_ctrl);
		}
	}

	uart_close(0);
	uart_close(1);
	tm4c_reset();
}
