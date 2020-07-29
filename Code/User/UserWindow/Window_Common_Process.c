#include "platform.h"
#include "Windows.h"
#include "app.h"
#include "keypad.h"

GUI_COLOR g_nGlobalBKColor = GUI_BLACK;
WM_HWIN g_nLastWindowHandle = NULL;

void Paint_Dialog_Header_Bottom(void)
{
	GUI_DrawBitmap(&bmCaption_Bottom, 0, 0);
	GUI_DrawBitmap(&bmCaption_Bottom, 0, LCD_YSIZE - Bottom_Height);
}

void Paint_BackGround(void)
{
	GUI_SetColor(BK_COLOR_CLIENT);
	GUI_FillRect(0, Caption_Height, LCD_XSIZE - 1, LCD_YSIZE - 1 - Bottom_Height);
}

void _Paint_Dialog_BackGround(void)
{
	GUI_DrawBitmap(&bmCaption_Bottom, 0, 0);
	GUI_DrawBitmap(&bmCaption_Bottom, 0, LCD_YSIZE - Bottom_Height);

	GUI_SetColor(BK_COLOR_CLIENT);
	GUI_FillRect(0, Caption_Height, LCD_XSIZE - 1, LCD_YSIZE - 1 - Bottom_Height);
}

void Enter_Callback_Hook(WM_MESSAGE *pMsg)
{
	switch(pMsg->MsgId)
	{
		case WM_PAINT:
			_Paint_Dialog_BackGround();
			break;

		case WM_KEY:
			switch (WM_Key_Code)
			{
				case GUI_KEY_LEFT_FUN:
					BUTTON_SetPressed(WM_GetDialogItem(pMsg->hWin, BUTTON_ID_LEFT), WM_Key_Status == 1 ? 1 : 0);
					break;
				case GUI_KEY_RIGHT_FUN:
					BUTTON_SetPressed(WM_GetDialogItem(pMsg->hWin, BUTTON_ID_RIGHT), WM_Key_Status == 1 ? 1 : 0);
					break;
				case GUI_KEY_MIDDLE_FUN:
					BUTTON_SetPressed(WM_GetDialogItem(pMsg->hWin, BUTTON_ID_MIDDLE), WM_Key_Status == 1 ? 1 : 0);
					break;
			}
			if(WM_Key_Status == eKeyIgnore)
			{
				BUTTON_SetPressed(WM_GetDialogItem(pMsg->hWin, BUTTON_ID_LEFT), 0);
				BUTTON_SetPressed(WM_GetDialogItem(pMsg->hWin, BUTTON_ID_RIGHT), 0);
				BUTTON_SetPressed(WM_GetDialogItem(pMsg->hWin, BUTTON_ID_MIDDLE), 0);
			}
			return;
			
		case WM_NOTIFY_PARENT:
			if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
			{
				if(pMsg->hWinSrc == WM_GetDialogItem(pMsg->hWin, BUTTON_ID_LEFT))
				{
					GUI_StoreKeyMsg(GUI_KEY_LEFT_FUN, 0);
					WM_SendMessageNoPara(pMsg->hWin, WM_KEY);
					pMsg->Data.v = 0;
				}
				else if(pMsg->hWinSrc == WM_GetDialogItem(pMsg->hWin, BUTTON_ID_RIGHT))
				{
					GUI_StoreKeyMsg(GUI_KEY_RIGHT_FUN, 0);
					WM_SendMessageNoPara(pMsg->hWin, WM_KEY);
					pMsg->Data.v = 0;
				}
				else if(pMsg->hWinSrc == WM_GetDialogItem(pMsg->hWin, BUTTON_ID_MIDDLE))
				{
					GUI_StoreKeyMsg(GUI_KEY_MIDDLE_FUN, 0);
					WM_SendMessageNoPara(pMsg->hWin, WM_KEY);
					pMsg->Data.v = 0;
				}
			}
			break;
	}
}

void Exit_Callback_Hook(WM_MESSAGE *pMsg)
{
	switch(pMsg->MsgId)
	{
		
	}
	WM_DefaultProc(pMsg);
}

void Draw_Dot_Line_Frame(void)
{
	GUI_SetColor(g_nGlobalBKColor == GUI_BLACK ? GUI_WHITE : GUI_BLACK);
	GUI_SetLineStyle(GUI_LS_DOT);
	GUI_DrawLine(0, 0, 0, 195);
	GUI_DrawLine(0, 0, 239, 0);
	GUI_DrawLine(239, 0, 239, 195);
	GUI_DrawLine(0, 195, 239, 195);
}

