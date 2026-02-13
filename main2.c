#include "lz77.h"
#include <string.h>


/* usage:
 * lzip77 input.txt output.lz (compresses to output.lz)
 * lzip77 -u input.lz output (decompresses to output
 */
int main(int argc, char* argv[])
{
	int is_compress = 1;
	if (!strcmp(argv[1], "-u")) {
		printf("LOG --- -u spotted, this is a decompress call\n");
		is_compress = 0;
	}

	if (is_compress && argc != 3) {
		fprintf(stderr, "error, bad input\n");
		return -1;
	}

	if (!is_compress && argc != 4) {
		fprintf(stderr, "error, bad input\n");
		return -1;
	}
	
	if (is_compress)
		compress(argv[1], argv[2]);
	else 
		decompress(argv[2], argv[3]);

	return 0;
}
