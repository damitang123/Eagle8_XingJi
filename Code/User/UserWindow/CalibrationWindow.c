#include "platform.h"
#include "Windows.h"
#include "calibrate.h"
#include "test.h"
#include "adc.h"
#include "data.h"
#include "system_misc.h"

static CW_Calibration *pThisWin;

static void _Updata_Temperature(WM_HWIN hWin)
{
	float fTemp;
	char aTemp[50];
	fTemp = Adc_Get_Sensor_Temperature();
	sprintf(aTemp, _TempCalibration, (int)fTemp, (int)(fTemp * 9 / 5 + 32));
	TEXT_SetText(hWin, aTemp);
}

static void GUI_Timer_Temperature_Callback(GUI_TIMER_MESSAGE *pTM)
{
	GUI_TIMER_Restart(pThisWin->hTMTemperature);
	WM_SendMessageNoPara((WM_HWIN)pTM->Context, WM_UPDATE_TEMPERATURE);
}

#if QUICK_TEST

static void _UpdateCalibrateValue(void)
{	
	char aTemp[50];
	char aResult[20];

	if(g_nCalibrateUnit == 0)
	{
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fHStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
		TEXT_SetText(pThisWin->hTextHighConcentration, aTemp);
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fLStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
		TEXT_SetText(pThisWin->hTextLowConcentration, aTemp);
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fPeekStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nPeekCaliValue);
		TEXT_SetText(pThisWin->hTextQuickTest, aTemp);
	}
	else if(g_nCalibrateUnit == 5)
	{
		float fTemp;
		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fHStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
		TEXT_SetText(pThisWin->hTextHighConcentration, aTemp);

		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fLStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
		TEXT_SetText(pThisWin->hTextLowConcentration, aTemp);

		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fPeekStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nPeekCaliValue);
		TEXT_SetText(pThisWin->hTextQuickTest, aTemp);
	}
}

static void _ConfirmRecoveryDefault(void)
{
	unsigned long nZeroIntergralValue;
	nZeroIntergralValue = g_sAlcCalibrationParam.nZeroIntergralValue;
	Spi_Flash_Init();
	
	Flash_Sector_Erase(FLASH_SECTOR_ALC_CALIBRATION * FLASH_SECTOR_SIZE);
	Data_AlcCalibration_Parameter_Load(&g_sAlcCalibrationParam);
	
	g_sAlcCalibrationParam.nZeroIntergralValue = nZeroIntergralValue;
	Data_AlcCalibration_Parameter_Save(&g_sAlcCalibrationParam);
	
	/*系数还原*/
	g_sTestParam.nAdjustCoefficientIntegral = 100;
	Data_Test_Parameter_Save(&g_sTestParam);
	Data_Test_Parameter_Load(&g_sTestParam);
	Spi_Flash_DeInit();
	_UpdateCalibrateValue();
}

/*高浓度校准相关*/
static void _StartHighCalibrate(void)
{
	g_eAlcTestPurpose = eTestForCalibration;
	g_sAlcCalibrationParam.nCurrCalibrteState = eCaliHighPoint;
	CW_TestMain_Create(pThisWin->wObj.hWin,(void *)eTestActive);
}

