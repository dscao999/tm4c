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
	struct qeishot *qs;
	struct laser_beam *lb;
	struct disp_blink *db;
	uint8_t in_motion;
	volatile uint8_t btn_pressed;
};

static int motor_running(struct global_control *g_ctrl)
{
	return g_ctrl->in_motion;
}

static void motor_start(struct global_control *g_ctrl)
{
	tm4c_gpio_write(GPIOC, GPIO_PIN_4, 1);
	g_ctrl->in_motion = 1;
	g_ctrl->btn_pressed = 0;
	laser_recali(g_ctrl->lb, 2);
}

static void motor_stop(struct global_control *g_ctrl)
{
	g_ctrl->btn_pressed = 0;
	g_ctrl->in_motion = 0;
	tm4c_gpio_write(GPIOC, GPIO_PIN_4, 0);
	laser_recali(g_ctrl->lb, 20);
}

struct uart_param debug_port;
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

static int position_match(struct global_control *g_ctrl)
{
	return qeipos_position(g_ctrl->qs) == laser_distance(g_ctrl->lb);
}


static struct global_control g_ctrl = {0, 0};

extern struct uart_param l_port;

void __attribute__((noreturn)) main(void)
{
	int8_t blinked = 0;

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
	debug_port.port = 0;
	debug_port.pos= 0;
	uart_write(0, hello, strlen(hello), 1);

	g_ctrl.lb = laser_init(20, &debug_port);
	g_ctrl.qs = qeipos_setup(g_ctrl.dist);
	g_ctrl.db = blink_init();
	g_ctrl.db->l_pos = &g_ctrl.lb->dist;
	g_ctrl.db->q_pos = &g_ctrl.qs->qeipos;
	while(1) {
		task_execute();
		if (qeipos_in_window(g_ctrl.qs)) {
			qeipos_reset_window(g_ctrl.qs);
			if (qeipos_position(g_ctrl.qs) != laser_distance(g_ctrl.lb)) {
				blink_activate(g_ctrl.db);
				qeipos_suspend(g_ctrl.qs);
				blinked = 1;
			}
		}
		if (uart_op(&debug_port)) {
			if (memcmp(debug_port.buf, RESET, 5) == 0) {
				uart_wait(0);
				tm4c_reset();
			}
			debug_port.pos = 0;
		}
		if (blink_ing(g_ctrl.db)) {
			if (motor_running(&g_ctrl)) {
				if (position_match(&g_ctrl)) {
					motor_stop(&g_ctrl);
					blink_taxing(g_ctrl.db);
					blinked = 0;
				}
			} else {
				if (check_key_press(&g_ctrl)) {
					blink_enlong(g_ctrl.db, 600);
					motor_start(&g_ctrl);
				}
			}
		} else {
			if (qeipos_suspended(g_ctrl.qs))
				qeipos_resume(g_ctrl.qs);
			if (g_ctrl.dist != laser_distance(g_ctrl.lb)) {
				g_ctrl.dist = laser_distance(g_ctrl.lb);
				qeipos_align(g_ctrl.qs, g_ctrl.dist);
			}
			if (blinked) {
				blinked = 0;
				qeipos_align(g_ctrl.qs, g_ctrl.dist);
			}
		}
	}
}
