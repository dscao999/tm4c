#ifndef MISCUTILS_DSCAO__
#define MISCUTILS_DSCAO__
#include <stdint.h>

static inline uint8_t hex2char(uint8_t v)
{
	if (v > 9)
		return 'a' + v - 10;
	else
		return '0' + v;
}

static inline int strlen(const char *str)
{
	const char *cstr;
	int i;

	for (i = 0, cstr = str; *cstr != 0; cstr++, i++)
		;
	return i;
}

static inline void hex2ascii(uint32_t v, char *buf)
{
	*(buf+7) = hex2char(v & 0x0f);
	*(buf+6) = hex2char((v>>=4) & 0x0f);
	*(buf+5) = hex2char((v>>=4) & 0x0f);
	*(buf+4) = hex2char((v>>=4) & 0x0f);
	*(buf+3) = hex2char((v>>=4) & 0x0f);
	*(buf+2) = hex2char((v>>=4) & 0x0f);
	*(buf+1) = hex2char((v>>=4) & 0x0f);
	*(buf) = hex2char((v>>=4) & 0x0f);
}

#endif /* MISCUTILS_DSCAO__ */
