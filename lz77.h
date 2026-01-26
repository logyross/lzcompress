#ifndef LZ77_H
#define LZ77_H

#include <stdio.h>

int compress(char *in, char *out);
int decompress(FILE *f);

#endif
