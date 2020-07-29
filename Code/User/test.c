#include "platform.h"
#include "DIALOG.h"
#include "test.h"
#include "parameter.h"
#include "gpio.h"
#include "adc.h"
#include "Windows.h"
#include "ads8320.h"
#include "sample.h"
#include "buzzer.h"
#include "calibrate.h"
#include "PROGBAR.h"
#include "app.h"
#include "data.h"
#include "Keypad.h"
#include "system_misc.h"

extern CW_TestMain *pTestWin;

volatile eTestState g_eTestState;
eTestMode g_eTestMode = eTestActive;
eAlcTestPurpose g_eAlcTestPurpose = eTestForNormal;

  
/*流量有关的-开始*****************************************************************************************/

unsigned char g_nInvalidBlowPressCnt;
/*吹气压力检测*/
unsigned long g_nAdcPressValidDataCnt;
unsigned long g_nSumOfStaticPressValue;

#if defined(USE_NEGATIVE_PRESSURE_PIPE)
const sPressVsBlowRatio g_sPressVsBlowRatioLUT[] = 
{
	0000, 	0,
	1112,	32,
	1390, 	55,
	1668, 	90,
	1946, 	132,
	2224, 	178,
	2502, 	222,
	2780, 	268,
	3058,	328,
	3336, 	398,
	3614, 	491,
	3892, 	560,
	4170, 	682,
	4448, 	753,
	4726, 	847,
	5004, 	973,
	5282, 	1067,
	5560, 	1160,
	5838,	1290,
	6116,	1390,
	6394,	1507,
	6672,	1627,
	6950,	1737,
	7228,	1807,
	7506,	1867	
};
#else
const sPressVsBlowRatio g_sPressVsBlowRatioLUT[] = 
{
	0000, 	0,
	1112,	33,
	1390, 	58,
	1668, 	93,
	1946, 	134,
	2224, 	184,
	2502, 	230,
	2780, 	287,
	3058,	345,
	3336, 	411,
	3614, 	484,
	3892, 	558,
	4170, 	644,
	4448, 	720,
	4726, 	800,
	5004, 	925,
	5282, 	1036,
	5560, 	1210,
	5838,	1331,
	6116,	1444,
	6950,	1850,
};
#endif

bool Adc_Get_Static_Press_Value(unsigned short * nStaticPressValue)
{
	if(g_nAdcPressValidDataCnt++ < 50)
	{
		g_nSumOfStaticPressValue += Adc_Get_PressSensorValue();
	}
	else
	{
		* nStaticPressValue = g_nSumOfStaticPressValue / 50;
		return true;
	}

	return false;
}

unsigned long Get_Blow_Ratio_Via_Via_Press_DiffAdc_Sub(unsigned short nDiffAdcValue)
{
	float k;
	unsigned char i;
	int y1, y2, x1, x2;
	unsigned short nAdc_Value = nDiffAdcValue;

	if(nAdc_Value <= g_sPressVsBlowRatioLUT[0].nDiffAdc)
	{
		return g_sPressVsBlowRatioLUT[0].nBlowRatio;
	}

	if(nAdc_Value >= g_sPressVsBlowRatioLUT[ARR_SIZE(g_sPressVsBlowRatioLUT) - 1].nDiffAdc)
	{
		return g_sPressVsBlowRatioLUT[ARR_SIZE(g_sPressVsBlowRatioLUT) - 1].nBlowRatio;
	}

	for(i = 0; i < ARR_SIZE(g_sPressVsBlowRatioLUT); i++)
	{
		if(nAdc_Value < g_sPressVsBlowRatioLUT[i].nDiffAdc)
			break;
	}

	y2 = g_sPressVsBlowRatioLUT[i].nBlowRatio;
	y1 = g_sPressVsBlowRatioLUT[i - 1].nBlowRatio;
	x2 = g_sPressVsBlowRatioLUT[i].nDiffAdc;
	x1 = g_sPressVsBlowRatioLUT[i - 1].nDiffAdc;

	k = (float)(y2 - y1) / (float)(x2 - x1);			/*求斜率*/

	return y1 + k * (nAdc_Value - x1);
}

