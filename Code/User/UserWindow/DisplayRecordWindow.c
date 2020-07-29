#include "platform.h"
#include "Windows.h"
#include "rtc.h"
#include "data.h"
#include "test.h"
#include "buzzer.h"
#include "CustomDisplayDriver.h"
#include "app.h"

static CW_DisplayRecordWindow *pThisWin = NULL;

#define USE_MUTIEDIT_REPLACE_TEXT			1

typedef enum
{
	DisplayRecord,
	PrintRecord
} eWinMode;

#if defined(CUST_CHINA_SZJAZN)

static void DisplayContentFormat(char *aBuffer, sRecord *pRecord)
{
	float fTemp = 0;
	char aTemp[30], *pStr;
	float fLongitude, fLatitude;

	pStr = &aBuffer[0];
	
	/*记录号*/
	sprintf(pStr, "%s%08ld\n", _R_RecordNumber, pRecord->nRecordIndex);
	pStr += strlen(pStr);

	/*记录日期*/
	sprintf(pStr, "%s%04d/%02d/%02d\n", _R_Date, pRecord->aDateTime[0] + 2000, pRecord->aDateTime[1], pRecord->aDateTime[2]);
	pStr += strlen(pStr);

	/*记录时间*/
	sprintf(pStr, "%s%02d:%02d\n", _R_Time, pRecord->aDateTime[3], pRecord->aDateTime[4]);
	pStr += strlen(pStr);

	/*测试温度*/
	sprintf(pStr, _R_Temperature, pRecord->nTemperature);
	pStr += strlen(pStr);
	*pStr++ = '\n';

	/*测试模式*/
	if(pRecord->nTestMode == eTestActive)
	{
		sprintf(pStr, "%s%s\n", _R_TestMode, _R_ActiveTest);
	}
	else if(pRecord->nTestMode == eTestPassive)
	{
		sprintf(pStr, "%s%s\n", _R_TestMode, _R_PassivelTest);
	}
	else if(pRecord->nTestMode == eTestQuickTest)
	{
		sprintf(pStr, "%s%s\n", _R_TestMode, _R_QuickTest);
	}
	else if(pRecord->nTestMode == eTestRefuseMode)
	{
		sprintf(pStr, "%s\n", _R_RefuseTest);
	}
	pStr += strlen(pStr);

	/*测试值*/
	if(pRecord->fAlcValue > MAX_DISPLAY_ALC_VALUE)
	{
		aTemp[0] = '>';
		fTemp = Get_Final_Convert_Display_Result(MAX_DISPLAY_ALC_VALUE,pRecord->nTestUnit, pRecord->nKBrACBAC);
		Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit, &aTemp[1],eFormatToInt);
	}
	else
	{
		fTemp = Get_Final_Convert_Display_Result(pRecord->fAlcValue,pRecord->nTestUnit, pRecord->nKBrACBAC);
		Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit, aTemp,eFormatToInt);
	}
	sprintf(pStr, "%s%s%s\n",
	        pRecord->nTestUnit > 4 ? "BrAC:" : "BAC:",aTemp,Get_Unit_String(pRecord->nTestUnit));
	pStr += strlen(pStr);

	/*判定结果*/
	if(pRecord->nTestMode != eTestRefuseMode)
	{
		if(pRecord->fAlcValue < 20)
			sprintf(pStr, "%s%s\n",_R_TestResult, _R_TestRestltNormal);
		else if(pRecord->fAlcValue >= 20 && pRecord->fAlcValue < 80)
			sprintf(pStr, "%s%s\n", _R_TestResult, _R_TestRestltDrink);
		else if(pRecord->fAlcValue >= 80)
			sprintf(pStr, "%s%s\n", _R_TestResult, _R_TestRestltDrunk);
	}
	else
	{
		sprintf(pStr, "%s%s\n", _R_TestResult, _P_RefuseTest);
	}
	pStr += strlen(pStr);

	/*地点*/
	sprintf(pStr, "%s%s\n", _R_Location, pRecord->aLocation);
	pStr += strlen(pStr);

	/*经度,纬度*/
	if(pRecord->fLongitude != 0 || pRecord->fLatitude != 0)
	{
		/*经度*/
		fLongitude = pRecord->fLongitude >= 0 ? pRecord->fLongitude : -pRecord->fLongitude;
		fTemp = (fLongitude - (int)fLongitude) * 60;
		sprintf(aTemp, "%d\xC2\xB0%d'%d.%d\"",
		        (int)fLongitude / 100, (int)fLongitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100));
		sprintf(pStr, "%s%s %s\n", _R_Longitude, pRecord->fLongitude >= 0 ? _R_Longitude_East : _R_Longitude_West, aTemp);
		pStr += strlen(pStr);
		/*纬度*/
		fLatitude = pRecord->fLatitude >= 0 ? pRecord->fLatitude : -pRecord->fLatitude;
		fTemp = (fLatitude - (int)fLatitude) * 60;
		sprintf(aTemp, "%d\xC2\xB0%d'%d.%d\"",
		        (int)fLatitude / 100, (int)fLatitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100));
		sprintf(pStr, "%s%s %s\n", _R_Latitude, pRecord->fLatitude >= 0 ? _R_Latitude_North : _R_Latitude_South, aTemp);
		pStr += strlen(pStr);
	}
	else
	{
		sprintf(pStr, "%s\n", _R_Longitude);
		pStr += strlen(pStr);
		sprintf(pStr, "%s\n", _R_Latitude);
		pStr += strlen(pStr);
	}
	
	/*测试人*/
	sprintf(pStr, "%s%s\n", _R_Testee, pRecord->aTestee);
	pStr += strlen(pStr);

	/*车牌号*/
	sprintf(pStr, "%s%s\n", _R_PlateNumber, pRecord->aPlateNumber);
	pStr += strlen(pStr);

	/*驾驶证号*/
	sprintf(pStr, "%s%s\n", "\xE8\xAF\x81\xE4\xBB\xB6\xE5\x8F\xB7\x3A" /*证件号:*/, pRecord->aDriverLicense);
	pStr += strlen(pStr);

	/*执勤民警警号*/
	sprintf(pStr, "%s%s\n", _R_PoliceID, pRecord->aPoliceID);
	pStr += strlen(pStr);
	
	/*执勤民警姓名*/
	sprintf(pStr, "%s%s\n", _R_PoliceName, pRecord->aPoliceName);
	pStr += strlen(pStr);

	/*所属单位*/
	sprintf(pStr, "%s%s", _R_Department, pRecord->aDepartment);
	pStr += strlen(pStr);
}

