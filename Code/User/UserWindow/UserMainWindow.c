#include "platform.h"
#include "Windows.h"
#include "rtc.h"
#include "adc.h"
#include "ads8320.h"
#include "data.h"
#include "test.h"
#include "gps.h"
#include "buzzer.h"
#include "gprs.h"
#include "printer.h"
#include "gprs_user.h"
#include "app.h"

static CW_UserMain *pThisWin = NULL;

static void Update_Rtc_Display(WM_HWIN hParent)
{
	char aTemp[30];
	/*刷新时间*/
	sprintf(aTemp, "%02d:%02d:%02d", g_sDateTime.wHour, g_sDateTime.wMinute, g_sDateTime.wSecond);
	TEXT_SetText(pThisWin->hTime, aTemp);

	/*刷新日期*/
	sprintf(aTemp, "%02d/%02d/%04d %s", g_sDateTime.wDay, g_sDateTime.wMonth, g_sDateTime.wYear + 2000, GetDayOfWeek(g_nDayOfWeek));
	TEXT_SetText(pThisWin->hDate, aTemp);
}

void Display_Logo(void)
{
	GUI_RECT rect;
	char aTemp[30];

	/*绘制LOGO*/
	if(g_sLogoAttribute.nValidDataFlag)
	{
		Paint_BackGround();
		
		if(g_sLogoAttribute.nFileType == 0)
		{
			TRACE_PRINTF("Logo is jpeg file.Not support format!\r\n");
		}
		else if(g_sLogoAttribute.nFileType == 2)
		{
			GUI_BMP_Draw((const void *)(INSIDE_FLASH_LOGO_START_ADDR),  g_sLogoAttribute.nStartX0 , g_sLogoAttribute.nStartY0);
		}
		else
		{
			Paint_BackGround();
		}
	}
	else
	{
		Paint_BackGround();
	}

	/*显示酒精检测仪*/
	GUI_SetTextMode(GUI_TEXTMODE_TRANS);
	GUI_SetFont(&GUI_FontSongTiBold24);
	GUI_SetColor(GUI_DARKGREEN);

	sprintf(aTemp, "%s%s", _AlcoholTestUnit, FW_VERSION_LOGO);
	GUI_GetTextExtend(&rect, aTemp, 50);
	GUI_DispStringAt(aTemp, (LCD_XSIZE - (rect.x1 - rect.x0 + 1)) >> 1, 68);

	GUI_GetTextExtend(&rect, g_sDeviceIDConfig.aDeviceTypeString, 50);
	GUI_DispStringAt(g_sDeviceIDConfig.aDeviceTypeString, (LCD_XSIZE - (rect.x1 - rect.x0 + 1)) >> 1, 100);
}

static void _CallBack_Battery(WM_MESSAGE *pMsg)
{
	static unsigned char nCnt = 0;
	if(pMsg->MsgId == WM_PAINT)
	{
		unsigned char nBatteryVolume;
		const GUI_BITMAP *pBmpFile = NULL;
		nBatteryVolume = Adc_Get_Battery_Volume();
		switch(nBatteryVolume)
		{
			case 0:
				nCnt++;
				if(nCnt > 5)				 /*实现闪烁效果*/
				{
					pBmpFile = &bmBATTERY1;
				}
				else if(nCnt > 10)	
				{
					nCnt = 0;
				}
				break;
			case 1:
				pBmpFile = &bmBATTERY2;
				break;
			case 2:
				pBmpFile = &bmBATTERY3;
				break;
			case 3:
				pBmpFile = &bmBATTERY4;
				break;
			case 4:
				pBmpFile = &bmBATTERY5;
				break;
			case 5:
				pBmpFile = &bmBATTERY6;
				break;
		}
		if(pBmpFile)
		{
			GUI_DrawBitmap(pBmpFile,
						(WM_GetWindowSizeX(pMsg->hWin) - pBmpFile->XSize) >> 1,
						(WM_GetWindowSizeY(pMsg->hWin) - pBmpFile->YSize) >> 1);
		}
	}
}