double Get_Blow_Ratio_Via_Press_DiffAdc(unsigned short nCurrBlowAdcValue, unsigned long nCurrTime)
{
	double fBlowRatio;
	unsigned long nTime;
	unsigned short nAverageAdc;

	nTime = (nCurrTime - pTestWin->nLastBlowAdcIntegralTime);
	pTestWin->nLastBlowAdcIntegralTime = nCurrTime;
	
	nAverageAdc = (nCurrBlowAdcValue + pTestWin->nLastBlowAdcValue) / 2;
	pTestWin->nLastBlowAdcValue = nCurrBlowAdcValue;

	fBlowRatio = (float)Get_Blow_Ratio_Via_Via_Press_DiffAdc_Sub(abs(nAverageAdc - pTestWin->nPressValueBeforeBlow)) * (float)nTime / 1000;

//	TRACE_PRINTF("nDiffAdc = %d, T = %d, f= %.4f\r\n",nAverageAdc - pTestWin->nPressValueBeforeBlow, nTime, fBlowRatio / 10000);

	/*从ml转换成L*/
	return (fBlowRatio / 10000);
}

/*流量有关的-结束*****************************************************************************************/


/*重测*/
static void _ConfirmRetryTest(void)
{
	g_eTestState = eStart;
}

/*拒绝测试*/
static void _ConfirmRefuseTest(void)
{
	g_eTestState = eDisplayResult;
	g_eTestMode = eTestRefuseMode;
	pTestWin->bNeedRefreshState = true;
	pTestWin->fAlcSampleTemperature =  Adc_Get_Sensor_Temperature();
}

static void _Start(WM_MESSAGE *pMsg)
{
	g_eTestState = ePrepare;
	pTestWin->bNeedRefreshState = true;

	Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,false);
	Gpio_Ctrl(eGpio_Pump_Charge_Sw_Ctrl,true);
	Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,true);
	Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl,true);

	g_nAdcPressValidDataCnt = 0;
	g_nSumOfStaticPressValue = 0;
	g_nInvalidBlowPressCnt = 100;
	pTestWin->nRequestBlowDurationTime = g_sTestParam.nBlowTime;
	pTestWin->fRequestBlowFlowVolume = (float)g_sTestParam.nBlowFlowVolume / 10;
	pTestWin->nEnvTemperatureBeforeBlow = Adc_Get_Environment_Temperature();
	Adc_Get_Static_Press_Value(&pTestWin->nPressValueBeforeBlow);
}

static void _Prepare(WM_MESSAGE *pMsg)
{
	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		TEXT_SetText(pTestWin->hTestTips, _T_Prepare);
	}

	if(Adc_Is_Pump_Ready())
	{
		g_eTestState = eAlcClean;
		pTestWin->bNeedRefreshState = true;
	}

	Adc_Get_Static_Press_Value(&pTestWin->nPressValueBeforeBlow);
	
	/*处理按键 事件*/
	switch(WM_Key_Code)
	{
		case GUI_KEY_LEFT_FUN:
			WM_DeleteWindow(pMsg->hWin);
			break;
	}
}

