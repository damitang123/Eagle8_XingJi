#include "platform.h"
#include "Windows.h"
#include "ads8320.h"
#include "app.h"
#include "rtc.h"
#include "delay.h"

#if USE_ADS8320
	#define 	ALCOHOL_SENSOR_CLEAN_AD	100
#elif USE_ADS3201
	#define 	ALCOHOL_SENSOR_CLEAN_AD	1
#endif


#define	READ_SDA_DATA()		GPIOE->IDR & GPIO_Pin_0

#define	SCL_ADS8320_H()		GPIOC->BSRR = GPIO_Pin_9
#define	SCL_ADS8320_L()		GPIOC ->BRR = GPIO_Pin_9


#define	CS_ADS8320_H()			GPIOC->BSRR = GPIO_Pin_6
#define	CS_ADS8320_L()			GPIOC->BRR = GPIO_Pin_6


static void Sample_Delay(long dwCount)
{
	long lcount = 6 * dwCount;
	while(--lcount >= 0);
}

void Ad8320_CS_Enable(void)
{
	CS_ADS8320_L()	;
}

void Ad8320_CS_Disable(void)
{
	CS_ADS8320_H()	;
}

void Ad8320_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 

	/*SCLK ,CS*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	/*MISO*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure); 

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure); 

	CS_ADS8320_H();
}


unsigned short Get_Ads8320_Value()
{
	int i;
	unsigned short nData = 0;

	SCL_ADS8320_L();
	CS_ADS8320_L();
	Sample_Delay(1);

#if USE_ADS8320

	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();

	Sample_Delay(1);

	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();

	Sample_Delay(1);

	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();

	Sample_Delay(1);

	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();

	Sample_Delay(1);

	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();

	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();

#elif USE_ADS3201

	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();
	Sample_Delay(1);
	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();
	Sample_Delay(1);
	SCL_ADS8320_H();
	Sample_Delay(1);
	SCL_ADS8320_L();

	Sample_Delay(1);

#endif

#if	USE_ADS8320
	for(i = 0; i < 16; i++)
	{
		SCL_ADS8320_H();
		Sample_Delay(1);
		nData <<= 1;
		if(READ_SDA_DATA())
			nData |= 0x01;
		SCL_ADS8320_L();
	}
#elif USE_ADS3201
	for (i = 0x0800; i > 0 ; i /= 2)
	{
		SCL_ADS8320_H();
		Sample_Delay(1);
		if (READ_SDA_DATA())
			nData = (nData | i);
		Sample_Delay(1);
		SCL_ADS8320_L();
		Sample_Delay(1);
	}
#endif

	CS_ADS8320_H();
	Sample_Delay(1);

	return nData;
}


unsigned short Adc_Get_Alcohol_Sensor_Value_Fast(void)
{
	return Get_Ads8320_Value();
}

unsigned short Adc_Get_Alcohol_Sensor_Value(void)
{
	unsigned short ret;
	Ad8320_Gpio_Init();
	ret = Get_Ads8320_Value();
	return ret;
}

BOOL Adc_Is_Alcohol_Sensor_Ready(void)
{
	unsigned char i = 4;
	unsigned short adc_value = 0;
	while(i--)
	{
		delay_ms(5);
		adc_value += Adc_Get_Alcohol_Sensor_Value();
	}

	if((adc_value / 4) < ALCOHOL_SENSOR_CLEAN_AD)
		return TRUE;
	else
		return FALSE;
}


void AD8320_Test(void)
{
	Ad8320_Gpio_Init();
	TRACE_PRINTF("Adc_Get_Alcohol_Sensor_Value=%d\r\n", Adc_Get_Alcohol_Sensor_Value());
}

