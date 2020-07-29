#include "platform.h"
#include "Windows.h"
#include "PROGBAR.h"
#include "test.h"
#include "adc.h"
#include "gpio.h"
#include "buzzer.h"
#include "data.h"
#include "rtc.h"
#include "system_misc.h"
#include "gps.h"
#include "sample.h"
#include "calibrate.h"
#include "app.h"

CW_TestMain *pTestWin = NULL;

void Scan_Test_State(WM_MESSAGE *pMsg);
void Scan_Quick_Test_State(WM_MESSAGE *pMsg);

static void Record_Basic_Init(void)
{
	g_sRecord.nRecordIndex = g_nNewRecordID;
	g_sRecord.nTemperature = pTestWin->fAlcSampleTemperature;
	g_sRecord.fAlcValue = g_fAlcoholVolume;
	g_sRecord.nIntergralValue = g_sAlcSample.nIntegralSumValue;
	*((sDateTime *)g_sRecord.aDateTime) = g_sDateTime;
	g_sRecord.nTestUnit = g_sTestParam.nTestUnit;
	g_sRecord.nKBrACBAC = g_sTestParam.nBrACBACK;
	g_sRecord.nAdjustCoefficient = g_sTestParam.nAdjustCoefficientIntegral;
	g_sRecord.nTestResult = 0;

	g_sRecord.fLongitude =  g_bGPS_Location_Valid == 1  ? g_fLongitude : 0;
	g_sRecord.fLatitude =  g_bGPS_Location_Valid == 1  ? g_fLatitude: 0;

	memcpy(g_sRecord.aLocation, g_sDeviceUserInfo.aLocation, sizeof(g_sRecord.aLocation));
	memcpy(g_sRecord.aPoliceID, g_sDeviceUserInfo.aPoliceID, sizeof(g_sRecord.aPoliceID));
	memcpy(g_sRecord.aPoliceName, g_sDeviceUserInfo.aPoliceName, sizeof(g_sRecord.aPoliceName));
	memcpy(g_sRecord.aDepartment, g_sDeviceUserInfo.aDepartment, sizeof(g_sRecord.aDepartment));
	
	memset(g_sRecord.aReserved, 0, sizeof(g_sRecord.aReserved));
}

static void GUI_Timer_Test_Callback(GUI_TIMER_MESSAGE *pTM)
{
	GUI_TIMER_Restart(pTestWin->hTMTest);
	WM_SendMessageNoPara((WM_HWIN)pTM->Context, WM_ALCOHOL_TEST);
}

#if QUICK_TEST
// 坐标位置
#define COORDINATE_X0	20
#define COORDINATE_Y0	200
#define COORDINATE_X_WITH		200
#define COORDINATE_Y_HIGH		180

//X 轴坐标刻度份数
#define COORDINATE_GRID_X_NUMBER		((unsigned short)(2 * g_sTestParam.nInhaleTime / 1000))
//Y 轴坐标刻度份数
#define COORDINATE_GRID_Y_NUMBER		4

//X 轴坐标刻度间隔
#define COORDINATE_GRID_X_WITH		((float)COORDINATE_X_WITH/COORDINATE_GRID_X_NUMBER)
#define COORDINATE_GRID_Y_WITH		((float)COORDINATE_Y_HIGH/COORDINATE_GRID_Y_NUMBER)

unsigned char Get_Curve_Windows_Height(void)
{
	return COORDINATE_Y_HIGH;
}

unsigned char Get_Curve_Windows_Width(void)
{
	return COORDINATE_X_WITH;
}

void _Draw_Grid(void)
{
	int i;
	
	//x,y 轴
	LCD_SetColor(0x0000FF);
	GUI_DrawVLine(COORDINATE_X0, COORDINATE_Y0-COORDINATE_Y_HIGH-10, COORDINATE_Y0);
	GUI_DrawHLine(COORDINATE_Y0, COORDINATE_X0, COORDINATE_X0+COORDINATE_X_WITH+10);

	//坐标箭头
	GUI_DrawLine(COORDINATE_X0,COORDINATE_Y0-COORDINATE_Y_HIGH-10,COORDINATE_X0+5,COORDINATE_Y0-COORDINATE_Y_HIGH-5);
	GUI_DrawLine(COORDINATE_X0,COORDINATE_Y0-COORDINATE_Y_HIGH-10,COORDINATE_X0-5,COORDINATE_Y0-COORDINATE_Y_HIGH-5);
	GUI_DrawLine(COORDINATE_X0+COORDINATE_X_WITH+10,COORDINATE_Y0,COORDINATE_X0+COORDINATE_X_WITH+5,COORDINATE_Y0+5);
	GUI_DrawLine(COORDINATE_X0+COORDINATE_X_WITH+10,COORDINATE_Y0,COORDINATE_X0+COORDINATE_X_WITH+5,COORDINATE_Y0-5);

	//中间网格
	for(i =1; i<=COORDINATE_GRID_Y_NUMBER ;i++)
	{
		GUI_DrawHLine(COORDINATE_Y0-(COORDINATE_GRID_Y_WITH*i),COORDINATE_X0,  COORDINATE_X0+COORDINATE_X_WITH);
	}
	for(i =1; i<COORDINATE_GRID_X_NUMBER ;i++)
	{
		GUI_DrawVLine(COORDINATE_X0+(i*COORDINATE_GRID_X_WITH), COORDINATE_Y0-COORDINATE_Y_HIGH, COORDINATE_Y0);
	}
	GUI_DrawVLine(COORDINATE_X0+COORDINATE_X_WITH, COORDINATE_Y0-COORDINATE_Y_HIGH, COORDINATE_Y0);
}

