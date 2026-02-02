#include "lz77.h"

int main()
{
	compress("test.txt", "x");
	decompress("x", "xx");
	
	return 0;
}
