
#include "lz77.h"

int compress(FILE *f)
{
	int c = getc(f);
	while (c  != EOF) {
		putchar(c);
		c = getc(f);
	}
	return 0;
}

int decompress(FILE *f)
{
	int c = getc(f);
	while (c != EOF) {
		putchar(c);
		c = getc(f);
	}
	return 0;
}