#else

static void DisplayContentFormat(char *aBuffer, sRecord *pRecord)
{
	float fTemp = 0;
	char aTemp[30], *pStr;
	float fLongitude, fLatitude;

	pStr = &aBuffer[0];
	
	/*记录号*/
	sprintf(pStr, "%s%08ld\n", _R_RecordNumber, pRecord->nRecordIndex);
	pStr += strlen(pStr);

	/*记录日期*/
	sprintf(pStr, "%s%04d/%02d/%02d\n", _R_Date, pRecord->aDateTime[0] + 2000, pRecord->aDateTime[1], pRecord->aDateTime[2]);
	pStr += strlen(pStr);

	/*记录时间*/
	sprintf(pStr, "%s%02d:%02d:%02d\n", _R_Time, pRecord->aDateTime[3], pRecord->aDateTime[4],pRecord->aDateTime[5]);
	pStr += strlen(pStr);

	/*测试温度*/
	sprintf(pStr, _R_Temperature, pRecord->nTemperature);
	pStr += strlen(pStr);
	*pStr++ = '\n';

	/*测试模式*/
	if(pRecord->nTestMode == eTestActive)
	{
		sprintf(pStr, "%s%s\n", _R_TestMode, _R_ActiveTest);
	}
	else if(pRecord->nTestMode == eTestPassive)
	{
		sprintf(pStr, "%s%s\n", _R_TestMode, _R_PassivelTest);
	}
	else if(pRecord->nTestMode == eTestQuickTest)
	{
		sprintf(pStr, "%s%s\n", _R_TestMode, _R_QuickTest);
	}
	else if(pRecord->nTestMode == eTestRefuseMode)
	{
		sprintf(pStr, "%s\n", _R_RefuseTest);
	}
	else if(pRecord->nTestMode == eTestMaintenance)
	{
		sprintf(pStr, "%s\n", "\xE7\xBB\xB4\xE6\x8A\xA4\xE6\xB5\x8B\xE8\xAF\x95" /*维护测试*/);
	}
	pStr += strlen(pStr);


	/*测试值*/
	if(pRecord->fAlcValue > (pRecord->nTestMode != eTestQuickTest ? MAX_DISPLAY_ALC_VALUE : MAX_QUICK_TEST_DISPLAY_ALC_VALUE))
	{
		aTemp[0] = '>';
		fTemp = Get_Final_Convert_Display_Result((pRecord->nTestMode != eTestQuickTest ? MAX_DISPLAY_ALC_VALUE : MAX_QUICK_TEST_DISPLAY_ALC_VALUE),pRecord->nTestUnit, pRecord->nKBrACBAC);
		Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit, &aTemp[1],eFormatToInt);
	}
	else
	{
		fTemp = Get_Final_Convert_Display_Result(pRecord->fAlcValue,pRecord->nTestUnit, pRecord->nKBrACBAC);
		Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit, aTemp,eFormatToInt);
	}
	sprintf(pStr, "%s%s%s\n",
	pRecord->nTestUnit > 4 ? "BrAC:" : "BAC:",aTemp,Get_Unit_String(pRecord->nTestUnit));
	pStr += strlen(pStr);

	/*判定结果*/
	if(pRecord->nTestMode != eTestRefuseMode)
	{
		if(pRecord->fAlcValue < 20)
			sprintf(pStr, "%s%s\n",_R_TestResult, _R_TestRestltNormal);
		else if(pRecord->fAlcValue >= 20 && pRecord->fAlcValue < 80)
			sprintf(pStr, "%s%s\n", _R_TestResult, _R_TestRestltDrink);
		else if(pRecord->fAlcValue >= 80)
			sprintf(pStr, "%s%s\n", _R_TestResult, _R_TestRestltDrunk);
	}
	else
	{
		sprintf(pStr, "%s%s\n", _R_TestResult, _P_RefuseTest);
	}
	pStr += strlen(pStr);

	/*地点*/
	sprintf(pStr, "%s%s\n", _R_Location, pRecord->aLocation);
	pStr += strlen(pStr);

	/*经度,纬度*/
	if(pRecord->fLongitude != 0 || pRecord->fLatitude != 0)
	{
		/*经度*/
		fLongitude = pRecord->fLongitude >= 0 ? pRecord->fLongitude : -pRecord->fLongitude;
		fTemp = (fLongitude - (int)fLongitude) * 60;
		sprintf(aTemp, "%d\xC2\xB0%d'%d.%d\"",
		        (int)fLongitude / 100, (int)fLongitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100));
		sprintf(pStr, "%s%s %s\n", _R_Longitude, pRecord->fLongitude >= 0 ? _R_Longitude_East : _R_Longitude_West, aTemp);
		pStr += strlen(pStr);
		/*纬度*/
		fLatitude = pRecord->fLatitude >= 0 ? pRecord->fLatitude : -pRecord->fLatitude;
		fTemp = (fLatitude - (int)fLatitude) * 60;
		sprintf(aTemp, "%d\xC2\xB0%d'%d.%d\"",
		        (int)fLatitude / 100, (int)fLatitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100));
		sprintf(pStr, "%s%s %s\n", _R_Latitude, pRecord->fLatitude >= 0 ? _R_Latitude_North : _R_Latitude_South, aTemp);
		pStr += strlen(pStr);
	}
	else
	{
		sprintf(pStr, "%s\n", _R_Longitude);
		pStr += strlen(pStr);
		sprintf(pStr, "%s\n", _R_Latitude);
		pStr += strlen(pStr);
	}
	
	/*测试人*/
	sprintf(pStr, "%s%s\n", _R_Testee, pRecord->aTestee);
	pStr += strlen(pStr);

	/*车牌号*/
	sprintf(pStr, "%s%s\n", _R_PlateNumber, pRecord->aPlateNumber);
	pStr += strlen(pStr);

	/*驾驶证号*/
	sprintf(pStr, "%s%s\n", _R_DriverLicense, pRecord->aDriverLicense);
	pStr += strlen(pStr);

	/*执勤民警警号*/
	sprintf(pStr, "%s%s\n", _R_PoliceID, pRecord->aPoliceID);
	pStr += strlen(pStr);
	
	/*执勤民警姓名*/
	sprintf(pStr, "%s%s\n", _R_PoliceName, pRecord->aPoliceName);
	pStr += strlen(pStr);

	/*所属单位*/
	sprintf(pStr, "%s%s", _R_Department, pRecord->aDepartment);
	pStr += strlen(pStr);
}

