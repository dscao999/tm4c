#include <stdint.h>
#include <stdbool.h>
#include "miscutils.h"
#include "tm4c_ssi.h"
#include "led_display.h"

static const char digits[]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};
enum regaddr {DECODE = 0x09, INTEN = 0x0a, SCAN = 0x0b, SHUT = 0x0c, TEST = 0x0f};
static const int16_t num_digits = 3;

static void led_clear(void)
{
	int i, pos;
	char cmd[24], *ccmd;

	memset(cmd, 0, 24);
	ccmd = cmd;
	for (i = 0; i < 3; i++) {
		*(ccmd+2) = i + 1;
		*(ccmd+3) = digits[0];
		tm4c_ssi_write(0, ccmd, 6);
		ccmd += 8;
	}
	tm4c_ssi_waitdma(0);
}

void led_init(void)
{
	char cmd1[8], cmd2[8];

	tm4c_ssi_setup(0);
	memset(cmd1, 0, 8);
	cmd1[2] = SHUT;
	cmd1[3] = 1;
	tm4c_ssi_write(0, cmd1, 6);
	memset(cmd2, 0, 8);
	cmd2[2] = TEST;
	cmd2[3] = 0;
	tm4c_ssi_write(0, cmd2, 6);

	cmd1[2] = DECODE;
	cmd1[3] = 0;
	tm4c_ssi_write(0, cmd1, 6);
	cmd2[2] = SCAN;
	cmd2[3] = num_digits - 1;
	tm4c_ssi_write(0, cmd2, 6);

	cmd1[2] = INTEN;
	cmd1[3] = 3;
	tm4c_ssi_write(0, cmd1, 6);
	cmd2[2] = TEST;
	cmd2[3] = 1;
	tm4c_ssi_write(0, cmd2, 6);
	tm4c_delay(20);

	led_clear();
}

void led_display_int(int num)
{
	int dpos, dig, neg;
	char cmd1[8], cmd2[8];

	memset(cmd1, 0, 8);
	memset(cmd2, 0, 8);
	neg = 0;
	if (num < 0) {
		v = -num;
		neg = 1;
	} else
		v = num;

	dpos = 1;
	dig = v % 10;
	v /= 10;
	cmd1[2] = dpos;
	cmd1[3] = digits[dig];
	tm4c_ssi_write(0, cmd1, 6);

	dpos = 2;
	dig = v % 10;
	v /= 10;
	cmd2[2] = dpos;
	cmd2[3] = digits[dig];
	tm4c_ssi_write(0, cmd2, 6);

	dpos = 3;
	cmd1[2] = dpos;
	if (neg)
		cmd1[3] = 1;
	else {
		dig = v % 10;
		cmd1[3] = digits[dig];
	}
	tm4c_ssi_write(0, cmd1, 6);
	tm4c_ssi_waitdma(0);
}
