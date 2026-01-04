#include <stdio.h>
#include <stdlib.h>

#include "lz77.h"

int main(int argc, char *argv[])
{
	if (argc <= 1) {
		fprintf(stderr, "usage: lzcompress FILE");
		return 1;
	}
	FILE *f = fopen(argv[1], "r");

	if (f == NULL) {
		fprintf(stderr, "failed to open file %s\n", argv[1]);
		return 1;
	}

	compress(f);
	fclose(f);
	return 0;
}
