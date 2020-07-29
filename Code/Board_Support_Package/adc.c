#include "platform.h"
#include "Windows.h"
#include "test.h"
#include "adc.h"
#include "parameter.h"
#include "public_function.h"
#include <stdlib.h>

/*
	PC0,AN10,PUMP VOLTAGE
	PC1,AN11,TC1047.
	PC2,AN12,NTC
	PC3,AN13,BATTERY
	PC5,AN15,BLOW PRESS
*/

static bool s_bAdcInConfigMode;
static unsigned char s_nAdcChannelIndex;
static unsigned short * s_pADCDataBuffer;
static unsigned char s_nADCRetryCnt;

unsigned short ADCConvertedValue[NumberOfSampleTimes][NumberOfChannel];

static void ADC_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void ADC_Circular_Scan(void)
{
	switch(s_nAdcChannelIndex)
	{
		case ADC_Channel_10:
			if(s_bAdcInConfigMode)
			{
				s_bAdcInConfigMode = false;
				ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_239Cycles5);
				ADC_Cmd(ADC1, ENABLE);
				ADC_SoftwareStartConvCmd(ADC1,ENABLE);
			}
			else
			{
				while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==RESET);
				*s_pADCDataBuffer++ = ADC_GetConversionValue(ADC1);
				s_bAdcInConfigMode = true;
				s_nAdcChannelIndex = ADC_Channel_11;
			}
			break;

		case ADC_Channel_11:
			if(s_bAdcInConfigMode)
			{
				s_bAdcInConfigMode = false;
				ADC_RegularChannelConfig(ADC1,ADC_Channel_11,1,ADC_SampleTime_239Cycles5);
				ADC_Cmd(ADC1, ENABLE);
				ADC_SoftwareStartConvCmd(ADC1,ENABLE);
			}
			else
			{
				while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==RESET);
				*s_pADCDataBuffer++ = ADC_GetConversionValue(ADC1);
				s_bAdcInConfigMode = true;
				s_nAdcChannelIndex = ADC_Channel_12;
			}
			break;

		case ADC_Channel_12:
			if(s_bAdcInConfigMode)
			{
				s_bAdcInConfigMode = false;
				ADC_RegularChannelConfig(ADC1,ADC_Channel_12,1,ADC_SampleTime_239Cycles5);
				ADC_Cmd(ADC1, ENABLE);
				ADC_SoftwareStartConvCmd(ADC1,ENABLE);
			}
			else
			{
				while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==RESET);
				*s_pADCDataBuffer++ = ADC_GetConversionValue(ADC1);
				s_bAdcInConfigMode = true;
				s_nAdcChannelIndex = ADC_Channel_13;
			}
			break;

		case ADC_Channel_13:
			if(s_bAdcInConfigMode)
			{
				s_bAdcInConfigMode = false;
				ADC_RegularChannelConfig(ADC1,ADC_Channel_13,1,ADC_SampleTime_239Cycles5);
				ADC_Cmd(ADC1, ENABLE);
				ADC_SoftwareStartConvCmd(ADC1,ENABLE);
			}
			else
			{
				while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==RESET);
				*s_pADCDataBuffer++ = ADC_GetConversionValue(ADC1);
				s_bAdcInConfigMode = true;
				s_nAdcChannelIndex = ADC_Channel_15;
			}
			break;

		case ADC_Channel_15:
			if(s_bAdcInConfigMode)
			{
				s_bAdcInConfigMode = false;
				ADC_RegularChannelConfig(ADC1,ADC_Channel_15,1,ADC_SampleTime_239Cycles5);
				ADC_Cmd(ADC1, ENABLE);
				ADC_SoftwareStartConvCmd(ADC1,ENABLE);
			}
			else
			{
				while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==RESET);
				*s_pADCDataBuffer++ = ADC_GetConversionValue(ADC1);
				s_bAdcInConfigMode = true;
				s_nAdcChannelIndex = ADC_Channel_10;
				if(++s_nADCRetryCnt == NumberOfSampleTimes)
				{
					s_nADCRetryCnt = 0;
					s_pADCDataBuffer = (unsigned short *)ADCConvertedValue;
				}
			}
			break;
	}
}

