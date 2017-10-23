#ifndef MODBUS_DSCAO__
#define MODBUS_DSCAO__
#include <stdint.h>
#include "../miscutils.h"
#include "../driverlib/rom.h"

struct modbus {
	uint8_t target;
	uint8_t func;
	uint16_t regaddr;
	union data_union {
		uint8_t buf[16];
		uint16_t v16[8];
		uint32_t v32[4];
	} dat;
	uint16_t datlen;
	uint16_t crc16;
};

static inline void modbus_set_crc(struct modbus *mb)
{
	int len;

	len = mb->datlen;
	mb->crc16 = ROM_Crc16(0xffff, (uint8_t *)mb, len + 4);
	memcpy(mb->dat.buf+len, &mb->crc16, 2);
}

#endif /* MODBUS_DSCAO__ */
