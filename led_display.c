#include <stdint.h>
#include <stdbool.h>
#include "miscutils.h"
#include "tm4c_ssi.h"
#include "tm4c_miscs.h"
#include "led_display.h"

static const unsigned char digits[]={0x7e,0x30,0x6d,0x79,0x33,0x5b,0x5f,0x70,0x7f,0x7b};
static struct leddisp {
	uint16_t curnum;
	uint8_t numled;
	uint8_t popos;
} leddat;

#define DECODE_REG	0x09
#define INTEN_REG	0x0a
#define SCAN_REG	0x0b
#define SHUT_REG	0x0c
#define TEST_REG	0x0f

static inline uint16_t led_cmd(int addr, uint8_t v)
{
	return (addr << 8)|v;
}

void led_display_init(int numdisp, int popos)
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
	cmd[pos++] = led_cmd(SCAN_REG, leddat.numled - 1);
	for (i = 0; i < leddat.popos; i++)
		cmd[pos++] = led_cmd(i+1, digits[0]);
	cmd[pos++] = led_cmd(i+1, digits[0]|0x80);
	for (i++; i < leddat.numled; i++)
		cmd[pos++] = led_cmd(i+1, 0);
	cmd[pos++] = led_cmd(SHUT_REG, 1);
	tm4c_ssi_write(0, cmd, pos, 1);
}

static void led_display(void)
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
	if (neg < 0 && i < leddat.numled)
		cmd[cpos++] = led_cmd(i+1, 1);
	for (i++; i < leddat.numled; i++)
		cmd[cpos++] = led_cmd(i+1, 0);

	tm4c_ssi_write(0, cmd, cpos, 1);
}

void led_display_int(int num)
{
	leddat.curnum = num;
	led_display();
}
