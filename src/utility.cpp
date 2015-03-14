#include <md5.h>


void charHex(unsigned char c, unsigned char &l, unsigned char &h)
{
	l = c & 0x0F;
	h = c >> 4;

	if (l > 9) {
		l = 'A' + l - 10;
	}
	else{
		l = '0' + l;
	}

	if (h > 9) {
		h = 'A' + h - 10;
	}
	else{
		h = '0' + h;
	}
}

std::string md5(const char *data, size_t size)
{
	MD5_CTX ctx = { 0 };
	MD5_Init(&ctx);
	MD5_Update(&ctx, data, size);
	unsigned char result[16];
	MD5_Final(result, &ctx);
	char hex[32];
	for (int i = 0; i < 16; i++)
	{
		unsigned char l, h;
		charHex(result[i], l, h);
		hex[i * 2] = h;
		hex[i * 2 + 1] = l;
	}
	std::string hex_str = std::string(hex, 32);
	return hex_str;
}