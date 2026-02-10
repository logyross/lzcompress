#include "lz77.h"

int main()
{
	compress("tests/test4.txt", "tests/test4.txt.lz");
	printf("---\n");
	decompress("tests/test4.txt.lz", "tests/xx");

	/* compress("tests/lz77.c", "tests/lz77.c.2.lz"); */
	/* compress("tests/bigfile.txt", "tests/bigfile.txt.2.lz"); */
	/* compress("tests/bigfile2.txt", "tests/bigfile2.txt.2.lz"); */
	/* printf("---\n"); */


	return 0;
}
