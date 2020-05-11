#include <stdio.h>

//Print out Eraseable memory map of Apollo Guidance Computer

// The addresses will be 00000-00377 for erasable bank E0,
//00400-00777 for memory bank E1,
//and so forth, up to 03400-03777 for memory bank E7

int main(void)
{
	int y = 0; //bytes
	int x = 0; //banks
	for (x = 0; x < 010; x++)
	{
		printf("Bank %05o %05o ", x, y);
		y = y + 255;
		printf("%05o\n", y);
		y = y + 1;
	}

	return 0;
}
