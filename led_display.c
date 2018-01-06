#include <stdint.h>
#include <stdbool.h>
#include "miscutils.h"
#include "tm4c_ssi.h"
#include "tm4c_miscs.h"
#include "led_display.h"

static const unsigned char digits[]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};
static struct leddisp {
	uint16_t curnum;
	uint8_t maxled;
	uint8_t numled;
	uint8_t popos;
} leddat = { .maxled = 8, .numled = 6, .popos = 2 };

#define DECODE_REG	0x09
#define INTEN_REG	0x0a
#define SCAN_REG	0x0b
#define SHUT_REG	0x0c
#define TEST_REG	0x0f

static inline uint16_t led_cmd(uint8_t addr, uint8_t v)
{
	return (addr << 8)|v;
}

static int led_display(void)
{
	int cpos, neg, i;
	uint16_t cmd[16];
	uint32_t v, rem;

	neg = 1;
	if (leddat.curnum < 0) {
		v = -leddat.curnum;
		neg = -1;
	} else
		v = leddat.curnum;

	cpos = 0;
	for (i = 0; i < leddat.popos; i++) {
		rem = v % 10;
		v /= 10;
		cmd[cpos++] = led_cmd(i+1, digits[rem]);
	}
	rem = v % 10;
	v /= 10;
	cmd[cpos++] = led_cmd(i+1, digits[rem]|0x80);
	for (i++; i < leddat.numled && v != 0; i++) {
		rem = v % 10;
		v /= 10;
		cmd[cpos++] = led_cmd(i+1, digits[rem]);
	}
	if (neg < 0 && i < leddat.maxled)
		cmd[cpos++] = led_cmd(i+1, 1);
	for (i++; i < leddat.numled; i++)
		cmd[cpos++] = led_cmd(i+1, 0);

	tm4c_ssi_write(0, cmd, cpos, 1);
	return cpos;
}

int led_display_init(int numdisp, int popos)
{
	uint16_t cmd[16];
	int pos, i;

	tm4c_ssi_setup(0);
	tm4c_ledlit(GREEN, 10);
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
	tm4c_ssi_write_sync(0, cmd, pos);

	pos = 0;
	cmd[pos++] = led_cmd(TEST_REG, 1);
	tm4c_ssi_write_sync(0, cmd, pos);
	tm4c_delay(10);
	pos = 0;
	cmd[pos++] = led_cmd(TEST_REG, 0);
	cmd[pos++] = led_cmd(SHUT_REG, 1);
	tm4c_ssi_write_sync(0, cmd, pos);
	tm4c_ledlit(RED, 10);

	leddat.curnum = 0;
	pos = led_display();
	return pos;
}

int led_display_int(int num)
{
	leddat.curnum = num;
	return led_display();
}