void TIM2_IRQHandler(void)
{
	 if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{   	
		TIM_ClearITPendingBit(TIM2 , TIM_FLAG_Update);
		ADC_Circular_Scan();
	}
}

static void ADC_Triger_Timer_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);          
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

	TIM_TimeBaseStructure.TIM_Prescaler = 64-1;
	TIM_TimeBaseStructure.TIM_Period = 10000 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up ;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 
  
	TIM_ARRPreloadConfig(TIM2,ENABLE);
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);            	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);	
	TIM_Cmd(TIM2, ENABLE); 
}

void ADC1_Configuration(void)
{
	ADC_InitTypeDef ADC_InitStructure;

	ADC_GPIO_Config();

	RCC_ADCCLKConfig(RCC_PCLK2_Div8);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);

	ADC_DeInit(ADC1);

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_ExternalTrigConvCmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* Enable ADC1 reset calibaration register */	
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));

	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));

	ADC_Cmd(ADC1, DISABLE);

	s_nADCRetryCnt = 0; 
	s_bAdcInConfigMode = true;
	s_nAdcChannelIndex = ADC_Channel_10;
	s_pADCDataBuffer = (unsigned short *)ADCConvertedValue;

	ADC_Triger_Timer_Init();
}


unsigned short ADC_Software_Filter(unsigned char  Index)
{
	unsigned char i;

	unsigned short aTemp[NumberOfSampleTimes];
	for(i = 0; i < NumberOfSampleTimes; i++)
	{
		aTemp[i] = ADCConvertedValue[i][Index];
	}
	Array_Sort_By_Ascending(aTemp, sizeof(aTemp) / sizeof(aTemp[0]));

	/*取结果中最中间的那个值*/
	return aTemp[(NumberOfSampleTimes - 1) / 2];	
}

/*判断抽气泵充电是否准备完毕*/
BOOL Adc_Is_Pump_Ready(void)
{
	if(Adc_Get_PumpVoltage() >= ADC_PUMP_VOLTAGE_READY)
		return TRUE;
	else
		return FALSE;

}

