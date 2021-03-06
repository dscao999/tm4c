/*
 *  MAX7219/7221 SSI interface display driver
 */
#include <stdint.h>
#include <stdbool.h>
#include "miscutils.h"
#include "tm4c_ssi.h"
#include "tm4c_miscs.h"
#include "ssi_display.h"

static const unsigned char digits[]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};
static struct leddisp {
	int curnum;
	uint8_t maxled;
	uint8_t numled;
	uint8_t popos;
} leddat = { .maxled = 3, .numled = 3, .popos = 2 };

#define DECODE_REG	0x09
#define INTEN_REG	0x0a
#define SCAN_REG	0x0b
#define SHUT_REG	0x0c
#define TEST_REG	0x0f

static inline uint16_t led_cmd(uint8_t addr, uint8_t v)
{
	return (addr << 8)|v;
}

static inline uint8_t posidx(int i)
{
	return leddat.numled - i;
}

static void led_display(void)
{
	int cpos, neg, i;
	uint16_t cmd[16];
	uint32_t v, rem;

	neg = 0;
	if (leddat.curnum < 0) {
		v = -leddat.curnum;
		neg = 1;
	} else
		v = leddat.curnum;

	cpos = 0;
	for (i = 0; i < leddat.popos; i++) {
		rem = v % 10;
		v /= 10;
		cmd[cpos++] = led_cmd(posidx(i), digits[rem]);
	}
	rem = v % 10;
	v /= 10;
	cmd[cpos++] = led_cmd(posidx(i), digits[rem]|0x80);
	for (i++; i < leddat.numled && v != 0; i++) {
		rem = v % 10;
		v /= 10;
		cmd[cpos++] = led_cmd(posidx(i), digits[rem]);
	}
	if (neg  && i < leddat.maxled)
		cmd[cpos++] = led_cmd(posidx(i++), 1);
	for (; i < leddat.numled; i++)
		cmd[cpos++] = led_cmd(posidx(i), 0);
	tm4c_ssi_write(0, cmd, cpos, 1);
}

void ssi_display_init(int numdisp, int popos)
{
	uint16_t cmd[16];
	int pos, i;

	tm4c_ssi_setup(0);
	leddat.numled = numdisp;
	if (leddat.numled > 6)
		leddat.numled = 6;
	leddat.popos = popos;
	if (leddat.popos > 5)
		leddat.popos = 5;


	pos = 0;
	cmd[pos++] = led_cmd(SHUT_REG, 0);
	cmd[pos++] = led_cmd(SCAN_REG, leddat.maxled - 1);
	cmd[pos++] = led_cmd(DECODE_REG, 0);
	for (i = 0; i < leddat.maxled; i++)
		cmd[pos++] = led_cmd(i+1, 0);
	tm4c_ssi_write(0, cmd, pos, 1);

	pos = 0;
	cmd[pos++] = led_cmd(TEST_REG, 1);
	tm4c_ssi_write_sync(0, cmd, pos);
	tm4c_delay(10);
	pos = 0;
	cmd[pos++] = led_cmd(TEST_REG, 0);
	cmd[pos++] = led_cmd(SHUT_REG, 1);
	tm4c_ssi_write_sync(0, cmd, pos);

	leddat.curnum = 0;
	led_display();
}

void ssi_display_int(int num)
{
	leddat.curnum = num;
	led_display();
}

void ssi_display_shut(void)
{
	uint16_t cmd[4];
	int pos = 0;

	cmd[pos++] = led_cmd(SHUT_REG, 0);
	tm4c_ssi_write_sync(0, cmd, pos);

}

void ssi_display_show(void)
{
	uint16_t cmd[4];
	int pos = 0;

	cmd[pos++] = led_cmd(SHUT_REG, 1);
	tm4c_ssi_write_sync(0, cmd, pos);

}

int ssi_display_get(void)
{
	return leddat.curnum;
}

void ssi_display_blink(int csec, int n, int tmpv)
{
	int i, oldv, step;

	step = 0;
	oldv = leddat.curnum;
	if (tmpv == 0)
		tmpv = oldv;
	i = 0;
	do {
		ssi_display_shut();
		tm4c_delay(csec);
		if (step++ % 2 == 0) {
			leddat.curnum = tmpv;
		} else
			leddat.curnum = oldv;
		led_display();
		ssi_display_show();
		tm4c_delay(csec);
	} while (++i < n);
	if (leddat.curnum != oldv) {
		leddat.curnum = oldv;
		led_display();
	}
}