#endif

static void _Create_Button(eWinMode WinMode, WM_HWIN hParent)
{
	WM_HWIN hWin;

	WM_DeleteWindow(WM_GetDialogItem(hParent, GUI_ID_BUTTON0));
	WM_DeleteWindow(WM_GetDialogItem(hParent, GUI_ID_BUTTON1));
	WM_DeleteWindow(WM_GetDialogItem(hParent, GUI_ID_BUTTON2));

#if USE_TEXT_REPLACE_ICON
	if(WinMode == DisplayRecord)
	{
		/*创建左键*/
		hWin = Create_Left_Button(hParent);
		BUTTON_SetText(hWin, _Previous);

		/*创建右键*/
		hWin = Create_Right_Button(hParent);
		BUTTON_SetText(hWin, _Next);

		/*创建中键*/
		hWin = Create_Middle_Button(hParent);
		BUTTON_SetText(hWin, _OK);
	}
	else if(WinMode == PrintRecord)
	{
		/*创建左键*/
		hWin = Create_Left_Button(hParent);
		BUTTON_SetText(hWin, __Exit);

		/*创建右键*/
		hWin = Create_Right_Button(hParent);
		BUTTON_SetText(hWin, _Back);

		/*创建中键*/
		hWin = Create_Middle_Button(hParent);
		BUTTON_SetText(hWin, _Print);
	}
#else
	if(WinMode == DisplayRecord)
	{
		/*创建左键*/
		hWin = Create_Left_Button(hParent);
		BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmLeft_Arrows_Released, hWin));
		BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmLeft_Arrows_Pressed, hWin));

		/*创建右键*/
		hWin = Create_Right_Button(hParent);
		BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmRight_Arrows_Released, hWin));
		BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmRight_Arrows_Pressed, hWin));

		/*创建中键*/
		hWin = Create_Middle_Button(hParent);
		BUTTON_SetText(hWin, _OK);
	}
	else if(WinMode == PrintRecord)
	{
		/*创建左键*/
		hWin = Create_Left_Button(hParent);
		BUTTON_SetText(hWin, __Exit);

		/*创建右键*/
		hWin = Create_Right_Button(hParent);
		BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmCancel_Released, hWin));
		BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmCancel_Pressed, hWin));

		/*创建中键*/
		hWin = Create_Middle_Button(hParent);
		BUTTON_SetText(hWin, _Print);
	}