static void _CallBack_QuickTest(WM_MESSAGE *pMsg)
{
	if(pMsg->MsgId == WM_PAINT)
	{
		GUI_SetBkColor(BK_COLOR_CLIENT);
		GUI_Clear();
		_Draw_Grid();
	}
}
#endif

static void _Create(WM_MESSAGE *pMsg)
{
	pMsg->Data.p = " ";
	pTestWin->hCaption = Create_Dialog_Caption(pMsg);

	/*创建左键*/
	pTestWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(pTestWin->hLeftButton , __Exit);

	/*中间提示语*/
	pTestWin->hTestTips = TEXT_CreateEx(5, Caption_Height + 20, LCD_XSIZE - 10, 230,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, "");
	TEXT_SetWrapMode(pTestWin->hTestTips, GUI_WRAPMODE_CHAR);
	TEXT_SetTextColor(pTestWin->hTestTips, GUI_BLUE);
	TEXT_SetFont(pTestWin->hTestTips, _GetFont(Font_Content));

	/*创建滚动条*/
	pTestWin->hTimeProgressBar = PROGBAR_CreateEx(20, (LCD_YSIZE - Bottom_Height - 22 - 15), LCD_XSIZE - 40, 22, pMsg->hWin, WM_CF_SHOW, 0, 0);
	PROGBAR_SetFont(pTestWin->hTimeProgressBar, &GUI_Font16_UTF8);
	PROGBAR_SetBarColor(pTestWin->hTimeProgressBar, 0, GUI_GREEN);
	PROGBAR_SetBarColor(pTestWin->hTimeProgressBar, 1, GUI_GRAY);
	PROGBAR_SetText(pTestWin->hTimeProgressBar, "TIME : 0%");

#if USE_GB_T21254_2017
	pTestWin->hBlowFlowVolume = PROGBAR_CreateEx(20, (LCD_YSIZE - Bottom_Height - 22 - 15 - 35), LCD_XSIZE - 40, 22, pMsg->hWin, WM_CF_SHOW, 0, 0);
	PROGBAR_SetFont(pTestWin->hBlowFlowVolume, &GUI_Font16_UTF8);
	PROGBAR_SetBarColor(pTestWin->hBlowFlowVolume, 0, GUI_GREEN);
	PROGBAR_SetBarColor(pTestWin->hBlowFlowVolume, 1, GUI_GRAY);
	PROGBAR_SetText(pTestWin->hBlowFlowVolume, "FLOW : 0%");
#endif

#if QUICK_TEST
	pTestWin->hQuickTestDraw = WM_CreateWindowAsChild(QUICK_TEST_WINDOW_X0,QUICK_TEST_WINDOW_Y0,
										QUICK_TEST_WINDOW_XWITH, QUICK_TEST_WINDOW_YHIGH,
										pMsg->hWin, 
										WM_CF_HIDE, _CallBack_QuickTest,0);
	pTestWin->hQuickTestDisplayTime = TEXT_CreateEx(150, 265, 80, 25,pMsg->hWin,WM_CF_HIDE, 0, 0, NULL);
	TEXT_SetFont(pTestWin->hQuickTestDisplayTime, &GUI_FontTahomaBold23);
	TEXT_SetTextColor(pTestWin->hQuickTestDisplayTime, GUI_DARKBLUE);
	TEXT_SetTextAlign(pTestWin->hQuickTestDisplayTime, GUI_TA_LEFT);
	pTestWin->hQuickTestDisplayValue = TEXT_CreateEx(20, Caption_Height, LCD_XSIZE, 25,pMsg->hWin,WM_CF_HIDE, 0, 0, NULL);
	TEXT_SetFont(pTestWin->hQuickTestDisplayValue, &GUI_FontTahomaBold23);
	TEXT_SetTextColor(pTestWin->hQuickTestDisplayValue, GUI_DARKBLUE);
	TEXT_SetTextAlign(pTestWin->hQuickTestDisplayValue, GUI_TA_LEFT);
#endif

	if(g_eAlcTestPurpose == eTestForCalibration)
	{
		char aTemp[30];
		char aResult[20];
		if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliLowPoint)
		{
			if(g_nCalibrateUnit == 0)
			{
				sprintf(aResult, "%.1f", g_fLStandardAlcoholValue);
			}
			else if(g_nCalibrateUnit == 5)
			{
				float fTemp;
				fTemp = Get_Final_Convert_Display_Result(g_fLStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
				Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
			}
			sprintf(aTemp, "%s(%s%s)",_Calibration, aResult,Get_Unit_String(g_nCalibrateUnit));
		}
		else if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliHighPoint)
		{
			if(g_nCalibrateUnit == 0)
			{
				sprintf(aResult, "%.1f", g_fHStandardAlcoholValue);
			}
			else if(g_nCalibrateUnit == 5)
			{
				float fTemp;
				fTemp = Get_Final_Convert_Display_Result(g_fHStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
				Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
			}
			sprintf(aTemp, "%s(%s%s)",_Calibration, aResult,Get_Unit_String(g_nCalibrateUnit));
		}
		else if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliQuickTest)
		{
			if(g_nCalibrateUnit == 0)
			{
				sprintf(aResult, "%.1f", g_fQuickTestStandardAlcoholValue);
			}
			else if(g_nCalibrateUnit == 5)
			{
				float fTemp;
				fTemp = Get_Final_Convert_Display_Result(g_fQuickTestStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
				Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
			}
			sprintf(aTemp, "%s(%s%s)",_Calibration, aResult,Get_Unit_String(g_nCalibrateUnit));
		}
		TEXT_SetText(pTestWin->hCaption, aTemp);

		/*创建右键*/
		pTestWin->hRightButton = Create_Right_Button(pMsg->hWin);
		BUTTON_SetText(pTestWin->hRightButton, _Retest);

		/*创建中键*/
		pTestWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
		BUTTON_SetText(pTestWin->hMiddleButton, _OK);
	}
	else if(g_eAlcTestPurpose == eTestForNormal)
	{
		TEXT_SetText(pTestWin->hCaption, _AlcoholTest);
	#if QUICK_TEST
		/*创建右键*/
		pTestWin->hRightButton = Create_Right_Button(pMsg->hWin);
		BUTTON_SetText(pTestWin->hRightButton, _QuickTest);
	#else
		/*创建右键*/
		pTestWin->hRightButton = Create_Right_Button(pMsg->hWin);
		BUTTON_SetText(pTestWin->hRightButton, _Refuse);
	#endif

		/*创建中键*/
		pTestWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
		BUTTON_SetText(pTestWin->hMiddleButton,  _Passive);
	}
	else if(g_eAlcTestPurpose == eTestForMaintain)
	{
		TEXT_SetText(pTestWin->hCaption, _AlcoholTest);
		/*创建右键*/
		pTestWin->hRightButton = Create_Right_Button(pMsg->hWin);
		BUTTON_SetText(pTestWin->hRightButton, _Retest);

		/*创建中键*/
		pTestWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
		BUTTON_SetText(pTestWin->hMiddleButton,  _Passive);
	}

	pApp->bNewDataNeedSave = false;
	memset(&g_sRecord, 0, sizeof(sRecord));
	pTestWin->hTMTest = GUI_TIMER_Create(GUI_Timer_Test_Callback, Get_System_Time() + 10, (unsigned long)pMsg->hWin, 0);
	GUI_TIMER_SetPeriod(pTestWin->hTMTest, 10);
	
	g_eTestState = eStart;
	g_eTestMode = eTestActive;
	WM_SetFocus(pMsg->hWin);
	
	g_fAlcoholVolume = 0;
}

