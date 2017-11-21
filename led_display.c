#include <stdint.h>
#include <stdbool.h>
#include "miscutils.h"
#include "tm4c_ssi.h"
#include "tm4c_miscs.h"
#include "led_display.h"

static const char digits[]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};
enum regaddr {DECODE = 0x09, INTEN = 0x0a, SCAN = 0x0b, SHUT = 0x0c, TEST = 0x0f};
static const int16_t num_digits = 3;

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
	char cmd[36];
	int pos;

	tm4c_ssi_setup(0);

	pos = 0;
	memset(cmd, 0, 36);
	cmd[pos+2] = SHUT;
	cmd[pos+3] = 1;
	pos += 6;
	cmd[pos+2] = TEST;
	cmd[pos+3] = 0;
	pos += 6;
	cmd[pos+2] = DECODE;
	cmd[pos+3] = 0;
	pos += 6;
	cmd[pos+2] = SCAN;
	cmd[pos+3] = num_digits - 1;
	pos += 6;
	cmd[pos+2] = INTEN;
	cmd[pos+3] = 3;
	pos += 6;
	cmd[pos+2] = TEST;
	cmd[pos+3] = 1;
	tm4c_ssi_write(0, cmd, 36);
	tm4c_delay(20);

	led_display_clear();
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