static void _AlcClean(WM_MESSAGE *pMsg)
{
	bool bAlreadyGetStaticBlowPress;
#if QUICK_TEST
	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		TEXT_SetText(pTestWin->hTestTips, _T_Clean);
		pTestWin->nStartDelayTime = 0;
		pTestWin->nCurrPumpState = 0;
		pTestWin->bAlcSensorReady = false;
		pTestWin->bQuickTestAlcSernsorReady = false;
		pTestWin->nCleanCnt = 0;
		Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,false);
		Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl,false);
	}

	bAlreadyGetStaticBlowPress = Adc_Get_Static_Press_Value(&pTestWin->nPressValueBeforeBlow);

	if(Get_System_Time() > pTestWin->nStartDelayTime + 500)
	{
		pTestWin->nStartDelayTime = Get_System_Time();
		
		Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl, false);
		
		/*普通测试传感器清零*/
		if(Adc_Is_Alcohol_Sensor_Ready())
		{
			if(pTestWin->bAlcSensorReady == false)
			{
				pTestWin->bAlcSensorReady = true;
				Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,true);
			}
		}
		else
		{
			if(pTestWin->nCleanCnt++ < 6)
			{
				pTestWin->nCleanCnt = 6;
				pTestWin->nCurrPumpState = pTestWin->nCurrPumpState ? 0 : 1;
				Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl, pTestWin->nCurrPumpState ? true : false);
			}
			else
			{
				pTestWin->nCurrPumpState = 0;
				Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl, false);
			}
		}

		/*快测传感器清零*/
		if(Adc_Is_Quick_Test_Alcohol_Sensor_Ready())
		{
			if(pTestWin->bQuickTestAlcSernsorReady == false)
			{
				pTestWin->bQuickTestAlcSernsorReady = true;
				Gpio_Ctrl(eGpio_Moto_Ctrl,false);
				Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl,true);
				if(pTestWin->wObj.Property == eTestQuickTest)
				{
					g_eTestState = eStartTestPrepare;
					g_eTestMode = eTestQuickTest;
					pTestWin->bNeedRefreshState = true;
					return;
				}
			}
		}
		else
		{
			Gpio_Ctrl(eGpio_Moto_Ctrl,true);
		}

		/*都清零完成之后再判断其他条件是否成熟*/
		if(pTestWin->bAlcSensorReady && pTestWin->bQuickTestAlcSernsorReady && bAlreadyGetStaticBlowPress && Adc_Is_Pump_Ready())
		{
			g_eTestState = eWaitBlowing;
			pTestWin->bNeedRefreshState = true;
			Voc_Cmd(eVOC_BLOW);
			Buzzer_Beep(eBUZ_DEVICE_OK);
		}
	}		

	/*处理按键 事件*/
	switch(WM_Key_Code)
	{
		case GUI_KEY_LEFT_FUN:
			WM_DeleteWindow(pMsg->hWin);
			break;
	}
	
#else

	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		TEXT_SetText(pTestWin->hTestTips, _T_Clean);
		pTestWin->nStartDelayTime = 0;
		pTestWin->nCurrPumpState = 0;
		pTestWin->bAlcSensorReady = false;
		pTestWin->nCleanCnt = 0;
		Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,false);
	}

	bAlreadyGetStaticBlowPress = Adc_Get_Static_Press_Value(&pTestWin->nPressValueBeforeBlow);

	if(Get_System_Time() > pTestWin->nStartDelayTime + 500)
	{
		pTestWin->nStartDelayTime = Get_System_Time();

		Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl, false);
		
		/*普通测试传感器清零*/
		if(Adc_Is_Alcohol_Sensor_Ready())
		{
			if(pTestWin->bAlcSensorReady == false) 
			{
				pTestWin->bAlcSensorReady = true;
				Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,true);
			}
		}
		else
		{
			if(pTestWin->nCleanCnt++ < 6)
			{
				pTestWin->nCurrPumpState = pTestWin->nCurrPumpState ? 0 : 1;
				Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl, pTestWin->nCurrPumpState ? true : false);
			}
			else
			{
				pTestWin->nCurrPumpState = 0;
				Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl, false);
			}
		}

		/*都清零完成之后再判断其他条件是否成熟*/
		if(pTestWin->bAlcSensorReady && bAlreadyGetStaticBlowPress && Adc_Is_Pump_Ready())
		{
			g_eTestState = eWaitBlowing;
			pTestWin->bNeedRefreshState = true;
			Voc_Cmd(eVOC_BLOW);
			Buzzer_Beep(eBUZ_DEVICE_OK);
		}
	}		

	/*处理按键 事件*/
	switch(WM_Key_Code)
	{
		case GUI_KEY_LEFT_FUN:
			WM_DeleteWindow(pMsg->hWin);
			break;
	}
	
#endif
}

static void _WaitStart(WM_MESSAGE *pMsg)
{

}

static void _StartTestPrepare(WM_MESSAGE *pMsg)
{
	
}

static void _Inhaling(WM_MESSAGE *pMsg)
{
	
}