static WM_HWIN _CreateHighConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(10,  Caption_Height + 10 , 150, Bottom_Height - 4,
						    pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE, 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static WM_HWIN _CreateDefaultHighConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(170,  Caption_Height + 10 , 60, Bottom_Height - 4,
						    pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE, 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

/*低浓度校准相关*/
static void _StartLowCalibrate(void)
{
	g_eAlcTestPurpose = eTestForCalibration;
	g_sAlcCalibrationParam.nCurrCalibrteState = eCaliLowPoint;
	CW_TestMain_Create(pThisWin->wObj.hWin,(void *)eTestActive);
}

static WM_HWIN _CreateLowConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(10, Caption_Height + 80, 150, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static WM_HWIN _CreateDefaultLowConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(170, Caption_Height + 80, 60, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static WM_HWIN _CreateQuickTestButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(10, Caption_Height + 150, 150, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static WM_HWIN _CreateDefaultQuickTestButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(170, Caption_Height + 150, 60, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static void _StartQuickTestCalibrate(void)
{
	g_eAlcTestPurpose = eTestForCalibration;
	g_sAlcCalibrationParam.nCurrCalibrteState = eCaliQuickTest;
	CW_TestMain_Create(pThisWin->wObj.hWin,(void *)eTestQuickTest);
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	GUI_RECT rect;
	char aTemp[50];
	char aResult[20];

	/*校准单位*/
	g_nCalibrateUnit = 5;

	pMsg->Data.p = _Calibration;
	Create_Dialog_Caption(pMsg);

	/*创建左键*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	/*创建右键*/
	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _Default);

	/*创建中键*/
	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);


	

	/*创建高浓度标定显示*/
	pThisWin->hButtonHighConcentration = _CreateHighConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonHighConcentration, _HighConcentrationCali);

	pThisWin->hButtonDefaultHighConcentration = _CreateDefaultHighConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonDefaultHighConcentration, _Default);
	
	GUI_SetFont(&GUI_FontSongTiBold19);
	if(g_nCalibrateUnit == 0)
	{
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fHStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
	}
	else if(g_nCalibrateUnit == 5)
	{
		float fTemp;
		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fHStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
	}
	GUI_GetTextExtend(&rect, aTemp, 50);
	pThisWin->hTextHighConcentration = TEXT_CreateEx(5, Caption_Height + 45, LCD_XSIZE - 10, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(pThisWin->hTextHighConcentration, &GUI_FontSongTiBold19);
	TEXT_SetTextColor(pThisWin->hTextHighConcentration, GUI_DARKBLUE);
	TEXT_SetTextAlign(pThisWin->hTextHighConcentration, GUI_TA_CENTER);

	

	/*创建低浓度标定显示*/
	pThisWin->hButtonLowConcentration = _CreateLowConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonLowConcentration, _LowConcentrationCali);

	pThisWin->hButtonDefaultLowConcentration = _CreateDefaultLowConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonDefaultLowConcentration, _Default);
	
	GUI_SetFont(&GUI_FontSongTiBold19);
	if(g_nCalibrateUnit == 0)
	{
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fLStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
	}
	else if(g_nCalibrateUnit == 5)
	{
		float fTemp;
		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fLStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
	}
	GUI_GetTextExtend(&rect, aTemp, 50);
	pThisWin->hTextLowConcentration = TEXT_CreateEx(5, Caption_Height + 115, LCD_XSIZE - 10, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(pThisWin->hTextLowConcentration, &GUI_FontSongTiBold19);
	TEXT_SetTextColor(pThisWin->hTextLowConcentration, GUI_DARKBLUE);
	TEXT_SetTextAlign(pThisWin->hTextLowConcentration, GUI_TA_CENTER);


	
	/*创建快标定显示*/
	pThisWin->hButtonQuickTest = _CreateQuickTestButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonQuickTest, _QuickTestCali);

	pThisWin->hButtonDefaultQuickTest = _CreateDefaultQuickTestButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonDefaultQuickTest, _Default);
	
	GUI_SetFont(&GUI_FontSongTiBold19);
	if(g_nCalibrateUnit == 0)
	{
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fPeekStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nPeekCaliValue);
	}
	else if(g_nCalibrateUnit == 5)
	{
		float fTemp;
		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fPeekStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nPeekCaliValue);
	}
	GUI_GetTextExtend(&rect, aTemp, 50);
	pThisWin->hTextQuickTest = TEXT_CreateEx(5, Caption_Height + 185, LCD_XSIZE - 10, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(pThisWin->hTextQuickTest, &GUI_FontSongTiBold19);
	TEXT_SetTextColor(pThisWin->hTextQuickTest, GUI_DARKBLUE);
	TEXT_SetTextAlign(pThisWin->hTextQuickTest, GUI_TA_CENTER);


	/*创建显示温度*/
	GUI_SetFont(g_Language == eLanguage_Chinese ? &GUI_FontSongTiBold24 : &GUI_FontTahomaBold23);
	GUI_GetTextExtend(&rect, aTemp, 50);
	pThisWin->hEnvTemperature = TEXT_CreateEx(0, Caption_Height + 215, LCD_XSIZE, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(pThisWin->hEnvTemperature, g_Language == eLanguage_Chinese ? &GUI_FontSongTiBold24 : &GUI_FontTahomaBold23);
	TEXT_SetTextColor(pThisWin->hEnvTemperature, GUI_DARKRED);
	TEXT_SetTextAlign(pThisWin->hEnvTemperature, GUI_TA_CENTER);
	_Updata_Temperature(pThisWin->hEnvTemperature);

	pThisWin->hTMTemperature = GUI_TIMER_Create(GUI_Timer_Temperature_Callback, Get_System_Time(), (unsigned long)pMsg->hWin, 0);
	GUI_TIMER_SetPeriod(pThisWin->hTMTemperature, 500);

	WM_SetFocus(pMsg->hWin);
}

static void _Callback(WM_MESSAGE *pMsg)
{
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;
			
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:
						WM_DeleteWindow(pMsg->hWin);
						break;
						
					case GUI_KEY_RIGHT_FUN:
						{
							sDialogWindowProperty s;
							s.nWinType = eDialogYesNo;
							s.pFont = _GetFont(Font_Content);
							s.pContent = _DeleteCaliDataTip;
							s.nContentColor = GUI_RED;
							s.nBackGroundColor = GUI_LIGHTGRAY;
							s.nAutoCloseTime = 10;
							s.pFunLeftYes = _ConfirmRecoveryDefault;
							s.pFunRightNo = NULL;
							CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
						}
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						WM_DeleteWindow(pMsg->hWin);
						break;
				}
			}
			return;

		case WM_UPDATE_TEMPERATURE:
			_Updata_Temperature(pThisWin->hEnvTemperature);
			break;

		case WM_NOTIFY_PARENT:
			if(pMsg->Data.v == WM_NOTIFICATION_WIN_DELETE)
			{
				WM_SetFocus(pMsg->hWin);
				_UpdateCalibrateValue();
			}
			else if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
			{
				/*进入相关校准*/
				if(pMsg->hWinSrc == pThisWin->hButtonHighConcentration)
				{
					sInputWindowProperty s;
					s.nWinType = eInputStardardHighAlcValue;
					s.aCaptionText = _PleaseInputHighStandard;
					s.pFunNext = _StartHighCalibrate;
					CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
				}
				else if(pMsg->hWinSrc == pThisWin->hButtonLowConcentration)
				{
					sInputWindowProperty s;
					s.nWinType = eInputStardardLowAlcValue;
					s.aCaptionText = _PleaseInputLowStandard;
					s.pFunNext = _StartLowCalibrate;
					CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
				}
				else if(pMsg->hWinSrc == pThisWin->hButtonQuickTest)
				{
					sInputWindowProperty s;
					s.nWinType = eInputQuickTestAlcValue;
					s.aCaptionText = _PleaseInputQuickTestStandard;
					s.pFunNext = _StartQuickTestCalibrate;
					CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
				}

				/*恢复默认值*/
				if(pMsg->hWinSrc == pThisWin->hButtonDefaultHighConcentration)
				{
					g_sAlcCalibrationParam.nIntergralHighPoint = 11000000;
					g_sAlcCalibrationParam.nIntergralLowPoint = 11000000 / 4;
					g_sAlcCalibrationParam.fLStandardAlcoholValue = 20;
					g_sAlcCalibrationParam.fHStandardAlcoholValue = 80;
					g_sAlcCalibrationParam.fLRealTestAlcoholValue = 20;
					g_sAlcCalibrationParam.fHRealTestAlcoholValue = 80;
					_UpdateCalibrateValue();
				}
				else if(pMsg->hWinSrc == pThisWin->hButtonDefaultLowConcentration)
				{
					g_sAlcCalibrationParam.nIntergralLowPoint = 11000000 / 4;
					g_sAlcCalibrationParam.fLStandardAlcoholValue = 20;
					g_sAlcCalibrationParam.fLRealTestAlcoholValue = 20;
					_UpdateCalibrateValue();
				}
				else if(pMsg->hWinSrc == pThisWin->hButtonDefaultQuickTest)
				{
					g_sAlcCalibrationParam.nPeekCaliValue = 3160;
					g_sAlcCalibrationParam.fPeekStandardAlcoholValue = 40;
					g_sAlcCalibrationParam.fPeekRealTestAlcoholValue = 40;
					_UpdateCalibrateValue();
				}
			}
			return;
			
		case WM_DELETE:
			GUI_TIMER_Delete(pThisWin->hTMTemperature);
			pThisWin->hTMTemperature = NULL;
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eCalibrationWin window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}

