#include "platform.h"
#include "test.h"
#include "Windows.h"
#include "adc.h"
#include "gpio.h"
#include "system_misc.h"
#include "rtc.h"
#include "calibrate.h"

extern CW_TestMain *pTestWin;

#if QUICK_TEST

/*��������ADCֵ*/
#define CLEAN_MAX_ADC_VALUE						200
/*80mg/100ml��Ӧ�ķ�ֵ*/
#define DEFAULT_PEEK_CALIBRATION_VALUE			3160
/*�оƾ�ֵ���������ADCֵ*/
#define DEAULT_ALC_ADC_LOWEST_THRESHOLD			300
/*20mg/100ml��Ӧ��ADCֵ*/
#define DEFAULT_ALC_ADC_20MG_100ML_CRITICAL		750

/*С�ڴ�ֵ,������ʾΪ0*/
#define MIN_DISPLAY_ALC_VALUE						8.0	
/*�оƱ���ʱ��*/
#define ALC_ALARM_DURATION_TIME					5000

bool s_bAlreadyTrigerAlcAlarm;
unsigned long s_nStartAlcAlarmTime;

/*�ƾ�����Ũ��ֵ*/
float g_fAlcValue;
static unsigned short s_nInhalingTimeCnt;

/*���ݱ궨ֵ�������������ֵ,20mg/100ml��Ӧ��ADCֵ.*/
static unsigned short s_nAlc_Adc_Lowest_Threshold;

/*��¼�����������������е������.*/
unsigned char g_aSampleBuffer[LCD_X_SIZE];
signed char g_nSampleTemp[LCD_X_SIZE];
unsigned short g_aSampleDebugBufferA[LCD_X_SIZE];
unsigned short g_aSampleDebugBufferB[LCD_X_SIZE];

static unsigned short Alc_Adc_Temperature_Calibration(unsigned long nOrignalValue, float fTemperature)
{
	typedef struct
	{
		signed char nTemperature;
		float fRatio;
	}__attribute__ ((packed))sTempCaliRatio;

	const sTempCaliRatio aTempCaliRatio[]=
	{
		-5,		2.0,
		0,		1.7,
		5,		1.4,
		10,		1.2,
		15,		1.1,
		20,		1.05,
		25,		1.0,
		30,		1.0,
		35,		1.0,
		40,		0.98,
		45,		0.95,
	};

	int i = 0;
	float fRatio,k;
	float x1,x2,y1,y2;
	unsigned char nSize = ARR_SIZE(aTempCaliRatio);

	if(fTemperature <= aTempCaliRatio[0].nTemperature)
	{
		fRatio = aTempCaliRatio[0].fRatio;
	}
	else if(fTemperature >= aTempCaliRatio[nSize - 1].nTemperature)
	{
		fRatio = aTempCaliRatio[nSize - 1].fRatio;
	}
	else
	{
		for(i = 0; i < nSize ; i++)
		{
			if(fTemperature < aTempCaliRatio[i].nTemperature) 
				break;
		}

		y2 = aTempCaliRatio[i].fRatio;
		y1 = aTempCaliRatio[i - 1].fRatio;
		x2 = aTempCaliRatio[i].nTemperature;
		x1 = aTempCaliRatio[i - 1].nTemperature;

		k = (float)(y2 - y1) / (float)(x2 - x1);

		fRatio = y1 + k * (fTemperature - x1);
	}

	return (unsigned short)(nOrignalValue * fRatio);
}

static void _WaitStart(WM_MESSAGE *pMsg)
{	
	char aTemp[30],aStrTemp[30];
	
	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		WM_HideWindow(pTestWin->hTestTips);
		WM_HideWindow(pTestWin->hTimeProgressBar);
		WM_HideWindow(pTestWin->hBlowFlowVolume);
		WM_HideWindow(pTestWin->hRightButton);
		
		WM_ShowWindow(pTestWin->hQuickTestDraw);
		WM_ShowWindow(pTestWin->hQuickTestDisplayValue);
		WM_ShowWindow(pTestWin->hQuickTestDisplayTime);
		BUTTON_SetText(pTestWin->hMiddleButton, _StartTest);

		TEXT_SetText(pTestWin->hQuickTestDisplayTime, "T= 0.0S");
		Get_Format_Alc_Convert_String(0.0, g_sTestParam.nTestUnit, aTemp, eFormatToInt);
		sprintf(aStrTemp,"%s= %s%s", Get_TestMode_String(g_sTestParam.nTestUnit), aTemp, Get_Unit_String(g_sTestParam.nTestUnit));
		TEXT_SetText(pTestWin->hQuickTestDisplayValue, aStrTemp);
	}
	
	/*������ �¼�*/
	switch(WM_Key_Code)
	{
		case GUI_KEY_LEFT_FUN:
			WM_DeleteWindow(pMsg->hWin);
			break;
			
		case GUI_KEY_MIDDLE_FUN:
			g_eTestState = eStartTestPrepare;
			break;
	}
}