#endif
}



#define TEXT_BK_COLOR	_RGB(255, 128, 0)

static void _Format_Record(WM_MESSAGE *pMsg, sRecord *pRecord)
{
	char *pBuffer;
	WM_HWIN hWin;
	hWin = GUI_ALLOC_AllocZero(600);
	pBuffer = GUI_ALLOC_h2p(hWin);
	DisplayContentFormat(pBuffer, pRecord);
#if !USE_MUTIEDIT_REPLACE_TEXT
	TEXT_SetTextColor(pThisWin->hRecord,  pRecord->nRecordIndex > g_nAlreadyUploadCounter ? GUI_BLACK : GUI_BLUE);
	TEXT_SetText(pThisWin->hRecord, pBuffer);
#else
	MULTIEDIT_SetText(pThisWin->hRecord, pBuffer);
	MULTIEDIT_SetTextColor(pThisWin->hRecord, MULTIEDIT_CI_READONLY, pRecord->nRecordIndex > g_nAlreadyUploadCounter ? GUI_BLACK : GUI_BLUE);
#endif
	GUI_ALLOC_Free(hWin);
	
	WM_HideWindow(pThisWin->hImage);
}

static void _Get_Whole_Single_Record(void)
{
	Spi_Flash_Init();
	Data_Record_Load_ByID(pThisWin->nCurrDisplayRecordIndex, &g_sRecord);
	Spi_Flash_DeInit();
}

