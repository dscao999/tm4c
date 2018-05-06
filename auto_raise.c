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
#include "tm4c_pwm.h"
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
	struct tm4c_pwm *pwm;
	uint8_t in_motion;
	uint8_t gpioc_pin4;
	uint8_t quick;
};

static int motor_init(struct global_control *gc)
{
	return tm4c_pwm_set(gc->pwm, 100, 0);
}

static void motor_start(struct global_control *g_ctrl)
{
	g_ctrl->in_motion = 1;
	tm4c_pwm_start(g_ctrl->pwm, 0);
}

static void motor_stop(struct global_control *g_ctrl)
{
	tm4c_pwm_stop(g_ctrl->pwm, 0);
	g_ctrl->in_motion = 0;
}

struct uart_param dbg_uart;
static int check_key_pressed(struct global_control *g_ctrl)
{
	int pined, retv;

	retv = 0;
	if (g_ctrl->in_motion)
		return retv;
	pined = tm4c_gpio_isrnum(GPIOC, GPIO_PIN_4);
	if (pined == ((g_ctrl->gpioc_pin4 + 1) & 0x0f))
		retv = 1;
	else if (pined == ((g_ctrl->gpioc_pin4 + 2) & 0x0f)) {
		retv = 2;
		g_ctrl->gpioc_pin4 = pined;
	} else
		g_ctrl->gpioc_pin4 = pined;
	return retv;
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
		if (gc->in_motion) {
			if (position_match(gc)) {
				motor_stop(gc);
				blink_taxing(gc->db);
			}
			if (!gc->quick)
				gc->quick = laser_quick(gc->lb);
		} else {
			if (check_key_pressed(gc) == 2) {
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
		if (gc->quick)
			gc->quick = !laser_normal(gc->lb);
	}
}

static struct global_control g_ctrl = {0, 0};

void __attribute__((noreturn)) main(void)
{
	int len;
	uint32_t isr_count;

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

	g_ctrl.pwm = tm4c_pwm_init(0);
	g_ctrl.lb = laser_init(50);
	g_ctrl.qs = qeipos_setup(laser_dist(g_ctrl.lb));
	g_ctrl.db = blink_init();
	g_ctrl.db->l_pos = &g_ctrl.lb->dist;
	g_ctrl.db->q_pos = &g_ctrl.qs->qeipos;
	motor_init(&g_ctrl);
	uart_write(0, hello, strlen(hello), 1);
	while(1) {
		task_execute();
		if (uart_op(&dbg_uart)) {
			len = 0;
			if (memcmp(dbg_uart.buf, RESET, 5) == 0)
				tm4c_reset();
			else if (memcmp(dbg_uart.buf, "BTNxInfo", 8) == 0) {
				memcpy(dbg_uart.buf, "Button Press: ", 14);
				isr_count = tm4c_gpio_isrtimes(GPIOC);
				len = num2str_dec(isr_count, dbg_uart.buf+14, 8);
				dbg_uart.buf[len+14] = 0x0d;
				len += 15;
			} else if (memcmp(dbg_uart.buf, "PWMxInfo", 8) == 0) {
				memcpy(dbg_uart.buf, "PWM Intrs: ", 11);
				isr_count = tm4c_pwm_get_intrs(g_ctrl.pwm, 0);
				len = num2str_dec(isr_count, dbg_uart.buf+11, 8);
				dbg_uart.buf[len+11] = 0x0d;
				len += 12;
			}
			if (len) {
				uart_wait_dma(0);
				uart_write(dbg_uart.port, dbg_uart.buf, len, 0);
			}
			dbg_uart.pos = 0;
		}
		global_task(&g_ctrl);
	}
}