static void _WaitBlowing(WM_MESSAGE *pMsg)
{
	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		pTestWin->nStartWaitBlowTime = Get_System_Time();
		TEXT_SetText(pTestWin->hTestTips, _T_WaitBlow);
	}

	/*检查是否进入快测模式*/
	if(g_eAlcTestPurpose == eTestForCalibration && g_sAlcCalibrationParam.nCurrCalibrteState == eCaliQuickTest)
	{
		g_eTestState = eWaitStart;
		g_eTestMode = eTestQuickTest;
		pTestWin->bNeedRefreshState = true;
		return;
	}

	if(Adc_Is_Blowing(pTestWin->nPressValueBeforeBlow, pTestWin->nEnvTemperatureBeforeBlow))
	{
		Voc_Close();
		g_eTestState = eBlowing;
		pTestWin->bNeedRefreshState = true;
		pTestWin->nStartBlowTime = Get_System_Time(); 
		pTestWin->fSumOfBlowFlowVolume = 0;
		pTestWin->nLastBlowAdcValue = Adc_Get_PressSensorValue();
		pTestWin->nLastBlowAdcIntegralTime = Get_System_Time();
		pTestWin->bAchieveSampleCondition = false;
		WM_HideWindow(pTestWin->hLeftButton);
		WM_HideWindow(pTestWin->hRightButton);
		Buzzer_On();
	}

	/*在标定模式下不允许出现测试*/
	if(g_eAlcTestPurpose == eTestForNormal || g_eAlcTestPurpose == eTestForMaintain)
	{
		if(Get_System_Time() > pTestWin->nStartWaitBlowTime + 30 * 1000)
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogCommon;
			s.pFont = _GetFont(Font_Content);
			s.pContent = _Retest_Refuse;
			s.nContentColor = GUI_RED;
			s.nBackGroundColor = GUI_LIGHTGRAY;
			s.nAutoCloseTime = 0;
			s.pFunLeftYes = _ConfirmRetryTest;
			s.pFunRightNo = _ConfirmRefuseTest;
			s.pLeftButton = _Retest;
			s.pRightButton = _Refuse;
			CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			g_eTestState = eIdle;
		}
	}

	/*处理按键 事件*/
	switch(WM_Key_Code)
	{
		case GUI_KEY_LEFT_FUN:
			WM_DeleteWindow(pMsg->hWin);
			break;

		case GUI_KEY_MIDDLE_FUN:
			if(g_eAlcTestPurpose == eTestForNormal || g_eAlcTestPurpose == eTestForMaintain)
			{
				/*发起手动测试*/
				g_eTestState = ePump;
				g_eTestMode = eTestPassive;
				pTestWin->bNeedRefreshState = true;
			}
			break;

		case GUI_KEY_RIGHT_FUN:
	#if QUICK_TEST
			if(g_eAlcTestPurpose == eTestForNormal || g_eAlcTestPurpose == eTestForMaintain)
			{
				g_eTestState = eStartTestPrepare;
				g_eTestMode = eTestQuickTest;
				pTestWin->bNeedRefreshState = true;
			}
	#else
			if(g_eAlcTestPurpose == eTestForNormal)
			{
				/*拒绝测试*/
				g_eTestState = eDisplayResult;
				g_eTestMode = eTestRefuseMode;
				pTestWin->bNeedRefreshState = true;
				pTestWin->fAlcSampleTemperature =  Adc_Get_Sensor_Temperature();
			}
	#endif
			break;
		
	}
}

