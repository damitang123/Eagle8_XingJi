#include "platform.h"
#include "voice.h"
#include "delay.h"

#if 0

#define O_CLK_1 GPIOG->BSRRL=GPIO_Pin_7;
#define O_CLK_0 GPIOG->BSRRH=GPIO_Pin_7;
#define O_DATA_1 GPIOG->BSRRL=GPIO_Pin_6;
#define O_DATA_0 GPIOG->BSRRH=GPIO_Pin_6;
#define O_POWEN_ON GPIOG->BSRRL=GPIO_Pin_15;
#define O_POWEN_OFF GPIOG->BSRRH=GPIO_Pin_15;

bool s_bEnableVoceBufferMode;

void Voc_Init(void)
{
	s_bEnableVoceBufferMode = false;
}

static void Voc_Star_Send(void)
{
	O_CLK_0;
	delay_ms(1);
}

static void Voc_Send_DataH(void)
{
	O_DATA_1;
	delay_us(800);
	O_CLK_1;
	delay_us(1600);

	O_CLK_0;
	delay_us(800);
}

static void Voc_Send_DataL(void)
{
	O_DATA_0;
	delay_us(800);
	O_CLK_1;
	delay_us(1600);

	O_CLK_0;
	delay_us(800);
}

static void Voc_End_Send(void)
{
	O_CLK_0;
	O_DATA_0;
}

static void Voc_Send_Data(unsigned char se_data)
{
	unsigned char mask = 0x80;
	unsigned char temp, data_coun = 8;
	Voc_Star_Send();
	for(; data_coun > 0; data_coun--)
	{
		temp = se_data;
		if(temp & mask)
			Voc_Send_DataH();
		else
			Voc_Send_DataL();
		mask >>= 1;
	}
	Voc_End_Send();
}

void Voc_Cmd(unsigned char eVoc)
{
	if(s_bEnableVoceBufferMode)
	{
		Voc_Send_Data(0xF3);
		s_bEnableVoceBufferMode = false;
	}
	Voc_Send_Data((unsigned char)eVoc);
}

void Voc_Close(void)
{
	Voc_Send_Data(0xF2);
	delay_ms(10);
	Voc_Send_Data(0x30);
}

#else

void Voc_Init(void){}
void Voc_Cmd(unsigned char eVoc){}
void Voc_Close(void){}

#endif

