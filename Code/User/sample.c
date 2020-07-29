#include "platform.h"
#include "sample.h"
#include "stdio.h"
#include "adc.h"
#include "ads8320.h"
#include "parameter.h"
#include "calibrate.h"
#include "public_function.h"
#include "Windows.h"
#include "test.h"
#include "app.h"

extern CW_TestMain *pTestWin;


sAlcSample g_sAlcSample;
sSamplePoint sSamplePointArray[SAMPLE_RPOCESS_BUFFER_SIZE];

#if ALCOHOL_SENSOR_INTEGRAL_DEBUG
	unsigned short g_aIntergralValue[2000];
#endif

static void Adc_Cache_ALC_Sample_Data(void);
static void Create_Process_Progress_Tip(void);

static void Array_Sort_By_Ascending_(sSamplePoint *a, char num)
{
	unsigned short i, j;
	sSamplePoint sTemp;
	for(i = 0; i < num - 1; i++)
	{
		for (j = 0; j < num - i - 1; j++)
			if((a + j)->nValue > (a + j + 1)->nValue)
			{
				sTemp = *(a + j);
				*(a + j) = *(a + j + 1);
				*(a + j + 1) = sTemp;
			}
	}
}

void TIM5_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM5, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM5 , TIM_FLAG_Update);
		Adc_Cache_ALC_Sample_Data();
	}
}

void Sample_Timer_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	TIM_DeInit(TIM5);

	/*Timer时基设置外设时钟频率*/
	TIM_TimeBaseStructure.TIM_Period = 420 - 1; 
	TIM_TimeBaseStructure.TIM_Prescaler = 200 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);

	TIM_ARRPreloadConfig(TIM5, ENABLE);
	TIM_ClearFlag(TIM5, TIM_FLAG_Update);
	TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM5, ENABLE);
}

void Start_Sample(void)
{
	g_bSpiHardwareBusBusy = true;
	
	memset(&g_sAlcSample,0,sizeof(g_sAlcSample));
	memset(sSamplePointArray, 0, sizeof(sSamplePointArray));

	pTestWin->fAlcSampleTemperature =  Adc_Get_Sensor_Temperature();

	/*确定多少少次积分可以获得最大值*/
	if(pTestWin->fAlcSampleTemperature <= -3)
	{
		g_sAlcSample.nCntGetPeekValue = 1800;
	}
	else if(pTestWin->fAlcSampleTemperature > -3 && pTestWin->fAlcSampleTemperature < 3)
	{
		g_sAlcSample.nCntGetPeekValue = 1400;
	}
	else
	{
		g_sAlcSample.nCntGetPeekValue = 1000;
	}

	/*采样接口初始化,创建采样进度指示*/
	Ad8320_Gpio_Init();
	Create_Process_Progress_Tip();

	Sample_Timer_Init();
}

void Stop_Sample(void)
{
	TIM_DeInit(TIM5);
	g_bSpiHardwareBusBusy = false;
}

static void Create_Process_Progress_Tip(void)
{
	GUI_RECT rect;
	GUI_SetFont(&GUI_FontTahoma58);
	GUI_GetTextExtend(&rect, "--", 10);
	pTestWin->hProcessProgress = TEXT_CreateEx((LCD_XSIZE - (rect.x1 - rect.x0 + 1 + 15)) >> 1, (LCD_YSIZE - (rect.y1 - rect.y0 + 1)) >> 1, rect.x1 - rect.x0 + 1 + 15, rect.y1 - rect.y0 + 1,
	                     pTestWin->wObj.hWin,
	                     WM_CF_SHOW, 0, 0, "--");
	TEXT_SetTextColor(pTestWin->hProcessProgress, GUI_BLUE);
	TEXT_SetFont(pTestWin->hProcessProgress, &GUI_FontTahoma58);
	TEXT_SetTextAlign(pTestWin->hProcessProgress, GUI_TA_CENTER);
}

static void Display_Process_Progress_Tip(void)
{
	static unsigned char nStringIndex = 0;
	const char *aProcessingTip[] = {"--", "\\", "|", "/"};
	TEXT_SetText(pTestWin->hProcessProgress, aProcessingTip[nStringIndex]);
	if(++nStringIndex > 3) nStringIndex = 0;
}

