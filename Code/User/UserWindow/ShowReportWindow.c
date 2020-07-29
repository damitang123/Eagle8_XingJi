#include "platform.h"
#include "Windows.h"
#include "parameter.h"
#include "data.h"
#include "calibrate.h"
#include "test.h"
#include "sample.h"
#include "buzzer.h"
#include "sample.h"
#include "app.h"

#include "gprs.h"
#include "gprs_user.h"


static CW_ShowReport *pThisWin;


static bool Save_Calibrate_Result(void)
{	
	float fTemp;
	if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliLowPoint)
	{
		fTemp = (float)g_sAlcSample.nIntegralSumValue * ((float)20.0 / (float)g_fLStandardAlcoholValue);
		if(fTemp < 1000000 || fTemp > 30000000)
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_Content);
			s.pContent = _InvalidCaliData;
			s.nContentColor = GUI_RED;
			s.nBackGroundColor = GUI_LIGHTGRAY;
			s.nAutoCloseTime = 3;
			s.pFunMiddleOK = NULL;
			CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			return false;
		}
		else
		{
			g_sAlcCalibrationParam.nIntergralLowPoint = fTemp;
			g_sAlcCalibrationParam.fLStandardAlcoholValue = g_fLStandardAlcoholValue;
			g_sAlcCalibrationParam.nUserIntergralLowPoint = g_sAlcSample.nIntegralSumValue;
		}
		memcpy(g_sAlcCalibrationParam.aDateTimeIntergral, (unsigned char *)&g_sDateTime, sizeof(sDateTime));
	}
	else if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliHighPoint)
	{
		/*积分*/
		fTemp = (float)g_sAlcSample.nIntegralSumValue * ((float)80.0 / (float)g_fHStandardAlcoholValue);
		if(fTemp < 1000000 || fTemp > 30000000)
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_Content);
			s.pContent = _InvalidCaliData;
			s.nContentColor = GUI_RED;
			s.nBackGroundColor = GUI_LIGHTGRAY;
			s.nAutoCloseTime = 3;
			s.pFunMiddleOK = NULL;
			CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			return false;
		}	
		else
		{
			g_sAlcCalibrationParam.nIntergralHighPoint = fTemp;
			g_sAlcCalibrationParam.nIntergralLowPoint = g_sAlcCalibrationParam.nIntergralHighPoint / 4.04;
			g_sAlcCalibrationParam.fHStandardAlcoholValue = g_fHStandardAlcoholValue;
			g_sAlcCalibrationParam.nUserIntergralHighPoint = g_sAlcSample.nIntegralSumValue;

			/*刷新低浓度标定值*/
			g_sAlcCalibrationParam.fLStandardAlcoholValue = 20;
			g_sAlcCalibrationParam.nUserIntergralLowPoint = g_sAlcCalibrationParam.nIntergralLowPoint;
		}
		memcpy(g_sAlcCalibrationParam.aDateTimeIntergral, (unsigned char *)&g_sDateTime, sizeof(sDateTime));
	}
	else if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliQuickTest)
	{
		fTemp = (float)g_sAlcCalibrationParam.nPeekCaliValue * ((float)g_fAlcoholVolume / (float)g_fQuickTestStandardAlcoholValue);
		if(fTemp < 1000 || fTemp > 4095)
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_Content);
			s.pContent = _InvalidCaliData;
			s.nContentColor = GUI_RED;
			s.nBackGroundColor = GUI_LIGHTGRAY;
			s.nAutoCloseTime = 3;
			s.pFunMiddleOK = NULL;
			CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			return false;
		}	
		else
		{
			g_sAlcCalibrationParam.nPeekCaliValue = fTemp;
		}
		memcpy(g_sAlcCalibrationParam.aDateTimeIntergral, (unsigned char *)&g_sDateTime, sizeof(sDateTime));
	}
	
	/*还原标定中的值为默认值.*/
	g_sAlcCalibrationParam.fLStandardAlcoholValue = 20;
	g_sAlcCalibrationParam.fHStandardAlcoholValue = 80;
	g_sAlcCalibrationParam.fLRealTestAlcoholValue = 20;
	g_sAlcCalibrationParam.fHRealTestAlcoholValue = 80;

	Spi_Flash_Init();
	Data_AlcCalibration_Parameter_Save(&g_sAlcCalibrationParam);
	Data_AlcCalibration_Parameter_Load(&g_sAlcCalibrationParam);
	g_sTestParam.nAdjustCoefficientIntegral = 100;
	Data_Test_Parameter_Save(&g_sTestParam);
	Data_Test_Parameter_Load(&g_sTestParam);
	Spi_Flash_DeInit();
	
	return true;
}
/*标定相关的函数--End*/