static void _CallBack_GPRS(WM_MESSAGE *pMsg)
{
	if(pMsg->MsgId == WM_PAINT)
	{
		const GUI_BITMAP *pBmpFile = NULL;
		switch(Get_GPRS_SingalQuality())
		{
			case 0:
				pBmpFile = &bmSignal1;
				break;
			case 1:
				pBmpFile = &bmSignal2;
				break;
			case 2:
				pBmpFile = &bmSignal3;
				break;
			case 3:
				pBmpFile = &bmSignal4;
				break;
			case 4:
				pBmpFile = &bmSignal5;
				break;
			case 5:
				pBmpFile = &bmSignal6;
				break;
		}
		if(pBmpFile)
		{
			GUI_DrawBitmap(pBmpFile,
						(WM_GetWindowSizeX(pMsg->hWin) - pBmpFile->XSize) >> 1,
						(WM_GetWindowSizeY(pMsg->hWin) - pBmpFile->YSize) >> 1);
		}
	}
}


static void _CallBack_GPS(WM_MESSAGE *pMsg)
{
	if(pMsg->MsgId == WM_PAINT)
	{
		unsigned char bGps;
		const GUI_BITMAP *pBmpFile = NULL;
		bGps = GPS_Is_Location_Valid();
		switch(bGps)
		{
			case 0:
				pBmpFile = &bmGPS_Search;
				break;
			case 1:
				pBmpFile = &bmGPS_Location;
				break;
		}

		if(pBmpFile)
		{
			GUI_DrawBitmap(pBmpFile, 
							(WM_GetWindowSizeX(pMsg->hWin) - pBmpFile->XSize) >> 1,
							(WM_GetWindowSizeY(pMsg->hWin) - pBmpFile->YSize) >> 1);
		}
	}
}



static void _CallBack_Header(WM_MESSAGE *pMsg)
{
	if(pMsg->MsgId == WM_PAINT)
	{
		GUI_DrawBitmap(&bmCaption_Bottom, 0, 0);
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	GUI_RECT rect;
	const char *pStr;

	pApp->bSystemLoginIn = true;
	
	pThisWin->hHeader = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, Caption_Height,
	                                        pMsg->hWin,
	                                        WM_CF_SHOW, _CallBack_Header, 0);
	/*时间*/
	GUI_SetFont(&GUI_FontTahomaBold19);
	pStr = "00:00:00";
	GUI_GetTextExtend(&rect, pStr, 50);
	pThisWin->hTime= TEXT_CreateEx((LCD_XSIZE - (rect.x1 - rect.x0)) >> 1,  (Caption_Height - (rect.y1 - rect.y0 + 1)) >> 1, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pThisWin->hHeader,
	                     WM_CF_SHOW, 0, 0, pStr);
	TEXT_SetFont(pThisWin->hTime, &GUI_FontTahomaBold19);
	TEXT_SetTextAlign(pThisWin->hTime, GUI_TA_CENTER);
	TEXT_SetTextColor(pThisWin->hTime, _RGB(80, 0, 71));
	
	/*电池图标显示*/
	pThisWin->hDrawBattery = WM_CreateWindowAsChild(LCD_XSIZE - 30, 0, 30, 30,
	                                        pThisWin->hHeader,
	                                        WM_CF_SHOW | WM_CF_HASTRANS, _CallBack_Battery, 0);
	/*GSM信号显示*/
	pThisWin->hDrawGPRSSignal = WM_CreateWindowAsChild(LCD_XSIZE - 30 - 35, 0, 30, 30,
	                                      pThisWin->hHeader,
	                                       WM_CF_SHOW | WM_CF_HASTRANS, _CallBack_GPRS, 0);
	/*GPS信号显示*/
	pThisWin->hDrawGpsSignal = WM_CreateWindowAsChild(5, 0, 30, 30,
	                                    pThisWin->hHeader,
	                                    WM_CF_SHOW | WM_CF_HASTRANS, _CallBack_GPS, 0);
	                                    
	/*日期---星期*/
	pStr = "00/00/0000";
	GUI_SetFont(_GetFont(Font_Content));
	GUI_GetTextExtend(&rect, pStr, 50);
	pThisWin->hDate = TEXT_CreateEx(20,  Caption_Height + 8, LCD_XSIZE - 40, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, pStr);
	TEXT_SetFont(pThisWin->hDate, _GetFont(Font_Title));
	TEXT_SetTextColor(pThisWin->hDate, GUI_DARKGREEN);
	TEXT_SetTextAlign(pThisWin->hDate, GUI_TA_CENTER);

#if USE_TEXT_REPLACE_ICON
	/*创建左键*/
	pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hLeftButton, _Option);

	/*创建右键*/
	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hRightButton, _Query);

	/*创建中键*/
	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hMiddleButton, _Test);