static void _StartTestPrepare(WM_MESSAGE *pMsg)
{
	float fTemp;
	char aTemp[30],aStrTemp[30];
	
	g_eTestState = eInhaling;

	/*����ǰ������ʼ��*/
	g_fAlcValue = 0;
	s_nInhalingTimeCnt = 0;
	s_nStartAlcAlarmTime = 0;
	s_bAlreadyTrigerAlcAlarm = false;
	memset(g_aSampleBuffer, 0, sizeof(g_aSampleBuffer));
	s_nAlc_Adc_Lowest_Threshold = g_sAlcCalibrationParam.nPeekCaliValue * DEAULT_ALC_ADC_LOWEST_THRESHOLD / DEFAULT_PEEK_CALIBRATION_VALUE;

	TRACE_PRINTF("s_nAlc_Adc_Lowest_Threshold = %d\r\n",s_nAlc_Adc_Lowest_Threshold);

	/*����ǰ��Ӳ��׼��*/
	Voc_Cmd(eVOC_BLOW);
	Gpio_Ctrl(eGpio_Moto_Ctrl, true);
	Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl, false);
	Gpio_Ctrl(eGpio_Buzzer_Ctrl,false);
	g_nStartInhaleTime  = Get_System_Time();

	WM_HideWindow(pTestWin->hTestTips);
	WM_HideWindow(pTestWin->hTimeProgressBar);
	WM_HideWindow(pTestWin->hBlowFlowVolume);
	WM_HideWindow(pTestWin->hLeftButton);
	WM_HideWindow(pTestWin->hRightButton);
	
	WM_ShowWindow(pTestWin->hQuickTestDraw);
	WM_ShowWindow(pTestWin->hQuickTestDisplayValue);
	WM_ShowWindow(pTestWin->hQuickTestDisplayTime);
	BUTTON_SetText(pTestWin->hMiddleButton, _OK);

	fTemp = Get_Final_Convert_Display_Result(0, g_sTestParam.nTestUnit, g_sTestParam.nBrACBACK);
	Get_Format_Alc_Convert_String(fTemp, g_sTestParam.nTestUnit, aTemp, eFormatToInt);

	TEXT_SetText(pTestWin->hQuickTestDisplayTime, "T= 0.0S");
	sprintf(aStrTemp,"%s= %s%s", Get_TestMode_String(g_sTestParam.nTestUnit), aTemp, Get_Unit_String(g_sTestParam.nTestUnit));
	TEXT_SetText(pTestWin->hQuickTestDisplayValue, aStrTemp);

	/*����ɨ�趨ʱ��.*/
	GUI_TIMER_SetPeriod(pTestWin->hTMTest, g_sTestParam.nInhaleTime / Get_Curve_Windows_Width());
}