static void _Blowing(WM_MESSAGE *pMsg)
{
	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		TEXT_SetText(pTestWin->hTestTips, _T_Blowing);
	}

	if(Adc_Is_Blowing(pTestWin->nPressValueBeforeBlow, pTestWin->nEnvTemperatureBeforeBlow))
	{
	#if USE_GB_T21254_2017
		char aTemp[30];
		int nBlowTimePercent;
		int nBlowFlowVolumePercent;
		
		Buzzer_On();

		pTestWin->nBlowDurationTime = Get_System_Time() - pTestWin->nStartBlowTime;
		nBlowTimePercent = (pTestWin->nBlowDurationTime) * 100 / pTestWin->nRequestBlowDurationTime;

		/*设置进度条*/
		PROGBAR_SetValue(pTestWin->hTimeProgressBar, nBlowTimePercent);
		sprintf(aTemp, "TIME : %d%%", nBlowTimePercent);
		PROGBAR_SetText(pTestWin->hTimeProgressBar,aTemp);
	
		pTestWin->fSumOfBlowFlowVolume += Get_Blow_Ratio_Via_Press_DiffAdc(Adc_Get_PressSensorValue(), Get_System_Time());
		/*流量校准系数*/
		pTestWin->fSumOfBlowFlowVolume = pTestWin->fSumOfBlowFlowVolume * (1 + g_sSystemParam.nFlowVolumeAdjustCoefficient * 0.05);
		nBlowFlowVolumePercent = pTestWin->fSumOfBlowFlowVolume * 100 / pTestWin->fRequestBlowFlowVolume;

		PROGBAR_SetValue(pTestWin->hBlowFlowVolume, nBlowFlowVolumePercent);
		sprintf(aTemp, "FLOW : %d%%", nBlowFlowVolumePercent);
		PROGBAR_SetText(pTestWin->hBlowFlowVolume,aTemp);

		/*判断吹气是否完成*/
		if(pTestWin->nBlowDurationTime >=  pTestWin->nRequestBlowDurationTime && (pTestWin->fSumOfBlowFlowVolume > pTestWin->fRequestBlowFlowVolume))
		{
			pTestWin->bAchieveSampleCondition = true;
		}
	#else
		char aTemp[30];
		int nBlowTimePercent;

		Buzzer_On();

		pTestWin->nBlowDurationTime = Get_System_Time() - pTestWin->nStartBlowTime;
		nBlowTimePercent = (pTestWin->nBlowDurationTime) * 100 / pTestWin->nRequestBlowDurationTime;

		/*设置进度条*/
		PROGBAR_SetValue(pTestWin->hTimeProgressBar, nBlowTimePercent);
		sprintf(aTemp, "TIME : %d%%", nBlowTimePercent);
		PROGBAR_SetText(pTestWin->hTimeProgressBar,aTemp);
		
		/*判断吹气是否完成*/
		if(pTestWin->nBlowDurationTime >=  pTestWin->nRequestBlowDurationTime)
		{
			pTestWin->bAchieveSampleCondition = true;
		}
	#endif
	}
	else if(pTestWin->bAchieveSampleCondition == false)
	{
		Buzzer_Off();
		Buzzer_Beep(eBUZ_BLOW_BREAK);
		Voc_Cmd(eVOC_BLOW_INT);
		g_eTestState = eBlowInterrupt;
		pTestWin->bNeedRefreshState = true;
		return;
	}

	if(pTestWin->bAchieveSampleCondition)
	{
		if(g_sTestParam.nSampleMethod == 0 || (abs(pTestWin->nPressValueBeforeBlow - Adc_Get_PressSensorValue()) < 50))
		{
			Buzzer_Off();
			g_eTestState = ePump;
			g_eTestMode = eTestActive;
			pTestWin->bNeedRefreshState = true;
			pTestWin->bAchieveSampleCondition = false;
		}
	}

	/*处理按键 事件*/
	switch(WM_Key_Code)
	{
		case GUI_KEY_LEFT_FUN:
			break;

		case GUI_KEY_MIDDLE_FUN:
			if(g_eAlcTestPurpose == eTestForNormal || g_eAlcTestPurpose == eTestForMaintain)
			{
				/*发起手动测试*/
				g_eTestState = ePump;
				g_eTestMode = eTestPassive;
				pTestWin->bNeedRefreshState = true;
			}
			break;

		case GUI_KEY_RIGHT_FUN:
			break;
	}
}

