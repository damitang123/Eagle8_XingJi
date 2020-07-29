#ifndef _PUBLIC_FUNCTION_H_
#define _PUBLIC_FUNCTION_H_

extern unsigned short	g_aXCHAR[50];
extern char 	g_aCommonBuffer[200];

unsigned short *Ascii_To_Unicode(unsigned short *dest, unsigned char *src );
unsigned short *Swap_High_Low_Byte(unsigned short *dest, unsigned char *src );
void Array_Sort_By_Ascending(unsigned short *a, char num);
void Array_Sort_By_Descending(unsigned short *a, char num);

#endif