static void _Create(WM_MESSAGE *pMsg)
{
	if(g_sRecordInforMap.nTotalRecordsCounter == 0)
	{
		sDialogWindowProperty s;
		s.nWinType = eDialogOK;
		s.pFont = _GetFont(Font_Content);
		s.pContent = _NoRecord;
		s.nContentColor = GUI_RED;
		s.nBackGroundColor = GUI_LIGHTGRAY;
		s.nAutoCloseTime = 3;
		s.pFunMiddleOK = NULL;
		WM_DeleteWindow(pMsg->hWin);
		CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
	}
	else
	{
	#if !USE_MUTIEDIT_REPLACE_TEXT
		pThisWin->hRecord = TEXT_CreateEx(3, 3, LCD_XSIZE - 6, LCD_YSIZE - 30 - 3,
		                    						 pMsg->hWin,
		                     					WM_CF_SHOW, 0, 0, "");
		TEXT_SetWrapMode(pThisWin->hRecord, GUI_WRAPMODE_WORD);
		TEXT_SetTextColor(pThisWin->hRecord, GUI_BLACK);
		TEXT_SetBkColor(pThisWin->hRecord, TEXT_BK_COLOR);
		TEXT_SetFont(pThisWin->hRecord, _GetFont(Font_DisplayRecord));
	#else
		pThisWin->hRecord = MULTIEDIT_CreateEx(3, 3, LCD_XSIZE - 6, LCD_YSIZE - 30 - 3,
							 pMsg->hWin,
		                     		WM_CF_SHOW, MULTIEDIT_CF_READONLY, NULL, 600, "");
		MULTIEDIT_SetFont(pThisWin->hRecord, _GetFont(Font_DisplayRecord));
		MULTIEDIT_SetTextColor(pThisWin->hRecord, MULTIEDIT_CI_READONLY, GUI_BLACK);
		MULTIEDIT_SetBkColor(pThisWin->hRecord, MULTIEDIT_CI_READONLY,TEXT_BK_COLOR);
		MULTIEDIT_SetWrapChar(pThisWin->hRecord);
		WIDGET_SetEffect(pThisWin->hRecord,&WIDGET_Effect_None);
		WM_DisableMemdev(pThisWin->hRecord);
		WIDGET_AndState(pThisWin->hRecord, WIDGET_STATE_FOCUSSABLE);
	#endif

		pThisWin->hImage = BUTTON_CreateEx(200, 4, 28, 28,
					                       pMsg->hWin,
					                       WM_CF_SHOW, 1, 0);
		BUTTON_SetBitmapEx(pThisWin->hImage, 0,&bmImage,0,0);
		BUTTON_SetBitmapEx(pThisWin->hImage, 1,&bmImage,0,0);
		BUTTON_SetFocussable(pThisWin->hImage, 0);
		
		pThisWin->nCurrDisplayRecordIndex = g_sRecordInforMap.nTotalRecordsCounter - 1;

		_Get_Whole_Single_Record();
		_Format_Record(pMsg,&g_sRecord);

		pThisWin->nCurrDisplayMode = DisplayRecord;
		_Create_Button((eWinMode)pThisWin->nCurrDisplayMode, pMsg->hWin);
		
		WM_SetFocus(pMsg->hWin);
	}
}