static void _Callback(WM_MESSAGE *pMsg)
{
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;

		case WM_PAINT:
			break;
			
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				if(g_eTestMode == eTestActive || g_eTestMode == eTestPassive || g_eTestMode == eTestRefuseMode)
				{
					Scan_Test_State(pMsg);
				}
				else if(g_eTestMode == eTestQuickTest)
				{
					Scan_Quick_Test_State(pMsg);
				}
			}
			return;
			
		case WM_ALCOHOL_TEST:
			if(g_eTestMode == eTestActive || g_eTestMode == eTestPassive || g_eTestMode == eTestRefuseMode)
			{
				Scan_Test_State(pMsg);
				
			}
			else if(g_eTestMode == eTestQuickTest)
			{
				Scan_Quick_Test_State(pMsg);
			}
			break;

		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pMsg->hWin);
					break;
			}
			break;

		case WM_DELETE:
			Voc_Close();
			Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,true);
			Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl,true);
			Gpio_Ctrl(eGpio_Pump_Charge_Sw_Ctrl,false);
			Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,false);
			Gpio_Ctrl(eGpio_Moto_Ctrl,false);
			Record_Basic_Init();
			g_bSpiHardwareBusBusy = false;
			GUI_TIMER_Delete(pTestWin->hTMTest);
			pTestWin->hTMTest = NULL;
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eTestMain window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_TestMain *CW_TestMain_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eTestMain;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eTestMain window is alreay exist!\r\n");
		return (CW_TestMain *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_TestMain));

	if(hWin)
	{
		TRACE_PRINTF("eTestMain windows is created!hWin = 0x%04X\r\n", hWin);

		pTestWin = (CW_TestMain *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pTestWin->wObj.hWin = hWin;
		pTestWin->wObj.Property = (unsigned long)pData;
		pTestWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pTestWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eTestMain window can't be created!\r\n");
	}

	return pTestWin;
}

