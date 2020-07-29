#include "platform.h"

#define	SCLK_H			(GPIOA ->BSRR = GPIO_Pin_5)
#define	SCLK_L			(GPIOA ->BRR = GPIO_Pin_5)

#define	MOSI_H			(GPIOA ->BSRR = GPIO_Pin_7)
#define	MOSI_L			(GPIOA ->BRR = GPIO_Pin_7)

#define	MISO_INPUT		GPIOB->IDR & GPIO_Pin_4


static void Spi_Delay(unsigned char n)
{
	volatile char i = n;
	while(i--);
}

void Spi_Sim_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5  | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*********************************************************************************/

unsigned char Spi_Sim_Get_Single_Byte(void)
{
	unsigned char i, nValue = 0;

	SCLK_L;
	Spi_Delay(1);
	for(i = 0; i < 8; i++)
	{
		SCLK_H;
		Spi_Delay(2);
		nValue <<= 1;
		if(MISO_INPUT)
		{
			nValue |= 0x01;
		}
		Spi_Delay(1);
		SCLK_L;
	}
	
	return nValue;
}

void Spi_Sim_Send_Single_Byte(unsigned char nData)
{
	unsigned char i;

	SCLK_L;
	Spi_Delay(1);
	for(i = 0; i < 8; i++)
	{
		if(0x80 & nData)
		{
			MOSI_H;
		}
		else
		{
			MOSI_L;
		}
		Spi_Delay(1);
		SCLK_H;
		Spi_Delay(1);
		SCLK_L;
		Spi_Delay(1);
		nData <<= 1;
	}
}

