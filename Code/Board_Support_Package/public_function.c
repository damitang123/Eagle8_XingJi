#include "stdio.h"

unsigned short	g_aXCHAR[50];
char 	g_aCommonBuffer[200];


unsigned short *Ascii_To_Unicode(unsigned short *dest, unsigned char *src )
{
	unsigned short *dest_backup = dest;
	while(0 != (*dest++ = *src++));
	return dest_backup;
}


/*外部FLASH中的字体是按大端显示的,编译器是以小端模式的*/
unsigned short *Swap_High_Low_Byte(unsigned short *dest, unsigned char *src )
{
	unsigned short i = 0;
	unsigned short *dest_backup = dest;

	while(*src != 0)
	{
		if(*src > 0x7f)
		{
			if(i % 2 != 0)
				*dest++ = *src + (*(src - 1) << 8);
			i++;
			src++;
		}
		else
		{
			*dest++ = *src++;
		}
	}

	*dest = 0x00;

	return dest_backup;
}

/*升序排列*/
void Array_Sort_By_Ascending(unsigned short *a, char num)
{
	unsigned short i, j, temp;
	for(i = 0; i < num - 1; i++)
	{
		for (j = 0; j < num - i - 1; j++)
			if (a[j] > a[j + 1])
			{
				temp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = temp;
			}
	}
}

/*降序排列*/
void Array_Sort_By_Descending(unsigned short *a, char num)
{
	unsigned short i, j, temp;
	for(i = 0; i < num - 1; i++)
	{
		for (j = 0; j < num - i - 1; j++)
			if (a[j] < a[j + 1])
			{
				temp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = temp;
			}
	}
}