#else
	/*创建左键*/
	pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(pThisWin->hLeftButton, 0, BtnSetBmp(bmTool_Released, pThisWin->hLeftButton));
	BUTTON_SetBitmapEx(pThisWin->hLeftButton, 1, BtnSetBmp(bmTool_Pressed, pThisWin->hLeftButton));

	/*创建右键*/
	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 0, BtnSetBmp(bmSearch_Released, pThisWin->hRightButton));
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 1, BtnSetBmp(bmSearch_Pressed, pThisWin->hRightButton));

	/*创建中键*/
	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(pThisWin->hMiddleButton, 0, BtnSetBmp(bmTest_Released, pThisWin->hMiddleButton));
	BUTTON_SetBitmapEx(pThisWin->hMiddleButton, 1, BtnSetBmp(bmTest_Pressed, pThisWin->hMiddleButton));
#endif

	Update_System_Time();
	Update_Rtc_Display(pMsg->hWin);
	
	WM_Paint(pMsg->hWin);

	WM_SetFocus(pMsg->hWin);

}

static void _ConfirmButton(WM_MESSAGE *pMsg)
{
	g_eAlcTestPurpose = eTestForNormal;
	CW_TestMain_Create(pMsg->hWin,(void *)eTestAllMode);
}

static void _Callback(WM_MESSAGE *pMsg)
{
	if(pMsg->MsgId != WM_PAINT)
	{
		Enter_Callback_Hook(pMsg);
	}
	
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;
			
		case WM_PAINT:
			Display_Logo();
			Paint_Dialog_Header_Bottom();
			break;
			
		case WM_UPDATE_TIME:
			Update_Rtc_Display(pMsg->hWin);
			WM_InvalidateWindow(pThisWin->hDrawBattery);
			WM_InvalidateWindow(pThisWin->hDrawGpsSignal);
			break;
			
		case WM_KEY:
			if(WM_Key_Status == 0)
			{	
				#if SERVER_USE_XING_JIKONG_GU
				g_sXingJiKongGU.bOpenDevice=false; //added by lxl 20181106
				g_sXingJiKongGU.bAdministration=false;
				#endif
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:
						CW_SettingMain_Create(pMsg->hWin,(void *)0x01);
						break;
						
					case GUI_KEY_RIGHT_FUN:
						CW_DisplayRecordWindow_Create(pMsg->hWin,(void *)1);
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						_ConfirmButton(pMsg);
						break;
				}
			}
			return;

		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pMsg->hWin);
					WM_Paint(pMsg->hWin);
					WM_SendMessageNoPara(pMsg->hWin,WM_UPDATE_TIME);
					break;
			}
			return;
			
		case WM_DELETE:
			break;
			
		default:
			break;
	}
	
	Exit_Callback_Hook(pMsg);
}


CW_UserMain *CW_UserMain_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eUserMain;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eMain window is already exist!\r\n");
		return (CW_UserMain *)0;
	}

	/*主窗体特殊处理,不使用存储设备*/
	WM_SetCreateFlags(0);
	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_UserMain));
	WM_SetCreateFlags(WM_CF_MEMDEV);

	if(hWin)
	{
		TRACE_PRINTF("eMain windows is created!hWin = 0x%04X\r\n",hWin);

		pThisWin = (CW_UserMain *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);
		
		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eMain window can't be created!\r\n");
	}

	return pThisWin ;
}