static void Display_AlcoholTest_Result(WM_MESSAGE *pMsg,sRecord *pRecord)
{
	GUI_RECT rect;
	WM_HWIN hWin;
	float fAlcUnitConvertValue;
	char *pStr = NULL;
	char aTemp[30];
	
	/*1.显示Brac或者BAC*/
	GUI_SetFont(&GUI_FontTahomaBold33);
	pStr = pRecord->nTestUnit ? "BrAC" : "BAC";
	GUI_GetTextExtend(&rect, pStr, 20);
	hWin = TEXT_CreateEx(5, 60, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin, WM_CF_SHOW, 0, 0, pStr);
	TEXT_SetFont(hWin, &GUI_FontTahomaBold33);
	TEXT_SetTextColor(hWin, GUI_WHITE);

	/*2显示值*/
	if(pRecord->fAlcValue > MAX_DISPLAY_ALC_VALUE && (pRecord->nTestMode == eTestActive || pRecord->nTestMode == eTestPassive))
	{
		aTemp[0] = '>';
		fAlcUnitConvertValue = Get_Final_Convert_Display_Result(MAX_DISPLAY_ALC_VALUE,pRecord->nTestUnit, pRecord->nKBrACBAC);
		if(g_eAlcTestPurpose == eTestForNormal)
		{
			Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, &aTemp[1], eFormatToInt);
		}
		else if(g_eAlcTestPurpose == eTestForMaintain)
		{
			Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, &aTemp[1], eFormatToFloat);
		}
	}
	else if(pRecord->fAlcValue > MAX_QUICK_TEST_DISPLAY_ALC_VALUE && pRecord->nTestMode == eTestQuickTest)
	{
		aTemp[0] = '>';
		fAlcUnitConvertValue = Get_Final_Convert_Display_Result(MAX_QUICK_TEST_DISPLAY_ALC_VALUE,pRecord->nTestUnit, pRecord->nKBrACBAC);
		if(g_eAlcTestPurpose == eTestForNormal)
		{
			Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, &aTemp[1], eFormatToInt);
		}
		else if(g_eAlcTestPurpose == eTestForMaintain)
		{
			Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, &aTemp[1], eFormatToFloat);
		}
	}
	else
	{
		fAlcUnitConvertValue = Get_Final_Convert_Display_Result(pRecord->fAlcValue,pRecord->nTestUnit, pRecord->nKBrACBAC);
		if(g_eAlcTestPurpose == eTestForNormal)
		{
			Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, aTemp,eFormatToInt);
		}
		else if(g_eAlcTestPurpose == eTestForMaintain)
		{
			Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, aTemp,eFormatToFloat);
		}
	}
	GUI_SetFont(&GUI_FontTimesNewRoman73);
	GUI_GetTextExtend(&rect, aTemp, 20);
	hWin = TEXT_CreateEx((LCD_XSIZE - (rect.x1 - rect.x0 + 1)) >> 1, (LCD_YSIZE - (rect.y1 - rect.y0 + 1)) >> 1, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin, WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(hWin, &GUI_FontTimesNewRoman73);
	TEXT_SetTextColor(hWin, GUI_WHITE);

	if(pRecord->fAlcValue < 20)
	{
		Voc_Cmd(eVOC_RESULT_PASS);
	}
	else if(pRecord->fAlcValue >= 20 && pRecord->fAlcValue < 80)
	{
		Voc_Cmd(eVOC_RESULT_DRINK);
	}
	else
	{
		Voc_Cmd(eVOC_RESULT_DRUNK);
	}

	/*3显示单位*/
	GUI_SetFont(&GUI_FontTahomaBold33);
	pStr = Get_Unit_String(pRecord->nTestUnit);
	GUI_GetTextExtend(&rect, pStr, 20);
	hWin = TEXT_CreateEx(LCD_XSIZE - (rect.x1 - rect.x0 + 1) - 5, 240, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin, WM_CF_SHOW, 0, 0, pStr);
	TEXT_SetFont(hWin, &GUI_FontTahomaBold33);
	TEXT_SetTextColor(hWin, GUI_WHITE);
}

void Display_Calibration_Result(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	char aTemp[15];
	GUI_RECT rect;
	GUI_SetFont(&GUI_FontTahomaBold33);
	if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliHighPoint || g_sAlcCalibrationParam.nCurrCalibrteState == eCaliLowPoint)
	{
		sprintf(aTemp, "%d", g_sAlcSample.nIntegralSumValue);
	}
	else if(g_sAlcCalibrationParam.nCurrCalibrteState == eCaliQuickTest)
	{
		sprintf(aTemp, "%d", (unsigned short)((float)g_sAlcCalibrationParam.nPeekCaliValue * ((float)g_fAlcoholVolume / (float)g_fQuickTestStandardAlcoholValue)));
	}
	GUI_GetTextExtend(&rect, aTemp, 20);
	hWin = TEXT_CreateEx((LCD_XSIZE - (rect.x1 - rect.x0 + 1)) >> 1, (LCD_YSIZE - (rect.y1 - rect.y0 + 1)) >> 1,
	                     rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin, WM_CF_SHOW, 0, 0, aTemp);
	TEXT_SetFont(hWin, &GUI_FontTahomaBold33);
	TEXT_SetTextColor(hWin, GUI_BLACK);
}

