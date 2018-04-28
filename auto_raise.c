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
	laser_speedup(g_ctrl->lb, 1);
}

static void motor_stop(struct global_control *g_ctrl)
{
	g_ctrl->btn_pressed = 0;
	g_ctrl->in_motion = 0;
	tm4c_gpio_write(GPIOC, GPIO_PIN_4, 0);
	laser_speedup(g_ctrl->lb, -1);
}

struct uart_param dbg_uart;
static int check_key_pressed(struct global_control *g_ctrl)
{
	if (g_ctrl->in_motion)
		return 0;
	if (dbg_uart.pos > 0) {
		g_ctrl->btn_pressed = 1;
		dbg_uart.pos = 0;
	}
	return g_ctrl->btn_pressed;
}

static int position_match(struct global_control *g_ctrl)
{
	return qeipos_pos(g_ctrl->qs) == laser_dist(g_ctrl->lb);
}

static void global_task(struct global_control *gc)
{
	if (qeipos_paused(gc->qs)) {
		if (qeipos_pos(gc->qs) != laser_dist(gc->lb)) {
			blink_activate(gc->db);
			qeipos_suspend(gc->qs);
		}
		qeipos_dect_reset(gc->qs);
	}
	if (blink_ing(gc->db)) {
		if (motor_running(gc)) {
			if (position_match(gc)) {
				motor_stop(gc);
				blink_taxing(gc->db);
			}
		} else {
			if (check_key_pressed(gc)) {
				blink_enlong(gc->db, 600);
				motor_start(gc);
			}
		}
	} else {
		if (qeipos_suspended(gc->qs))
			qeipos_resume(gc->qs);
		if (!qeipos_varied(gc->qs)) {
			if (qeipos_pos(gc->qs) != laser_dist(gc->lb))
				qeipos_align(gc->qs, laser_dist(gc->lb));
			if (ssi_display_get() != laser_dist(gc->lb))
				ssi_display_int(laser_dist(gc->lb));
		} else if (ssi_display_get() != qeipos_pos(gc->qs))
			ssi_display_int(qeipos_pos(gc->qs));
	}
}

static struct global_control g_ctrl = {0, 0};

void __attribute__((noreturn)) main(void)
{
	int len, isr_count;

	tm4c_gpio_setup(GPIOA, 0, 0, 0);
	tm4c_gpio_setup(GPIOB, 0, 0, 0);
	tm4c_gpio_setup(GPIOC, GPIO_PIN_4, 0, GPIO_PIN_4);
	tm4c_gpio_setup(GPIOD, 0, 0, 0);
	tm4c_gpio_setup(GPIOF, 0, GPIO_PIN_3|GPIO_PIN_2|GPIO_PIN_1, 0);
	tm4c_setup();
	task_init();
	tm4c_dma_enable();
	tm4c_qei_setup(0, 0, 999, 0);
	ssi_display_init(3, 2);
	uart_open(0);
	dbg_uart.port = 0;
	dbg_uart.pos= 0;
	uart_write(0, hello, strlen(hello), 1);

	g_ctrl.lb = laser_init(50);
	g_ctrl.qs = qeipos_setup(laser_dist(g_ctrl.lb));
	g_ctrl.db = blink_init();
	g_ctrl.db->l_pos = &g_ctrl.lb->dist;
	g_ctrl.db->q_pos = &g_ctrl.qs->qeipos;
	while(1) {
		task_execute();
		if (uart_op(&dbg_uart)) {
			if (memcmp(dbg_uart.buf, RESET, 5) == 0) {
				uart_wait(0);
				tm4c_reset();
			}
			if (memcmp(dbg_uart.buf, "BuPr", 4) == 0) {
				memcpy(dbg_uart.buf, "Button Press: ", 14);
				isr_count = tm4c_gpio_isrtimes(GPIOC);
				len = num2str_dec(isr_count, dbg_uart.buf+14, 8);
				dbg_uart.buf[len+14] = 0x0d;
				uart_write(dbg_uart.port, dbg_uart.buf, len+15, 0);
			}
			dbg_uart.pos = 0;
		}
		global_task(&g_ctrl);
	}
}
