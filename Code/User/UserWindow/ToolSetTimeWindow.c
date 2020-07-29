#include "platform.h"
#include "Windows.h"
#include "rtc.h"
#include "time_calc.h"
#include "EDIT_Private.h"
#include "app.h"

static CW_SetTime *pThisWin;

static const  char aBtn_Adjust_Up[] = {0xE2, 0x96, 0xB2, 0x00};   /*▲*/;
static const  char aBtn_Adjust_Down[] = {0xE2, 0x96, 0xBC, 0x00}; /*▼*/

#define ID_HOUR_UP			0x800
#define ID_HOUR_DOWN		0x801
#define ID_MINUTE_UP		0x802
#define ID_MINUTE_DOWN	0x803
#define ID_YEAR_UP			0x804
#define ID_YEAR_DOWN		0x805
#define ID_MONTH_UP		0x806
#define ID_MONTH_DOWN		0x807
#define ID_DAY_UP			0x808
#define ID_DAY_DOWN		0x809


static void SetButton_Prop(WM_HWIN hWin)
{
	BUTTON_SetFocussable(hWin, 0);
	BUTTON_SetFont(hWin, &GUI_FontTriangle);
	BUTTON_SetBkColor(hWin, 0, 0xB59179);
	BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
	BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
	BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
	BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	GUI_RECT rect;

	pMsg->Data.p = _TimeDateSet;
	Create_Dialog_Caption(pMsg);

	/*创建左键*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	/*创建右键*/
	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	/*创建中键*/
	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);

	/*Hour*/
	hWin = EDIT_CreateEx(49, 82, 50, 25,
	                     pMsg->hWin, WM_CF_SHOW, 0, GUI_ID_EDIT3, 2);
	EDIT_SetFont(hWin, &GUI_FontTahomaBold19);
	EDIT_SetTextAlign(hWin, GUI_TA_HCENTER);
	EDIT_SetDecMode(hWin, g_sDateTime.wHour, 0, 23, 0, 0);

	/*Minute*/
	hWin = EDIT_CreateEx(139, 82, 50, 25,
	                     pMsg->hWin, WM_CF_SHOW, 0, GUI_ID_EDIT4, 2);
	EDIT_SetFont(hWin, &GUI_FontTahomaBold19);
	EDIT_SetTextAlign(hWin, GUI_TA_HCENTER);
	EDIT_SetDecMode(hWin, g_sDateTime.wMinute, 0, 59, 0, 0);

	/*分隔符:*/
	GUI_SetFont(&GUI_FontTahomaBold19);
	GUI_GetTextExtend(&rect, ":", 50);
	hWin = TEXT_CreateEx((LCD_XSIZE - (rect.x1 - rect.x0 + 1)) >> 1, 82 + 3, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, GUI_ID_TEXT7, ":");
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetFont(hWin, &GUI_FontTahomaBold19);

	hWin = BUTTON_CreateEx(60, 50, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_HOUR_UP);
	BUTTON_SetText(hWin, aBtn_Adjust_Up);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(150, 50, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_MINUTE_UP);
	BUTTON_SetText(hWin, aBtn_Adjust_Up);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(60, 114, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_HOUR_DOWN);
	BUTTON_SetText(hWin, aBtn_Adjust_Down);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(150, 114, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_MINUTE_DOWN);
	BUTTON_SetText(hWin, aBtn_Adjust_Down);
	SetButton_Prop(hWin);


	/*Day*/
	hWin = EDIT_CreateEx(22, 203, 50, 25,
	                     pMsg->hWin, WM_CF_SHOW, 0, GUI_ID_EDIT0, 2);
	EDIT_SetFont(hWin, &GUI_FontTahomaBold19);
	EDIT_SetTextAlign(hWin, GUI_TA_HCENTER);
	EDIT_SetDecMode(hWin, g_sDateTime.wDay, 1, 31, 0, 0);
	WIDGET_AndState(hWin, WIDGET_STATE_FOCUS);

	/*Month*/
	hWin = EDIT_CreateEx(91, 203, 50, 25,
	                     pMsg->hWin, WM_CF_SHOW, 0, GUI_ID_EDIT1, 2);
	EDIT_SetFont(hWin, &GUI_FontTahomaBold19);
	EDIT_SetTextAlign(hWin, GUI_TA_HCENTER);
	EDIT_SetDecMode(hWin, g_sDateTime.wMonth, 1, 12, 0, 0);

	/*Year*/
	hWin = EDIT_CreateEx(160, 203, 50, 25,
	                     pMsg->hWin, WM_CF_SHOW, 0, GUI_ID_EDIT2, 4);
	EDIT_SetFont(hWin, &GUI_FontTahomaBold19);
	EDIT_SetTextAlign(hWin, GUI_TA_HCENTER);
	EDIT_SetDecMode(hWin, g_sDateTime.wYear + 2000, 2010, 2099, 0, 0);

	/*分隔符'/'*/
	GUI_SetFont(&GUI_FontTahomaBold19);
	GUI_GetTextExtend(&rect, "/", 50);
	hWin = TEXT_CreateEx(78, 203 + 3, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, GUI_ID_TEXT5, "/");
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetFont(hWin, &GUI_FontTahomaBold19);
	hWin = TEXT_CreateEx(148, 203 + 3, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, GUI_ID_TEXT6, "/");
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetFont(hWin, &GUI_FontTahomaBold19);


	hWin = BUTTON_CreateEx(35, 170, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_DAY_UP);
	BUTTON_SetText(hWin, aBtn_Adjust_Up);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(104, 170, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_MONTH_UP);
	BUTTON_SetText(hWin, aBtn_Adjust_Up);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(173, 170, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_YEAR_UP);
	BUTTON_SetText(hWin, aBtn_Adjust_Up);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(35, 236, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_DAY_DOWN);
	BUTTON_SetText(hWin, aBtn_Adjust_Down);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(104, 236, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_MONTH_DOWN);
	BUTTON_SetText(hWin, aBtn_Adjust_Down);
	SetButton_Prop(hWin);
	hWin = BUTTON_CreateEx(173, 236, 29, 25, pMsg->hWin, WM_CF_SHOW, 1, ID_YEAR_DOWN);
	BUTTON_SetText(hWin, aBtn_Adjust_Down);
	SetButton_Prop(hWin);

	hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT3);
	WM_SetFocus(hWin);
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
						WM_DeleteWindow(pMsg->hWin);
						break;
					case GUI_KEY_MIDDLE_FUN:
						if(1)
						{
							sDateTime s;
							s.wYear = EDIT_GetValue(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT2)) - 2000;
							s.wMonth = EDIT_GetValue(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1));
							s.wDay = EDIT_GetValue(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0));
							s.wHour = EDIT_GetValue(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT3));
							s.wMinute = EDIT_GetValue(WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT4));
							if(Is_Valid_DateTime(&s))
							{
								Rtc_Gpio_Init();
								Rtc_Set_Time(&s);
								g_sDateTime = s;
								WM_DeleteWindow(pMsg->hWin);
							}
							else
							{
								sDialogWindowProperty s;
								s.nWinType = eDialogOK;
								s.pFont = _GetFont(Font_Content);
								s.pContent = _InvalidTimeDateSet;
								s.nContentColor = GUI_RED;
								s.nBackGroundColor = GUI_LIGHTGRAY;
								s.nAutoCloseTime = 3;
								s.pFunMiddleOK = NULL;
								CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
							}
						}
						break;
				}
			}
			return;
			
		case WM_NOTIFY_PARENT:
			if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
			{
				int nValue;
				WM_HWIN hWin;
				int BTN_ID = WM_GetId(pMsg->hWinSrc);
				if(BTN_ID >= ID_HOUR_UP)
				{
					if(BTN_ID == ID_HOUR_UP)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT3);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue + 1 > 23 ? 0 : nValue + 1);
					}
					else if(BTN_ID == ID_HOUR_DOWN)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT3);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue - 1 < 0 ? 23 : nValue - 1);
					}
					else if(BTN_ID == ID_MINUTE_UP)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT4);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue + 1 > 59 ? 0 : nValue + 1);
					}
					else if(BTN_ID == ID_MINUTE_DOWN)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT4);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue - 1 < 0 ? 59 : nValue - 1);
					}
					else if(BTN_ID == ID_DAY_UP)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue + 1 > 31 ? 0 : nValue + 1);
					}
					else if(BTN_ID == ID_DAY_DOWN)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT0);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue - 1 < 0 ? 31 : nValue - 1);
					}
					else if(BTN_ID == ID_MONTH_UP)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue + 1 > 12 ? 1 : nValue + 1);
					}
					else if(BTN_ID == ID_MONTH_DOWN)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT1);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue - 1 < 1 ? 12 : nValue - 1);
					}
					else if(BTN_ID == ID_YEAR_UP)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT2);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue + 1 > 2099 ? 2010 : nValue + 1);
					}
					else if(BTN_ID == ID_YEAR_DOWN)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_EDIT2);
						nValue = EDIT_GetValue(hWin);
						EDIT_SetValue(hWin, nValue - 1 < 2010 ? 2099 : nValue - 1);
					}
				}
			}
			else if(pMsg->Data.v == WM_NOTIFICATION_WIN_DELETE)
			{
				WM_SetFocus(pMsg->hWin);
			}
			return;
			
		case WM_DELETE:
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eSetTime window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_SetTime *CW_SetTime_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSetTime;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSetTime window is alreay exist!\r\n");
		return (CW_SetTime *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_SetTime));

	if(hWin)
	{
		TRACE_PRINTF("eSetTime windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_SetTime *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSetTime window can't be created!\r\n");
	}

	return pThisWin;
}

