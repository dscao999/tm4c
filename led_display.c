#include <stdint.h>
#include <stdbool.h>
#include "miscutils.h"
#include "tm4c_ssi.h"
#include "tm4c_miscs.h"
#include "led_display.h"

static const char digits[]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};

#define DECODE_REG	0x09
#define INTEN_REG	0x0a
#define SCAN_REG	0x0b
#define SHUT_REG	0x0c
#define TEST_REG	0x0f

#define num_digits	3

static void led_display_clear(void)
{
	int i;
	char cmd[24], *ccmd;

	memset(cmd, 0, 24);
	ccmd = cmd;
	for (i = 0; i < 3; i++) {
		*(ccmd+2) = i + 1;
		*(ccmd+3) = digits[0];
		ccmd += 6;
	}
	tm4c_ssi_write(0, cmd, ccmd - cmd);
	tm4c_ssi_waitdma(0);
}

void led_display_init(void)
{
	char cmd[64];
	int pos;

	tm4c_ssi_setup(0);

	pos = 0;
	memset(cmd, 0, 64);
	cmd[pos+2] = SHUT_REG;
	cmd[pos+3] = 1;
	pos += 6;
	cmd[pos+2] = TEST_REG;
	cmd[pos+3] = 0;
	pos += 6;
	cmd[pos+2] = DECODE_REG;
	cmd[pos+3] = 0;
	pos += 6;
	cmd[pos+2] = SCAN_REG;
	cmd[pos+3] = num_digits - 1;
	pos += 6;
	cmd[pos+2] = INTEN_REG;
	cmd[pos+3] = 3;
	pos += 6;
	cmd[pos+2] = TEST_REG;
	cmd[pos+3] = 1; 
	tm4c_ssi_write(0, cmd, pos+6);
	tm4c_ssi_waitdma(0);
	tm4c_ledlit(BLUE, 20);
/*	cmd[2] = TEST_REG;
	cmd[3] = 0;
	tm4c_ssi_write(0, cmd, 6);

	led_display_clear();*/
}

void led_display_int(int num)
{
	int dpos, dig, neg, cpos;
	char cmd[18];
	uint32_t v;

	memset(cmd, 0, 18);
	neg = 0;
	if (num < 0) {
		v = -num;
		neg = 1;
	} else
		v = num;

	cpos = 0;
	dpos = 1;
	dig = v % 10;
	v /= 10;
	cmd[cpos+2] = dpos;
	cmd[cpos+3] = digits[dig];

	cpos += 6;
	dpos += 1;
	dig = v % 10;
	v /= 10;
	cmd[cpos+2] = dpos;
	cmd[cpos+3] = digits[dig];

	cpos += 6;
	dpos += 1;
	cmd[cpos+2] = dpos;
	if (neg)
		cmd[cpos+3] = 1;
	else {
		dig = v % 10;
		cmd[cpos+3] = digits[dig];
	}
	tm4c_ssi_write(0, cmd, 18);
	tm4c_ssi_waitdma(0);
}