void Execute_Action_After_Test_Or_ShowReport(WM_MESSAGE *pMsg)
{
	sInputWindowProperty s;
	s.nWinType = ePlateNumber;
	s.aCaptionText = _PlateNumberInput;
	if(g_sRecord.nTestMode == eTestRefuseMode)
	{
		WM_DeleteWindow(pMsg->hWin);
	}
	CW_InputPlateNumWindow_Create(GetLastWinFromList(),(void *)&s);
}

static void _Create(WM_MESSAGE *pMsg)
{
	pMsg->Data.p = " ";
	pThisWin->hCaption = Create_Dialog_Caption(pMsg);

	#if SERVER_USE_XING_JIKONG_GU
	if(g_eAlcTestPurpose == eTestForNormal)
	{
         	if(g_sRecord.fAlcValue<20)
         	{
			/*创建左键*/
			pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
			BUTTON_SetText(pThisWin->hLeftButton, __Exit);

			/*创建右键*/
			pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
			BUTTON_SetText(pThisWin->hRightButton, _Retest);
		}
	}
	else
	{
		/*创建左键*/
		pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
		BUTTON_SetText(pThisWin->hLeftButton, __Exit);

		/*创建右键*/
		pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
		BUTTON_SetText(pThisWin->hRightButton, _Retest);
	}
	#else
	pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hLeftButton, __Exit);

	/*创建右键*/
	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hRightButton, _Retest);
	#endif

	/*创建中键*/
	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);

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
		Set_Dialog_Caption_Content(aTemp, pThisWin->hCaption);
		Display_Calibration_Result(pMsg);
		BUTTON_SetText(pThisWin->hMiddleButton, _Save);
	}
	else if(g_eAlcTestPurpose == eTestForNormal)
	{
		pApp->bNewDataNeedSave = true;
		if(g_sRecord.nTestMode == eTestRefuseMode)
		{
			Execute_Action_After_Test_Or_ShowReport(pMsg);
			return;
		}
		else
		{
			Set_Dialog_Caption_Content(_TestReport,pThisWin->hCaption);
			Display_AlcoholTest_Result(pMsg,&g_sRecord);
			BUTTON_SetText(pThisWin->hMiddleButton, _OK);
		}
	}
	else if(g_eAlcTestPurpose == eTestForMaintain)
	{
		Set_Dialog_Caption_Content(_TestReport,pThisWin->hCaption);
		g_sRecord.nTestUnit = 5;
		Display_AlcoholTest_Result(pMsg,&g_sRecord);
		BUTTON_SetText(pThisWin->hMiddleButton, _OK);
	}

	Buzzer_Beep(eBUZ_REPORT);

	#if SERVER_USE_XING_JIKONG_GU
	// 如果有网且记录信息上传完毕，方可上传
	if((g_sXingJiKongGU.bReveiceAckFromServer==false)&&g_sXingJiKongGU.bSingal_Strength)// 如果有网且记录信息上传完毕，方可上传
	{
		memset(GPRS_Tx_Buffer,0,sizeof(GPRS_Tx_Buffer));
        	g_nGprs_Tx_Cnt=Get_GPRS_To_Server_Result_Packet(GPRS_Tx_Buffer);
        	g_sXingJiKongGU.bResult_Packet_Flag=true;//g_Result_Packet_Flag=true;//有结果数据包上传 20180815
        	
       		TRACE_PRINTF("GPRS_Tx_Buffer:%s\r\n",GPRS_Tx_Buffer);
    		TRACE_PRINTF("length:%d\r\n",g_nGprs_Tx_Cnt);
    	}
    	else
    	{
    		g_sXingJiKongGU.bResult_Packet_Flag=false;//g_Result_Packet_Flag=false;
    	}

    	if(g_sRecord.fAlcValue>=20) //酒精浓度标志位 超标后则不能手动关机也不能进行超时关机
    	{
    		g_sXingJiKongGU.bAlcValue_Excessive_Flag=true;//g_nAlcValue_Excessive_Flag=true;
    	}
	#endif
	
	/*更新测试次数*/
	Spi_Flash_Init();
	g_nTotalTestCounter++;
	Data_Total_Test_Counter_Save(&g_nTotalTestCounter);
	Data_Total_Test_Counter_Load(&g_nTotalTestCounter);
	Spi_Flash_DeInit();

	/*更新总的抽气时间*/
	Spi_Flash_Init();
	g_fTotalInhaleTimeCounter += (float)g_sAlcSample.nTotalIntegralCnt / 1000;
	Data_Total_InhaleTime_Counter_Save(&g_fTotalInhaleTimeCounter);
	Data_Total_InhaleTime_Counter_Load(&g_fTotalInhaleTimeCounter);
	Spi_Flash_DeInit();

	
	
	WM_SetFocus(pMsg->hWin);
}

