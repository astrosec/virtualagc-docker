#include <stdio.h>

//Print out fixed memory map of Apollo Guidance Computer

//The addresses will be 00000-01777 for memory bank 00, 
//02000-03777 for memory bank 01, and so forth, 
//up to 76000-77777 for memory bank 37. 

int main(void)
{
        int y = 0; //bytes
	int x = 0; //banks
	for (x=0;x<040;x++)
	{
		printf("Bank %05o %05o ",x,y);
                y=y+1023;
                printf("%05o\n", y);
                y=y+1;
	}

return 0;
}
