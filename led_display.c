#include <stdint.h>
#include <stdbool.h>
#include "miscutils.h"
#include "tm4c_uart.h"
#include "modbus/modbus.h"
#include "led_display.h"

static struct modbus cmd = {
	.target = 1,
	.func = 6
};

void led_disp_int(int num)
{
	cmd.regaddr = 0;
	cmd.dat.v16[0] = swap16(num);
	cmd.datlen = 2;
	modbus_set_crc(&cmd);
	uart_write(1, (void *)&cmd, 8);
}

void led_set_ppos(int pos)
{
	cmd.regaddr = swap16(4);
	cmd.dat.v16[0] = swap16(pos);
	cmd.datlen = 2;
	modbus_set_crc(&cmd);
	uart_write(1, (void *)&cmd, 8);
}

void led_set_baud(int baud)
{
	int bv;

	switch(baud) {
	case 1200:
		bv = 0;
		break;
	case 2400:
		bv = 1;
		break;
	case 4800:
		bv = 2;
		break;
	case 9600:
		bv = 3;
		break;
	case 19200:
		bv = 4;
		break;
	case 38400:
		bv = 5;
		break;
	case 57600:
		bv = 6;
		break;
	case 115200:
		bv = 7;
		break;
	default:
		bv = 3;
		break;
	}
	cmd.regaddr = swap16(3);
	cmd.dat.v16[0] = swap16(bv);
	cmd.datlen = 2;
	modbus_set_crc(&cmd);
	uart_write(1, (void *)&cmd, 8);
}
