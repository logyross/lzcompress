#include "lz77.h"

int main()
{
	compress("tests/smallfile2.txt", "tests/smallfile2.txt.lz");
	printf("---\n");
	decompress("tests/smallfile2.txt.lz", "tests/xx");
	
	return 0;
}