#else

static void _UpdateCalibrateValue(void)
{	
	char aTemp[50];
	char aResult[20];

	if(g_nCalibrateUnit == 0)
	{
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fHStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
		TEXT_SetText(pThisWin->hTextHighConcentration, aTemp);
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fLStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
		TEXT_SetText(pThisWin->hTextLowConcentration, aTemp);
	}
	else if(g_nCalibrateUnit == 5)
	{
		float fTemp;
		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fHStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
		TEXT_SetText(pThisWin->hTextHighConcentration, aTemp);

		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fLStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
		TEXT_SetText(pThisWin->hTextLowConcentration, aTemp);
	}
}

static void _ConfirmRecoveryDefault(void)
{
	unsigned long nZeroIntergralValue;
	nZeroIntergralValue = g_sAlcCalibrationParam.nZeroIntergralValue;
	Spi_Flash_Init();
	
	Flash_Sector_Erase(FLASH_SECTOR_ALC_CALIBRATION * FLASH_SECTOR_SIZE);
	Data_AlcCalibration_Parameter_Load(&g_sAlcCalibrationParam);
	
	g_sAlcCalibrationParam.nZeroIntergralValue = nZeroIntergralValue;
	Data_AlcCalibration_Parameter_Save(&g_sAlcCalibrationParam);
	
	/*系数还原*/
	g_sTestParam.nAdjustCoefficientIntegral = 100;
	Data_Test_Parameter_Save(&g_sTestParam);
	Data_Test_Parameter_Load(&g_sTestParam);
	Spi_Flash_DeInit();
	_UpdateCalibrateValue();
}