#if SERVER_USE_XING_JIKONG_GU
static void XingJiKongGu_AlcoholValue_Analyse_Left(WM_MESSAGE *pMsg)
{
	if(g_eAlcTestPurpose == eTestForNormal)
	{
		if(g_sRecord.fAlcValue<20)
		 {
			#if ALWAYS_SAVE_TEST_RESULT //小于20按左键退出保存，但大于20的需要到录入信息的最后面方可
			Data_Record_Full_Save();//added by lxl 20180817
			#endif
			WM_DeleteWindow(pMsg->hWin);
		}
			 
	}
	else
	WM_DeleteWindow(pMsg->hWin);
}

static void XingJiKongGu_AlcoholValue_Analyse_Right(WM_MESSAGE *pMsg)
{
	if(g_eAlcTestPurpose == eTestForNormal)
	{
		if(g_sRecord.fAlcValue<20)	//小于20按左键退出保存，否则无效
		{
		#if ALWAYS_SAVE_TEST_RESULT 
				Data_Record_Full_Save();
		#endif
			WM_DeleteWindow(pMsg->hWin);
			CW_TestMain_Create(GetLastWinFromList(),(void *)g_eTestMode);
		}
	}
	else
	{
		WM_DeleteWindow(pMsg->hWin);
		CW_TestMain_Create(GetLastWinFromList(),(void *)g_eTestMode);
	}

}
#endif

static void _Callback(WM_MESSAGE *pMsg)
{
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;

		case WM_PAINT:
			if(g_eAlcTestPurpose == eTestForNormal || g_eAlcTestPurpose == eTestForMaintain)
			{
				if(g_sRecord.fAlcValue < 20)
				{
					GUI_SetColor(GUI_DARKGREEN);
				}
				else if(g_sRecord.fAlcValue >= 20 && g_sRecord.fAlcValue < 80)
				{
					GUI_SetColor(_RGB(255, 128, 64));
				}
				else
				{
					GUI_SetColor(GUI_DARKRED);
				}
				GUI_FillRect(0, Caption_Height, LCD_XSIZE - 1, LCD_YSIZE - 1 - Bottom_Height);
			}
			break;
			
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:
					     #if SERVER_USE_XING_JIKONG_GU
						XingJiKongGu_AlcoholValue_Analyse_Left(pMsg);
					    #else
					    	#if ALWAYS_SAVE_TEST_RESULT
						Data_Record_Full_Save();
						#endif
						WM_DeleteWindow(pMsg->hWin);
					    #endif
						break;
						
					case GUI_KEY_RIGHT_FUN:
						#if SERVER_USE_XING_JIKONG_GU
							XingJiKongGu_AlcoholValue_Analyse_Right(pMsg);
						#else
							#if ALWAYS_SAVE_TEST_RESULT
								Data_Record_Full_Save();
							#endif
						WM_DeleteWindow(pMsg->hWin);
						CW_TestMain_Create(GetLastWinFromList(),(void *)g_eTestMode);
						#endif
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						if(g_eAlcTestPurpose == eTestForCalibration)
						{
							if(Save_Calibrate_Result())
							{
								WM_DeleteWindow(pMsg->hWin);
							}
						}
						else if(g_eAlcTestPurpose == eTestForNormal)
						{
							Execute_Action_After_Test_Or_ShowReport(pMsg);
						}
						else if(g_eAlcTestPurpose == eTestForMaintain)
						{
							/*保存记录*/
							pApp->bNewDataNeedSave = true;
							g_sRecord.nTestUnit = 0;
							g_sRecord.nTestMode = eTestMaintenance;
							memset(g_sRecord.aLocation, 0, sizeof(g_sRecord.aLocation));
							Data_Record_Full_Save();
							WM_DeleteWindow(pMsg->hWin);
						}
						break;
					default:
						break;
				}
			}
			return;
			
		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pMsg->hWin);
					break;
			}
			return;
			
		case WM_DELETE:
			Voc_Close();
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("ShowReportWindow will be destroyed!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}

CW_ShowReport *CW_ShowReport_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eShowReport;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eShowReport window is alreay exist!\r\n");
		return (CW_ShowReport *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_ShowReport));

	if(hWin)
	{
		TRACE_PRINTF("eShowReport windows is created!hWin = 0x%04X\r\n", hWin);

		pThisWin = (CW_ShowReport *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eShowReport window can't be created!\r\n");
	}

	return pThisWin;
}