/*此函数要被中断定时器调用,调用周期1ms*/
static void Adc_Cache_ALC_Sample_Data(void)
{
	unsigned short nTemp;
	nTemp = Adc_Get_Alcohol_Sensor_Value_Fast();
	g_sAlcSample.aAdcSampleDataBuffer[g_sAlcSample.nSampleDataInCnt++] = nTemp;
	if(g_sAlcSample.nSampleDataInCnt >= ADC_SAMPLE_BUFFER_SIZE)
	{
		g_sAlcSample.nSampleDataInCnt = 0;
	}
}

static unsigned short Get_Cached_ALC_Sample_Data(void)
{
	unsigned short nTemp;
	nTemp = g_sAlcSample.aAdcSampleDataBuffer[g_sAlcSample.nSampleDataOutCnt];
	if(++g_sAlcSample.nSampleDataOutCnt >= ADC_SAMPLE_BUFFER_SIZE)
	{
		g_sAlcSample.nSampleDataOutCnt = 0;
	}
	return nTemp;
}

bool Alcohol_Sensor_AD_Integral(void)
{
	int i;
	unsigned short nAdc_Value;
	unsigned long nAverageValue;

	while(g_sAlcSample.nSampleDataOutCnt != g_sAlcSample.nSampleDataInCnt)
	{
		g_sAlcSample.nTotalIntegralCnt++;

		if(g_sAlcSample.nTotalIntegralCnt % 500 == 0)
		{
			Display_Process_Progress_Tip();
		}

		if(g_sAlcSample.nTotalIntegralCnt > MAX_AD_INTEGRAL_CNT)
		{
			return true;
		}

		nAdc_Value = Get_Cached_ALC_Sample_Data();

		g_sAlcSample.nIntegralSumValue += nAdc_Value;

	#if ALCOHOL_SENSOR_INTEGRAL_DEBUG
		if(g_sAlcSample.nTotalIntegralCnt < (sizeof(g_aIntergralValue) >> 1))
		{
			g_aIntergralValue[g_sAlcSample.nTotalIntegralCnt - 1] = nAdc_Value;
		}
	#endif
		
		for(i = 0; i < SAMPLE_RPOCESS_BUFFER_SIZE; i++)
		{
			if(nAdc_Value > sSamplePointArray[i].nValue)
			{
				Array_Sort_By_Ascending_(sSamplePointArray, SAMPLE_RPOCESS_BUFFER_SIZE);
				sSamplePointArray[0].nCnt = g_sAlcSample.nTotalIntegralCnt;
				sSamplePointArray[0].nValue = nAdc_Value;
				break;
			}
		}

		if(g_sAlcSample.nTotalIntegralCnt > g_sAlcSample.nCntGetPeekValue && !g_sAlcSample.bGetPeekValue)
		{
			nAverageValue = 0;
			for(i = 0; i < SAMPLE_RPOCESS_BUFFER_SIZE; i++)
			{
				nAverageValue += sSamplePointArray[i].nValue;
			}
			g_sAlcSample.nSampleMaxValue = nAverageValue / SAMPLE_RPOCESS_BUFFER_SIZE;

			g_sAlcSample.nIntergralLowerValue = g_sAlcSample.nSampleMaxValue * SAMPLE_INTEGRAL_LOWER_RATIO + 0.5;
			
			g_sAlcSample.bGetPeekValue = TRUE;
		}
		else
		{
			if(g_sAlcSample.bGetPeekValue)
			{
				sSamplePointArray[g_sAlcSample.nTotalIntegralCnt % SAMPLE_RPOCESS_BUFFER_SIZE].nCnt = g_sAlcSample.nTotalIntegralCnt;
				sSamplePointArray[g_sAlcSample.nTotalIntegralCnt % SAMPLE_RPOCESS_BUFFER_SIZE].nValue = nAdc_Value;

				nAverageValue = 0;
				for(i = 0; i < SAMPLE_RPOCESS_BUFFER_SIZE; i++)
				{
					nAverageValue += sSamplePointArray[i].nValue;
				}

				nAverageValue = nAverageValue / SAMPLE_RPOCESS_BUFFER_SIZE;

				if(nAverageValue <= g_sAlcSample.nIntergralLowerValue || nAverageValue <= 40)
				{
					return true;
				}
			}
		}
	}

	return false;
}