static void _Inhaling(WM_MESSAGE *pMsg)
{
	char aTemp[30],aStrTemp[30];
	float fK,fTemp,fAlc;
	signed int x0,x1,y0,y1;
	unsigned short nAdc,nCaliAdc;
	/*�ƾ�Ũ�ȶ�Ӧ�Ŀ̶�*/
	unsigned short nAlcVsYCoordinate;

	/*1.��ȡ������ADC,�������¶�У׼*/
	nAdc = ADC_Get_Quick_ALC_Sensor();
	fTemp = Adc_Get_Alc_Sensor_Temprature_Value();
	nCaliAdc = Alc_Adc_Temperature_Calibration(nAdc, fTemp);

	/*2.ʵʱ���㵱ǰ�ľƾ�ֵ.*/
	if(nCaliAdc < s_nAlc_Adc_Lowest_Threshold)
	{
		fAlc = 0;
	}
	else
	{
		fK = ((float) 80) / (g_sAlcCalibrationParam.nPeekCaliValue - s_nAlc_Adc_Lowest_Threshold);
		fAlc = fK * (nCaliAdc - s_nAlc_Adc_Lowest_Threshold) + 0;
	}

	/*3.��ӳ���ֵ����*/
	if(fAlc < g_sTestParam.nAlcMinRemapValue)
	{
		fAlc = 0;
	}
	else
	{
		fK = (float)MAX_QUICK_TEST_DISPLAY_ALC_VALUE / (float)(MAX_QUICK_TEST_DISPLAY_ALC_VALUE - g_sTestParam.nAlcMinRemapValue);
		fAlc = fK * (fAlc - g_sTestParam.nAlcMinRemapValue) + 0;
	}

	/*4.������ľƾ�Ũ��ֵ*/
	if(fAlc > g_fAlcValue)
	{
		g_fAlcValue = fAlc;

	}

	/*5.�����ͼ������*/
	nAlcVsYCoordinate = fAlc * Get_Curve_Windows_Height() / MAX_QUICK_TEST_DISPLAY_ALC_VALUE;

	/*6.��ֹ���Ƶ�����̫ϸ,��ֹ������,�����Ƴ�����Χ������.*/
	if(nAlcVsYCoordinate < 3)
	{
		nAlcVsYCoordinate = 3;
	}
	else if(nAlcVsYCoordinate >= Get_Curve_Windows_Height())
	{
		nAlcVsYCoordinate = Get_Curve_Windows_Height();
	}

	/*7.���ݾƾ�Ũ�Ȼ��ƴ�ֱ��.*/
	x0 = s_nInhalingTimeCnt + 20; /*COORDINATE_X0 = 20*/
	y0 = 200;
	x1 = s_nInhalingTimeCnt + 20;/*COORDINATE_X0 = 20*/
	y1  = 200 - nAlcVsYCoordinate;

	WM_SelectWindow(pTestWin->hQuickTestDraw);
	GUI_SetColor(RGB2Hex(255,128,0));
	GUI_DrawLine(x0, y0,  x1, y1);

	/*8.��¼�²���ʱ���������Y����,�ػ�ʱʹ��*/
	if(s_nInhalingTimeCnt < Get_Curve_Windows_Width())
	{
		g_aSampleBuffer[s_nInhalingTimeCnt] = nAlcVsYCoordinate;
	#if defined(_USER_DEBUG_)
		g_aSampleDebugBufferA[s_nInhalingTimeCnt] = nAdc;
		g_aSampleDebugBufferB[s_nInhalingTimeCnt] = nCaliAdc;
		g_nSampleTemp[s_nInhalingTimeCnt] = fTemp;
	#endif
	}

#if !TRIGER_DRINK_ALARM_AFTER_INHALE
	/*9.���ݾƾ�����ֵ,��ʾ��ͬ��״̬.*/
	if(g_fAlcValue > g_sTestParam.fAlcDrinkThreshold && !s_bAlreadyTrigerAlcAlarm)
	{
		Voc_Cmd(eVOC_RESULT_DRINK);
		if(g_sTestParam.nAlcAlarmMode == 2 || g_sTestParam.nAlcAlarmMode == 3)
		{
		//	Gpio_Ctrl(eGpio_Vibrate_Ctrl,true);
		}
		if(g_sTestParam.nAlcAlarmMode == 1 || g_sTestParam.nAlcAlarmMode == 3)
		{
		//	Led_Alcohol_Concentration_Alarm_On(g_fAlcValue);
		}
		s_nStartAlcAlarmTime= Get_System_Time();
		s_bAlreadyTrigerAlcAlarm = true;
	}
#endif

	/*10.����ʵʱ����ʱ��*/
	sprintf(aTemp,"T= %.1fS", ((float)g_sTestParam.nInhaleTime * (float)s_nInhalingTimeCnt / (float)Get_Curve_Windows_Width()) / 1000);
	TEXT_SetText(pTestWin->hQuickTestDisplayTime, aTemp);

	/*11.����ʵʱ���Ĳ����ƾ�ֵ*/
	if(g_fAlcValue >= MAX_QUICK_TEST_DISPLAY_ALC_VALUE)
	{
		float fAlcUnitConvertValue;
		aTemp[0] = '>';
		fAlcUnitConvertValue = Get_Final_Convert_Display_Result(MAX_QUICK_TEST_DISPLAY_ALC_VALUE,g_sTestParam.nTestUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fAlcUnitConvertValue,g_sTestParam.nTestUnit,&aTemp[1],eFormatToInt);
	}
	else
	{
		float fAlcUnitConvertValue;
		fAlcUnitConvertValue = Get_Final_Convert_Display_Result(g_fAlcValue,g_sTestParam.nTestUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fAlcUnitConvertValue,g_sTestParam.nTestUnit,aTemp,eFormatToInt);
	}
	sprintf(aStrTemp,"%s= %s%s", Get_TestMode_String(g_sTestParam.nTestUnit), aTemp, Get_Unit_String(g_sTestParam.nTestUnit));
	TEXT_SetText(pTestWin->hQuickTestDisplayValue, aStrTemp);

	/*12.�жϳ����Ƿ����.*/
	if(++s_nInhalingTimeCnt > Get_Curve_Windows_Width())
	{
		/*����ʱ����*/
	#if ALWAYS_INHAL_WHILE_WITHOUT_ALCOHOL
		if(g_fAlcValue <= g_sTestParam.fAlcDrinkThreshold)
		{
			/*��������*/
			g_eTestState = eStartTestPrepare;
			if(g_nStartInhaleTime)
			{
				g_fTotalInhaleTimeCounter += (float)(Get_System_Time() - g_nStartInhaleTime) / (float)1000;
				Data_Total_InhaleTime_Counter_Save(&g_fTotalInhaleTimeCounter);
				g_nStartInhaleTime = 0;
			}
		}
		else
		{
			g_eTestState = eFinishTest;
		}
	#else
		g_eTestState = eFinishTest;
	#endif
	}

	/*13.������ �¼�,ֻ�����м䰴��-->��ǰ��������*/
	switch(WM_Key_Code)
	{
		case GUI_KEY_ENTER:
			g_eTestState = eFinishTest;
			break;
	}
}