/*判断是否正在吹气，参数PrimitiveValue 为未呼气时压力值*/
BOOL Adc_Is_Blowing(unsigned short nOriginalAdcValue, signed char nEnvironmentTemperature)
{
	unsigned short nAdc_Value; 
	unsigned short nActualPressChange;
	unsigned short nRequestPressChange;

	nAdc_Value = Adc_Get_PressSensorValue();
	nRequestPressChange = Parameter_Get_MinDifferencePress(&g_sTestParam);

#if defined(USE_NEGATIVE_PRESSURE_PIPE)
	if(nAdc_Value > nOriginalAdcValue)
	{
		return FALSE;
	}
#else
	if(nAdc_Value < nOriginalAdcValue)
	{
		return FALSE;
	}
#endif
	
	nActualPressChange = abs(nAdc_Value - nOriginalAdcValue);

	if(nActualPressChange > nRequestPressChange)
	{
		g_nInvalidBlowPressCnt = 0;
		return TRUE;
	}
	else
	{
		if(++g_nInvalidBlowPressCnt > 3)
		{
			g_nInvalidBlowPressCnt = 3;
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}


BOOL Adc_Is_Inhaling(unsigned short nOriginalAdcValue, signed char nEnvironmentTemperature)
{
	short nActualPressChange;
	unsigned short nAdc_Value = Adc_Get_PressSensorValue();

	nActualPressChange = nOriginalAdcValue - nAdc_Value;

	if(nEnvironmentTemperature > 10 || nEnvironmentTemperature < 60)
	{
		if(nActualPressChange > 200 * 3)
			return TRUE;
	}
	else
	{
		if(nActualPressChange > 260 * 0.3 * 3)
			return TRUE;
	}

	return FALSE;
}

const sTemperature_VS_Adc g_Table_TC1047[] =
{
	{-15,	512 	},
	{-10,	570 	},
	{-5	,	634 	},
	{0	,	688 	},
	{5	,	762 	},
	{10	,	834 	},
	{15	,	911 	},
	{20	,	972 	},
	{25	,	1037 },
	{30	,	1084 },
	{35	,	1134},
	{40	,	1196},
	{45	,	1281},
	{50	,	1338},
};

const sTemperature_VS_Adc g_Table_NTC103AT[] =
{
	{-15,	716 	},
	{-10,	836 	},
	{-5	,	990 	},
	{0	,	1124 },
	{5	,	1326 },
	{10	,	1527 },
	{15	,	1749 },
	{20	,	1911 },
	{25	,	2100 },
	{30	,	2226 },
	{35	,	2366},
	{40	,	2545},
	{45	,	2725},
	{50	,	2856},
};

float Get_Temperature_Via_Adc(unsigned short nAdcValue,const sTemperature_VS_Adc *LUT, unsigned char LUT_SIZE)
{
	float k;
	unsigned char i;
	int y1, y2, x1, x2;
	
	if(nAdcValue <= LUT[0].adcValue)
	{
		return LUT[0].temperature;
	}

	if(nAdcValue >= LUT[LUT_SIZE - 1].adcValue)
	{
		return LUT[LUT_SIZE - 1].temperature;
	}

	for(i = 0; i < LUT_SIZE; i++)
	{
		if(nAdcValue < LUT[i].adcValue)	break;
	}

	y2 = LUT[i].temperature;
	y1 = LUT[i - 1].temperature;
	x2 = LUT[i].adcValue;
	x1 = LUT[i - 1].adcValue;

	k = (float)(y2 - y1) / (float)(x2 - x1);			/*求斜率*/

	return y1 + k * (nAdcValue - x1);
}


/*获取酒精传感器的温度*/
float Adc_Get_Sensor_Temperature(void)
{
	return Get_Temperature_Via_Adc(Adc_Get_Alc_Sensor_Temprature_Value(),g_Table_NTC103AT,ARR_SIZE(g_Table_NTC103AT));
}


/*获取环境温度,此IC没有焊接.默认返回0*/
float Adc_Get_Environment_Temperature(void)
{
	return Get_Temperature_Via_Adc(Adc_Get_TC1047_Value(),g_Table_TC1047,ARR_SIZE(g_Table_TC1047));
}


/*获取电池电压值*/
float Adc_Get_Battery_Voltage(void)
{
	float k;
	int y1, y2, x1, x2;
	unsigned short nAdc_Value = Adc_Get_Battery_Value();
	/*3V~8V, adc range: 840~2015*/
	y2 = 8;
	y1 = 3;
	x2 = 2114;
	x1 = 793;

	k = (float)(y2 - y1) / (float)(x2 - x1);			/*求斜率*/

	return y1 + k * (nAdc_Value - x1);
}

unsigned char Adc_Get_Battery_Volume(void)
{
	#define BATTERY_MAX	7.6
	#define BATTERY_MIN		6.5
	#define STATIC_OFFSET	0.05	/*防止电池电量显示漂移.*/

	float fDiv = (BATTERY_MAX - BATTERY_MIN) / 5.0;
	float fBattery = Adc_Get_Battery_Voltage();

	static unsigned char bFirstGetVoltage = 1;
	static unsigned char nVolumeBak;

	if(bFirstGetVoltage) goto _FirstGetVoltage;


	if(fBattery > (BATTERY_MAX - fDiv * 0 + STATIC_OFFSET))
	{
		nVolumeBak = 5;
		return nVolumeBak;
	}

	if((fBattery > (BATTERY_MAX - fDiv * 1 + STATIC_OFFSET)) && (fBattery < (BATTERY_MAX - fDiv * 0 - STATIC_OFFSET)))
	{
		nVolumeBak = 4;
		return nVolumeBak;
	}

	if((fBattery > (BATTERY_MAX - fDiv * 2 + STATIC_OFFSET)) && (fBattery < (BATTERY_MAX - fDiv * 1 - STATIC_OFFSET)))
	{
		nVolumeBak = 3;
		return nVolumeBak;
	}

	if((fBattery > (BATTERY_MAX - fDiv * 3 + STATIC_OFFSET)) && (fBattery < (BATTERY_MAX - fDiv * 2 - STATIC_OFFSET)))
	{
		nVolumeBak = 2;
		return nVolumeBak;
	}

	if((fBattery > (BATTERY_MAX - fDiv * 4 + STATIC_OFFSET)) && (fBattery < (BATTERY_MAX - fDiv * 3 - STATIC_OFFSET)))
	{
		nVolumeBak = 1;
		return nVolumeBak;
	}

	if((fBattery > (BATTERY_MAX - fDiv * 5 + STATIC_OFFSET)) && (fBattery < (BATTERY_MAX - fDiv * 4 - STATIC_OFFSET)))
	{
		nVolumeBak = 0;
		return nVolumeBak;
	}

	if((fBattery < BATTERY_MIN - 0.7 ) && (fBattery > 5.5))
	{
		return 0;	/*电量太低,准备关机.*/
	}

	if(fBattery <= 5.5)
	{
		return 0xFF;	/*未装电池,不显示电池图标*/
	}

	return nVolumeBak;


_FirstGetVoltage:
	bFirstGetVoltage = 0;

	if(fBattery >= (BATTERY_MAX - fDiv * 0))
	{
		nVolumeBak = 5;
		return nVolumeBak;
	}

	if(fBattery >= (BATTERY_MAX - fDiv * 1))
	{
		nVolumeBak = 4;
		return nVolumeBak;
	}

	if(fBattery >= (BATTERY_MAX - fDiv * 2))
	{
		nVolumeBak = 3;
		return nVolumeBak;
	}

	if(fBattery >= (BATTERY_MAX - fDiv * 3))
	{
		nVolumeBak = 2;
		return nVolumeBak;
	}

	if(fBattery >= (BATTERY_MAX - fDiv * 4))
	{
		nVolumeBak = 1;
		return nVolumeBak;
	}

	if(fBattery >= (BATTERY_MAX - fDiv * 5))
	{
		nVolumeBak = 0;
		return nVolumeBak;
	}

	return 0xff;
}

void ADC_Test(void)
{
	TRACE_PRINTF("Adc_Get_TC1047_Value() = %d\r\n", Adc_Get_TC1047_Value());
	TRACE_PRINTF("Adc_Get_Alc_Sensor_Temprature_Value() = %d\r\n", Adc_Get_Alc_Sensor_Temprature_Value());
	TRACE_PRINTF("Adc_Get_PumpVoltage() = %d\r\n", Adc_Get_PumpVoltage());
	TRACE_PRINTF("Adc_Get_PressSensorValue() = %d\r\n", Adc_Get_PressSensorValue());
	TRACE_PRINTF("Adc_Get_Battery_Value() = %d\r\n", Adc_Get_Battery_Value());
	TRACE_PRINTF("\r\n\r\n");
}

#if QUICK_TEST

#include "delay.h"

#define 	ALCOHOL_SENSOR_CLEAN_AD	100

void ADC2_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	ADC_InitTypeDef ADC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	ADC_InitStructure.ADC_Mode=ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode=DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode=DISABLE;
	ADC_InitStructure.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign=ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel=1;
	ADC_Init(ADC2,&ADC_InitStructure);

	ADC_RegularChannelConfig(ADC2,ADC_Channel_14,1,ADC_SampleTime_239Cycles5);

	ADC_Cmd(ADC2,ENABLE);

	ADC_ResetCalibration(ADC2);
	while(ADC_GetResetCalibrationStatus(ADC2));

	ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2));
}

unsigned short ADC_Get_Quick_ALC_Sensor(void)
{
	ADC_SoftwareStartConvCmd(ADC2,ENABLE);
	while(!ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC2);
}

bool Adc_Is_Quick_Test_Alcohol_Sensor_Ready(void)
{
	unsigned char i = 4;
	unsigned short adc_value = 0;
	while(i--)
	{
		delay_ms(5);
		adc_value += ADC_Get_Quick_ALC_Sensor();
	}

	if((adc_value / 4) < ALCOHOL_SENSOR_CLEAN_AD)
		return true;
	else
		return false;
}

#else

void ADC2_Configuration(void){}
unsigned short ADC_Get_Quick_ALC_Sensor(void){return 0;}

#endif

