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

static inline uint16_t led_cmd(int addr, int v)
{
	return (addr << 8)|v;
}

void led_display_init(void)
{
	uint16_t cmd[16];
	int pos;

	tm4c_ssi_setup(0);
	tm4c_ledlit(GREEN, 10);

	pos = 0;
	cmd[pos++] = led_cmd(SCAN_REG, 3);
	cmd[pos++] = led_cmd(1, 1);
	cmd[pos++] = led_cmd(2, 2);
	cmd[pos++] = led_cmd(3, 3);
	cmd[pos++] = led_cmd(4, 4);
	cmd[pos++] = led_cmd(SHUT_REG, 0);
	tm4c_ssi_write(0, cmd, pos, 1);

/*	tm4c_ledlit(GREEN, 20);
	pos = 0;
	cmd[pos++] = 0;
	cmd[pos++] = led_cmd(SHUT_REG, 1);
	cmd[pos++] = 0;
	tm4c_ssi_write(0, cmd, pos, 1);*/
}

void led_display_int(int num)
{
	int dig, cpos;
	uint16_t cmd[16];
	uint32_t v;

	if (num < 0)
		v = -num;
	else
		v = num;

	cpos = 0;
	dig = v % 10;
	v /= 10;
	cmd[cpos++] = led_cmd(1, digits[dig]);

	dig = v % 10;
	v /= 10;
	cmd[cpos++] = led_cmd(2, digits[dig]);

	dig = v % 10;
	v /= 10;
	cmd[cpos++] = led_cmd(3, digits[dig]);

	tm4c_ssi_write(0, cmd, cpos, 1);
}