static void _FinishTest(WM_MESSAGE *pMsg)
{
	/*����ɨ�趨ʱ��.*/
	GUI_TIMER_SetPeriod(pTestWin->hTMTest, 50);
	
	/*ͣ������,�����㿪�ؿ�������*/
	Gpio_Ctrl(eGpio_Moto_Ctrl, false);
	Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl, true);

	/*�洢�ۼƳ���ʱ��.*/
	if(g_nStartInhaleTime)
	{
		g_fTotalInhaleTimeCounter += (float)(Get_System_Time() - g_nStartInhaleTime) / (float)1000;
		Data_Total_InhaleTime_Counter_Save(&g_fTotalInhaleTimeCounter);
		g_nStartInhaleTime = 0;
	}

	/*���ݵ�λת�����*/
	if(g_fAlcValue < MIN_DISPLAY_ALC_VALUE)
	{
		g_fAlcValue = 0;
	}

#if TRIGER_DRINK_ALARM_AFTER_INHALE
	/*���ݾƾ�����ֵ,��ʾ��ͬ��״̬.*/
	if(g_fAlcValue > g_sTestParam.fAlcDrinkThreshold && !s_bAlreadyTrigerAlcAlarm)
	{
		Voc_Cmd(eVOC_RESULT_DRINK);
		if(g_sTestParam.nAlcAlarmMode == 2 || g_sTestParam.nAlcAlarmMode == 3)
		{
		//	Gpio_Ctrl(eGpio_Vibrate_Ctrl,true);
		}
		if(g_sTestParam.nAlcAlarmMode == 1 || g_sTestParam.nAlcAlarmMode == 3)
		{
		//	Led_Alcohol_Concentration_Alarm_On(g_fAlcValue);
		}
		s_nStartAlcAlarmTime= Get_System_Time();
		s_bAlreadyTrigerAlcAlarm = true;
	}
	else
	{
		Voc_Cmd(eVOC_RESULT_PASS);
	}
#else
	if(g_fAlcValue < g_sTestParam.fAlcDrinkThreshold)
	{
		Voc_Cmd(eVOC_RESULT_PASS);
	}
#endif

	/*��¼����*/
	g_eTestMode = eTestQuickTest;
	if(g_sTestParam.nResultMode == 0)
	{
		g_sRecord.nTestResult = 0;
	}
	else
	{
		if(g_sRecord.fAlcValue > g_sTestParam.fAlcDrunkThreshold)
		{
			g_sRecord.nTestResult = 3;
		}
		else if(g_sRecord.fAlcValue > g_sTestParam.fAlcDrinkThreshold)
		{
			g_sRecord.nTestResult = 2;
		}
		else
		{
			g_sRecord.nTestResult = 1;
		}
	}
	
	/*״̬��ת*/
	g_eTestState = eDisplayResult;
}

static void _DisplayResult(WM_MESSAGE *pMsg)
{
	g_sRecord.nTestMode = g_eTestMode;
	g_fAlcoholVolume = g_fAlcValue;
	pTestWin->fAlcSampleTemperature = Adc_Get_Sensor_Temperature();
	g_sAlcSample.nIntegralSumValue = 0;
	WM_DeleteWindow(pMsg->hWin);
	CW_ShowReport_Create(GetLastWinFromList(),(void *)1);
}

void Scan_Quick_Test_State(WM_MESSAGE *pMsg)
{
	switch(g_eTestState)
	{	
		case eWaitStart:
			_WaitStart(pMsg);
			break;
			
		case eStartTestPrepare:
			_StartTestPrepare(pMsg);
			break;
	
		case eInhaling:
			_Inhaling(pMsg);
			break;

		case eFinishTest:
			_FinishTest(pMsg);
			break;
		
		case eDisplayResult:
			_DisplayResult(pMsg);
			break;
	}
}

#else

void Scan_Quick_Test_State(WM_MESSAGE *pMsg){}	

#endif