static void _BlowInterrupt(WM_MESSAGE *pMsg)
{
	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		g_nInvalidBlowPressCnt = 100;
		pTestWin->nStartDelayTime = Get_System_Time();
		TEXT_SetText(pTestWin->hTestTips, _T_BlowBreak);
		PROGBAR_SetValue(pTestWin->hTimeProgressBar, 0);
	#if USE_GB_T21254_2017
		PROGBAR_SetValue(pTestWin->hBlowFlowVolume, 0);
	#endif
	}

	if(Get_System_Time() > pTestWin->nStartDelayTime + 600)
	{
		/*只更新状态,不改变显示内容.*/
		g_eTestState = eWaitBlowing;
		pTestWin->nStartWaitBlowTime = Get_System_Time();
		WM_ShowWindow(pTestWin->hLeftButton);
		WM_ShowWindow(pTestWin->hRightButton);
	}
}

static void _Pump(WM_MESSAGE *pMsg)
{
	if(pTestWin->bNeedRefreshState)
	{
		pTestWin->bNeedRefreshState = false;
		TEXT_SetText(pTestWin->hTestTips, _T_Precessing);
	}
	Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,false);
	Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,true);

	Start_Sample();

	g_eTestState = eProcessing;
	pTestWin->bNeedRefreshState = true;
}

static void _Processing(WM_MESSAGE *pMsg)
{
	if(Alcohol_Sensor_AD_Integral())
	{
		Stop_Sample();
		
		Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,true);
		Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl,true);
		Gpio_Ctrl(eGpio_Pump_Charge_Sw_Ctrl,false);
		Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,false);

		g_fAlcoholVolumeOriginal = Calculate_Alcohol_Concentration(g_sAlcSample.nIntegralSumValue,pTestWin->fAlcSampleTemperature);
		g_fAlcoholVolume = Temperature_Calibrate(g_fAlcoholVolumeOriginal, pTestWin->fAlcSampleTemperature);
		//g_fAlcoholVolume=30;//test use

		TRACE_PRINTF("g_fAlcoholVolume = %.3f\r\n",g_fAlcoholVolume);
		TRACE_PRINTF("g_fAlcoholVolumeOriginal = %.3f\r\n",g_fAlcoholVolumeOriginal);
		TRACE_PRINTF("g_sAlcSample.nIntegralSumValue = %ld\r\n",g_sAlcSample.nIntegralSumValue);
		TRACE_PRINTF("g_sAlcSample.nTotalIntegralCnt = %ld\r\n",g_sAlcSample.nTotalIntegralCnt);
		TRACE_PRINTF("g_sAlcSample.nSampleMaxValue = %ld\r\n",g_sAlcSample.nSampleMaxValue);
		
		g_eTestState = eDisplayResult;
		pTestWin->bNeedRefreshState = true;
	}
}

static void _FinishTest(WM_MESSAGE *pMsg)
{
	g_eTestState = eDisplayResult;
}

static void _DisplayResult(WM_MESSAGE *pMsg)
{
	WM_DeleteWindow(pMsg->hWin);
	g_sRecord.nTestMode = g_eTestMode;
	if(g_sRecord.nTestMode == eTestRefuseMode)
	{
		sInputWindowProperty s;
		s.nWinType = ePlateNumber;
		s.aCaptionText = _PlateNumberInput;
		CW_InputPlateNumWindow_Create(GetLastWinFromList(),(void *)&s);
	}
	else
	{
		CW_ShowReport_Create(GetLastWinFromList(),(void *)1);
	}
}

void Scan_Test_State(WM_MESSAGE *pMsg)
{
	switch(g_eTestState)
	{
		case eStart:
			_Start(pMsg);
			break;

		case ePrepare:
			_Prepare(pMsg);
			break;

		case eAlcClean:
			_AlcClean(pMsg);
			break;

		case eWaitStart:
			_WaitStart(pMsg);
			break;
				
		case eStartTestPrepare:
			_StartTestPrepare(pMsg);
			break;
	
		case eInhaling:
			_Inhaling(pMsg);
			break;

		case eWaitBlowing:
			_WaitBlowing(pMsg);
			break;

		case eBlowing:
			_Blowing(pMsg);
			break;

		case eBlowInterrupt:
			_BlowInterrupt(pMsg);
			break;

		case ePump:
			_Pump(pMsg);
			break;

		case eProcessing:
			_Processing(pMsg);
			break;

		case eFinishTest:
			_FinishTest(pMsg);
			break;
		
		case eDisplayResult:
			_DisplayResult(pMsg);
			break;
	}
}