void _Paint_BackGround(void)
{
	GUI_SetColor(TEXT_BK_COLOR);
	GUI_FillRect(0, 0, LCD_XSIZE, LCD_YSIZE);
}

static void _Callback(WM_MESSAGE *pMsg)
{
	if(pMsg->MsgId == WM_PAINT)
	{
		_Paint_BackGround();
		return;
	}
	
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
						if(pThisWin->nCurrDisplayMode == DisplayRecord)
						{
							if(g_sRecordInforMap.nTotalRecordsCounter)
							{
								if(pThisWin->nCurrDisplayRecordIndex)
								{
									pThisWin->nCurrDisplayRecordIndex--;
								}
								else
								{
									pThisWin->nCurrDisplayRecordIndex = g_sRecordInforMap.nTotalRecordsCounter - 1;
								}

								_Get_Whole_Single_Record();
								_Format_Record(pMsg,&g_sRecord);
							}
						}
						else if(pThisWin->nCurrDisplayMode == PrintRecord)
						{
							WM_DeleteWindow(pMsg->hWin);
						}
						break;
						
					case GUI_KEY_RIGHT_FUN:
						if(pThisWin->nCurrDisplayMode == DisplayRecord)
						{
							if(g_sRecordInforMap.nTotalRecordsCounter > 0)
							{
								if(pThisWin->nCurrDisplayRecordIndex < g_sRecordInforMap.nTotalRecordsCounter - 1)
								{
									pThisWin->nCurrDisplayRecordIndex ++;
								}
								else
								{
									pThisWin->nCurrDisplayRecordIndex = 0;
								}

								_Get_Whole_Single_Record();
								_Format_Record(pMsg,&g_sRecord);
							}
						}
						else if(pThisWin->nCurrDisplayMode == PrintRecord)
						{
							pThisWin->nCurrDisplayMode = DisplayRecord;
							_Create_Button((eWinMode)pThisWin->nCurrDisplayMode, pMsg->hWin);
						}
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						if(pThisWin->nCurrDisplayMode == DisplayRecord)
						{
							pThisWin->nCurrDisplayMode = PrintRecord;
							_Create_Button((eWinMode)pThisWin->nCurrDisplayMode, pMsg->hWin);
						}
						else if(pThisWin->nCurrDisplayMode == PrintRecord)
						{
							CW_PrintRecordWindow_Create(pMsg->hWin, (void *)0);
						}
						break;
				}
			}
			return;
			
		case WM_NOTIFY_GPRS_UPLOAD_EVENT:
			_Format_Record(pMsg,&g_sRecord);
			break;

		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pMsg->hWin);
					break;

				case WM_NOTIFICATION_RELEASED:
					break;
			}
			return;

		case WM_DELETE:
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eDisplayRecord window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_DisplayRecordWindow *CW_DisplayRecordWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eDisplayRecord;

	if(GetWinHandleByWinType(WinType)) 
	{
		TRACE_PRINTF("eDisplayRecord window is already exist!\r\n");
		return (CW_DisplayRecordWindow *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_DisplayRecordWindow));

	if(hWin)
	{
		TRACE_PRINTF("eDisplayRecord windows is created!hWin = 0x%04X\r\n",hWin);

		pThisWin = (CW_DisplayRecordWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);
		
		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eDisplayRecord window can't be created!\r\n");
	}

	return pThisWin ;
}