void CallBack_Draw_Battery(WM_MESSAGE *pMsg)
{
#if 0
	if(pMsg->MsgId == WM_PAINT)
	{
		switch(g_nBatteryLevel)
		{
			case 0:
			case 1:
				GUI_SetColor(g_nGlobalBKColor == GUI_BLACK ? GUI_RED  : GUI_RED);
				/*填充容量*/
				break;
			case 2:
				GUI_SetColor(g_nGlobalBKColor == GUI_BLACK ? GUI_RED  : GUI_RED);
				/*填充容量*/
				GUI_FillRect(24, 3 , 27, 9);
				break;
			case 3:
				/*橙色*/
				GUI_SetColor(g_nGlobalBKColor == GUI_BLACK ? RGB2Hex(246, 186, 64) : RGB2Hex(226, 159, 54));
				/*填充容量*/
				GUI_FillRect(24, 3 , 27, 9);
				GUI_FillRect(18, 3 , 21, 9);
				break;
			case 4:
				GUI_SetColor(g_nGlobalBKColor == GUI_BLACK ? GUI_GREEN : GUI_DARKGREEN);
				/*填充容量*/
				GUI_FillRect(24, 3 , 27, 9);
				GUI_FillRect(18, 3 , 21, 9);
				GUI_FillRect(12, 3 , 15, 9);
				break;
			case 5:
				GUI_SetColor(g_nGlobalBKColor == GUI_BLACK ? GUI_GREEN : GUI_DARKGREEN);
				/*填充容量*/
				GUI_FillRect(24, 3 , 27, 9);
				GUI_FillRect(18, 3 , 21, 9);
				GUI_FillRect(12, 3 , 15, 9);
				GUI_FillRect(6, 3 , 9, 9);
				break;
		}
		/*画电池外形*/
		GUI_FillRect(0, 4 , 1, 8);
		GUI_DrawRect(2, 0, WIN_BATTERY_XSIZE - 1, WIN_BATTERY_YSIZE - 1);
	}
#endif
}

int Get_Display_Number_Of_Lines(const char GUI_UNI_PTR *s, GUI_RECT *pRect, int TextAlign, GUI_WRAPMODE WrapMode)
{
	GUI_RECT Rect;
	int xSize, NumBytesToNextLine, NumLines = 0;
	if (!s)
	{
		return 0;
	}
	Rect      = *pRect;
	xSize     = Rect.x1 - Rect.x0 + 1;
	/* Vertical alignment */
//    if ((TextAlign & GUI_TA_VERTICAL) != GUI_TA_TOP)
	{
		const char GUI_UNI_PTR *pText;
		pText = s;
		/* Count the number of lines */
		do
		{
			NumBytesToNextLine = GUI__WrapGetNumBytesToNextLine(pText, xSize, WrapMode);
			pText += NumBytesToNextLine;
			if (NumBytesToNextLine)
			{
				NumLines++;
			}
		}
		while (NumBytesToNextLine);
	}
	return NumLines;
}

void Config_Button_Property(WM_HWIN hWin)
{
	BUTTON_SetBkColor(hWin, 0, _RGB(94, 109, 131));
	BUTTON_SetBkColor(hWin, 1, _RGB(50, 60, 80));
	BUTTON_SetTextColor(hWin, 0, BUTTON_TEXT_COLOR_RELEASED);
	BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
	BUTTON_SetFocussable(hWin, 0);
}

WM_HWIN Create_Left_Button(WM_HWIN hParent)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(2,  LCD_YSIZE - Bottom_Height + 2, LEFT_BUTTON_WIDTH, Bottom_Height - 4,
						   hParent,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE, BUTTON_ID_LEFT);
	BUTTON_SetFont(hWin, _GetFont(Font_Left_Button));
	Config_Button_Property(hWin);
	return hWin;
}

WM_HWIN Create_Right_Button(WM_HWIN hParent)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(LCD_XSIZE - RIGHT_BUTTON_WIDTH - 2,  LCD_YSIZE - Bottom_Height + 2, RIGHT_BUTTON_WIDTH, Bottom_Height - 4,
						   hParent,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , BUTTON_ID_RIGHT);
	BUTTON_SetFont(hWin, _GetFont(Font_Right_Button));
	Config_Button_Property(hWin);
	return hWin;
}

WM_HWIN Create_Middle_Button(WM_HWIN hParent)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx((LCD_XSIZE - MIDDLE_BUTTON_WIDTH) >> 1,  LCD_YSIZE - Bottom_Height + 2, MIDDLE_BUTTON_WIDTH, Bottom_Height - 4,
						   hParent,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , BUTTON_ID_MIDDLE);
	BUTTON_SetFont(hWin, _GetFont(Font_Middle_Button));
	Config_Button_Property(hWin);
	return hWin;
}


WM_HWIN Create_Dialog_Caption(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin = NULL;
	GUI_RECT rect;
	const char *pStr = (const char *)(pMsg->Data.p);

	if(pStr)
	{
		GUI_SetFont(_GetFont(Font_Title));
		GUI_GetTextExtend(&rect, pStr, 50);
		hWin = TEXT_CreateEx(0, (Caption_Height - (rect.y1 - rect.y0 + 1)) >> 1, LCD_XSIZE, rect.y1 - rect.y0 + 3,
							 pMsg->hWin,
							 WM_CF_SHOW, 0, 0, pStr);
		TEXT_SetFont(hWin, _GetFont(Font_Title));
		TEXT_SetTextColor(hWin, _RGB(80, 0, 71));
		TEXT_SetTextAlign(hWin, GUI_TA_CENTER);
	}

	return hWin;
}