/*高浓度校准相关*/
static void _StartHighCalibrate(void)
{
	g_eAlcTestPurpose = eTestForCalibration;
	g_sAlcCalibrationParam.nCurrCalibrteState = eCaliHighPoint;
	CW_TestMain_Create(pThisWin->wObj.hWin,(void *)eTestActive);
}

static WM_HWIN _CreateHighConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(10,  Caption_Height + 10 , 150, Bottom_Height - 4,
						    pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE, 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static WM_HWIN _CreateDefaultHighConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(170,  Caption_Height + 10 , 60, Bottom_Height - 4,
						    pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE, 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

/*低浓度校准相关*/
static void _StartLowCalibrate(void)
{
	g_eAlcTestPurpose = eTestForCalibration;
	g_sAlcCalibrationParam.nCurrCalibrteState = eCaliLowPoint;
	CW_TestMain_Create(pThisWin->wObj.hWin,(void *)eTestActive);
}

static WM_HWIN _CreateLowConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(10, Caption_Height + 110, 150, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static WM_HWIN _CreateDefaultLowConcentrationButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(170, Caption_Height + 110, 60, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	GUI_RECT rect;
	char aTemp[50];
	char aResult[20];

	/*校准单位*/
	g_nCalibrateUnit = 5;

	pMsg->Data.p = _Calibration;
	Create_Dialog_Caption(pMsg);

	/*创建左键*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	/*创建右键*/
	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _Default);

	/*创建中键*/
	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);


	

	/*创建高浓度标定显示*/
	pThisWin->hButtonHighConcentration = _CreateHighConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonHighConcentration, _HighConcentrationCali);

	pThisWin->hButtonDefaultHighConcentration = _CreateDefaultHighConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonDefaultHighConcentration, _Default);
	
	GUI_SetFont(&GUI_FontSongTiBold19);
	if(g_nCalibrateUnit == 0)
	{
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fHStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
	}
	else if(g_nCalibrateUnit == 5)
	{
		float fTemp;
		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fHStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralHighPoint);
	}
	GUI_GetTextExtend(&rect, aTemp, 50);
	pThisWin->hTextHighConcentration = TEXT_CreateEx(5, Caption_Height + 45, LCD_XSIZE - 10, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(pThisWin->hTextHighConcentration, &GUI_FontSongTiBold19);
	TEXT_SetTextColor(pThisWin->hTextHighConcentration, GUI_DARKBLUE);
	TEXT_SetTextAlign(pThisWin->hTextHighConcentration, GUI_TA_CENTER);

	

	/*创建低浓度标定显示*/
	pThisWin->hButtonLowConcentration = _CreateLowConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonLowConcentration, _LowConcentrationCali);

	pThisWin->hButtonDefaultLowConcentration = _CreateDefaultLowConcentrationButton(pMsg);
	BUTTON_SetText(pThisWin->hButtonDefaultLowConcentration, _Default);
	
	GUI_SetFont(&GUI_FontSongTiBold19);
	if(g_nCalibrateUnit == 0)
	{
		sprintf(aTemp, "%.1f%s-%ld", g_sAlcCalibrationParam.fLStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
	}
	else if(g_nCalibrateUnit == 5)
	{
		float fTemp;
		fTemp = Get_Final_Convert_Display_Result(g_sAlcCalibrationParam.fLStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
		Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
		sprintf(aTemp, "%s%s-%ld", aResult,Get_Unit_String(g_nCalibrateUnit),g_sAlcCalibrationParam.nUserIntergralLowPoint);
	}
	GUI_GetTextExtend(&rect, aTemp, 50);
	pThisWin->hTextLowConcentration = TEXT_CreateEx(5, Caption_Height + 145, LCD_XSIZE - 10, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(pThisWin->hTextLowConcentration, &GUI_FontSongTiBold19);
	TEXT_SetTextColor(pThisWin->hTextLowConcentration, GUI_DARKBLUE);
	TEXT_SetTextAlign(pThisWin->hTextLowConcentration, GUI_TA_CENTER);


	/*创建显示温度*/
	GUI_SetFont(g_Language == eLanguage_Chinese ? &GUI_FontSongTiBold24 : &GUI_FontTahomaBold23);
	GUI_GetTextExtend(&rect, aTemp, 50);
	pThisWin->hEnvTemperature = TEXT_CreateEx(0, Caption_Height + 200, LCD_XSIZE, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(pThisWin->hEnvTemperature, g_Language == eLanguage_Chinese ? &GUI_FontSongTiBold24 : &GUI_FontTahomaBold23);
	TEXT_SetTextColor(pThisWin->hEnvTemperature, GUI_DARKRED);
	TEXT_SetTextAlign(pThisWin->hEnvTemperature, GUI_TA_CENTER);
	_Updata_Temperature(pThisWin->hEnvTemperature);

	pThisWin->hTMTemperature = GUI_TIMER_Create(GUI_Timer_Temperature_Callback, Get_System_Time(), (unsigned long)pMsg->hWin, 0);
	GUI_TIMER_SetPeriod(pThisWin->hTMTemperature, 500);

	WM_SetFocus(pMsg->hWin);
}

static void _Callback(WM_MESSAGE *pMsg)
{
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;
			
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:
						WM_DeleteWindow(pMsg->hWin);
						break;
						
					case GUI_KEY_RIGHT_FUN:
						{
							sDialogWindowProperty s;
							s.nWinType = eDialogYesNo;
							s.pFont = _GetFont(Font_Content);
							s.pContent = _DeleteCaliDataTip;
							s.nContentColor = GUI_RED;
							s.nBackGroundColor = GUI_LIGHTGRAY;
							s.nAutoCloseTime = 10;
							s.pFunLeftYes = _ConfirmRecoveryDefault;
							s.pFunRightNo = NULL;
							CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
						}
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						WM_DeleteWindow(pMsg->hWin);
						break;
				}
			}
			return;

		case WM_UPDATE_TEMPERATURE:
			_Updata_Temperature(pThisWin->hEnvTemperature);
			break;

		case WM_NOTIFY_PARENT:
			if(pMsg->Data.v == WM_NOTIFICATION_WIN_DELETE)
			{
				WM_SetFocus(pMsg->hWin);
				_UpdateCalibrateValue();
			}
			else if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
			{
				/*进入相关校准*/
				if(pMsg->hWinSrc == pThisWin->hButtonHighConcentration)
				{
					sInputWindowProperty s;
					s.nWinType = eInputStardardHighAlcValue;
					s.aCaptionText = _PleaseInputHighStandard;
					s.pFunNext = _StartHighCalibrate;
					CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
				}
				else if(pMsg->hWinSrc == pThisWin->hButtonLowConcentration)
				{
					sInputWindowProperty s;
					s.nWinType = eInputStardardLowAlcValue;
					s.aCaptionText = _PleaseInputLowStandard;
					s.pFunNext = _StartLowCalibrate;
					CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
				}

				/*恢复默认值*/
				if(pMsg->hWinSrc == pThisWin->hButtonDefaultHighConcentration)
				{
					g_sAlcCalibrationParam.nIntergralHighPoint = 11000000;
					g_sAlcCalibrationParam.nIntergralLowPoint = 11000000 / 4;
					g_sAlcCalibrationParam.fLStandardAlcoholValue = 20;
					g_sAlcCalibrationParam.fHStandardAlcoholValue = 80;
					g_sAlcCalibrationParam.fLRealTestAlcoholValue = 20;
					g_sAlcCalibrationParam.fHRealTestAlcoholValue = 80;
					_UpdateCalibrateValue();
				}
				else if(pMsg->hWinSrc == pThisWin->hButtonDefaultLowConcentration)
				{
					g_sAlcCalibrationParam.nIntergralLowPoint = 11000000 / 4;
					g_sAlcCalibrationParam.fLStandardAlcoholValue = 20;
					g_sAlcCalibrationParam.fLRealTestAlcoholValue = 20;
					_UpdateCalibrateValue();
				}
			}
			return;
			
		case WM_DELETE:
			GUI_TIMER_Delete(pThisWin->hTMTemperature);
			pThisWin->hTMTemperature = NULL;
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eCalibrationWin window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


#endif


CW_Calibration *CW_Calibration_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eCalibrationWin;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eCalibrationWin window is alreay exist!\r\n");
		return (CW_Calibration *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_Calibration));

	if(hWin)
	{
		TRACE_PRINTF("eCalibrationWin windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_Calibration *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eCalibrationWin window can't be created!\r\n");
	}

	return pThisWin;
}



