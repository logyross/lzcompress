#include <stdio.h>

int main()
{
	int a1 = 1;
	int a2 = 2;
	int a3 = 3;
	int all = (((a1 << sizeof(int)) | a2) << sizeof(int)) | a3;
	int all2 = (((a1 << 2) | a2) << 2) | a3;
	fprintf(stderr, "%d\n", all);
	return 0;
}