void Set_Dialog_Caption_Content(const char *s,WM_HWIN hCaption)
{
	TEXT_SetText(hCaption, s);
}

char * Get_Language_String(unsigned short nStringIndex)
{    
	return (char *)Strings[g_Language][nStringIndex];
}

const GUI_FONT *_GetFont(eWindowFontSelection nWindowIndex)
{
	if(g_Language == eLanguage_Chinese)
	{
		switch(nWindowIndex)
		{
			case Font_Title:
				return &GUI_FontSongTiBold19;

			case Font_Content:
				return &GUI_FontSongTi24;

			case Font_WarningContent:
				return &GUI_FontSongTi18_Warning;

			default:
				return &GUI_Font16_UTF8;
		}
		
	}
	
	return &GUI_Font16_UTF8;
}

/*********************************************************************************************************************************/

static sWinManageList   *_pWM_List = NULL;

void AddWindowToList(sWinManageList *pWin)
{
	sWinManageList *pIndex;

	if(_pWM_List == NULL)
	{
		_pWM_List = pWin;
	}
	else
	{
		pIndex = _pWM_List;
		while(pIndex->pNext)
		{
			pIndex = pIndex->pNext;
		}

		pWin->pPrev = pIndex;
		pWin->pNext = pIndex->pNext;
		pIndex->pNext = pWin;
	}
}

void DeleteWindowFromList(WM_HWIN hWin)
{
	sWinManageList *pIndex;

	if(_pWM_List == NULL)
	{
		return;
	}

	/*在链表中查找*/
	pIndex = _pWM_List;
	while(pIndex)
	{
		if(pIndex->hWin == hWin)
		{
			break;
		}
		pIndex = pIndex->pNext;
	}

	if(pIndex == NULL)
	{
		return;
	}

	if(pIndex == _pWM_List)
	{
		if(pIndex->pNext == NULL)
		{
			_pWM_List = NULL;
		}
		else
		{
			_pWM_List = pIndex->pNext;
			_pWM_List->pPrev = NULL;
		}
	}
	else
	{
		if(pIndex->pNext)
		{
			pIndex->pNext->pPrev = pIndex->pPrev;
		}
		pIndex->pPrev->pNext = pIndex->pNext;
	}
}

/*在链表中查找指定窗口类型的管理模块*/
sWinManageList *FindWMBByWinType(eUserWindow WinType)
{
	sWinManageList *pResult;

	if(_pWM_List == NULL)
	{
		return 0;
	}

	pResult = _pWM_List;

	while(pResult)
	{
		if(pResult->eWinType == WinType)
		{
			return pResult;
		}
		pResult = pResult->pNext;
	}

	return NULL;
}

/*在链表中查找指定的窗口类型是否已存在*/
WM_HWIN GetWinHandleByWinType(eUserWindow WinType)
{
	sWinManageList *pResult;

	if(_pWM_List == NULL)
	{
		return 0;
	}

	pResult = _pWM_List;

	while(pResult)
	{
		if(pResult->eWinType == WinType)
		{
			return pResult->hWin;
		}
		pResult = pResult->pNext;
	}

	return NULL;
}

/*从链表中查找指定句柄的管理模块*/
sWinManageList *FindWMBFromListByWin(WM_HWIN  hWin)
{
	sWinManageList *pResult;

	if(_pWM_List == NULL)
	{
		return NULL;
	}

	pResult = _pWM_List;

	while(pResult)
	{
		if(pResult->hWin == hWin)
		{
			return pResult;
		}

		pResult = pResult->pNext;
	}

	return NULL;
}

/*查找链表中第一个窗体句柄*/
WM_HWIN GetFirstWinFromList(void)
{
	sWinManageList *pResult;

	if(_pWM_List == NULL)
	{
		return (WM_HWIN)NULL;
	}

	pResult = _pWM_List;

	return (pResult->hWin);
}

/*查找链表中最后一个窗体句柄*/
WM_HWIN GetLastWinFromList(void)
{
	sWinManageList *pResult;

	if(_pWM_List == NULL)
	{
		return (WM_HWIN)NULL;
	}

	pResult = _pWM_List;

	while(pResult->pNext)
	{
		pResult = pResult->pNext;
	}

	return (pResult->hWin);
}

/*查找链表中最后一个窗体的结构体指针*/
sWinManageList *GetLastWMBFromList(void)
{
	sWinManageList *pResult;

	if(_pWM_List == NULL)
	{
		return (sWinManageList *)NULL;
	}

	pResult = _pWM_List;

	while(pResult->pNext)
	{
		pResult = pResult->pNext;
	}

	return (pResult);
}

/*查找链表中最后一个窗体的窗体类型*/
eUserWindow GetLastWinTypeFromList(void)
{
	sWinManageList *pResult;

	if(_pWM_List == NULL)
	{
		return (eUserWindow)NULL;
	}

	pResult = _pWM_List;

	while(pResult->pNext)
	{
		pResult = pResult->pNext;
	}

	return (pResult->eWinType);
}

