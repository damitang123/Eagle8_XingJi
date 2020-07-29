#include "platform.h"
#include "Windows.h"
#include "calibrate.h"
#include "test.h"
#include "data.h"
#include "buzzer.h"
#include "system_misc.h"
#include "app.h"

#define TOP_BOTTOM_GAP		40

static void GUI_Timer_ShowDialog_Callback(GUI_TIMER_MESSAGE *pTM)
{
	CW_ShowDialog *pThisWin;
	pThisWin = (CW_ShowDialog *)FindWMBFromListByWin((WM_HWIN)pTM->Context);

	GUI_TIMER_Restart(pThisWin->hTimer);
	WM_SendMessageNoPara((WM_HWIN)pTM->Context, WM_WINDOW_TIMER);
}

static void User_Delete_Window(WM_MESSAGE *pMsg)
{
	sWinManageList * pWM;
	CW_ShowDialog *pThisWin;

	if(WM_IsWindow(pMsg->hWin) == 0)
	{
		return;
	}
	
	pWM = FindWMBFromListByWin(pMsg->hWin);
	pThisWin = (CW_ShowDialog *)FindWMBFromListByWin(pMsg->hWin);
	pThisWin->hPrev = pWM->pPrev->hWin;
	
	if(pWM->pNext)
	{
		WM_AttachWindow(pWM->pNext->hWin, pWM->pPrev->hWin);
		WM_DetachWindow(pMsg->hWin);
		pThisWin->hNext = pWM->pNext->hWin;
	}
	else
	{
		pThisWin->hNext = NULL;
	}

	WM_DeleteWindow(pMsg->hWin);
}

static void _Create(WM_MESSAGE *pMsg)
{
	CW_ShowDialog *pThisWin;
	pThisWin = (CW_ShowDialog *)FindWMBFromListByWin(pMsg->hWin);

	/*中间正文提示语*/
	pThisWin->hContent = TEXT_CreateEx(5, TOP_BOTTOM_GAP + 10, LCD_XSIZE - 5, LCD_YSIZE - TOP_BOTTOM_GAP - 30,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, pThisWin->sProperty.pContent);
	TEXT_SetFont(pThisWin->hContent, pThisWin->sProperty.pFont);
	TEXT_SetWrapMode(pThisWin->hContent, GUI_WRAPMODE_WORD);
	TEXT_SetTextColor(pThisWin->hContent, pThisWin->sProperty.nContentColor);
	
	switch(pThisWin->sProperty.nWinType)
	{
		case eDialogOK:
			pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
			BUTTON_SetText(pThisWin->hMiddleButton, _OK);
			WM_MoveChildTo(pThisWin->hMiddleButton,(LCD_XSIZE - MIDDLE_BUTTON_WIDTH) >> 1, LCD_YSIZE - TOP_BOTTOM_GAP - (Bottom_Height - 4) - 5);
			break;
			
		case eDialogYesNo:
			pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
			BUTTON_SetText(pThisWin->hLeftButton , _Yes);
			WM_MoveChildTo(pThisWin->hLeftButton, 30, LCD_YSIZE - TOP_BOTTOM_GAP - (Bottom_Height - 4) - 5);
		
			pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
			BUTTON_SetText(pThisWin->hRightButton, _No);
			WM_MoveChildTo(pThisWin->hRightButton, LCD_XSIZE - MIDDLE_BUTTON_WIDTH - 20, LCD_YSIZE - TOP_BOTTOM_GAP - (Bottom_Height - 4) - 5);
			break;

		case eDialogCommon:
			pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
			BUTTON_SetText(pThisWin->hLeftButton , pThisWin->sProperty.pLeftButton);
			WM_MoveChildTo(pThisWin->hLeftButton, 30, LCD_YSIZE - TOP_BOTTOM_GAP - (Bottom_Height - 4) - 5);
		
			pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
			BUTTON_SetText(pThisWin->hRightButton, pThisWin->sProperty.pRightButton);
			WM_MoveChildTo(pThisWin->hRightButton, LCD_XSIZE - MIDDLE_BUTTON_WIDTH - 20, LCD_YSIZE - TOP_BOTTOM_GAP - (Bottom_Height - 4) - 5);
			break;
			
		default:
			break;
	}
	
	if(pThisWin->sProperty.nAutoCloseTime)
	{
		pThisWin->hTimer = GUI_TIMER_Create(GUI_Timer_ShowDialog_Callback, Get_System_Time() + 1000, (unsigned long)pMsg->hWin, 0);
		GUI_TIMER_SetPeriod(pThisWin->hTimer, 1000);
	}

	Buzzer_Beep(eBUZ_ERROR);

	WM_SetFocus(pMsg->hWin);
}

static void _Callback(WM_MESSAGE *pMsg)
{
	void(*pFun)(void);
	CW_ShowDialog *pThisWin;
	pThisWin = (CW_ShowDialog *)FindWMBFromListByWin(pMsg->hWin);
	
	Enter_Callback_Hook(pMsg);

	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;

		case WM_PAINT:
			GUI_SetColor(GUI_BLACK);
			GUI_FillRect(0, 0, LCD_XSIZE - 1, TOP_BOTTOM_GAP - 1);
			GUI_FillRect(0, LCD_YSIZE - TOP_BOTTOM_GAP, LCD_XSIZE - 1, LCD_YSIZE - 1);
			GUI_SetColor(pThisWin->sProperty.nBackGroundColor);
			GUI_FillRect(0, TOP_BOTTOM_GAP, LCD_XSIZE - 1, LCD_YSIZE - TOP_BOTTOM_GAP - 1);
			break;
			
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:
						switch(pThisWin->sProperty.nWinType)
						{
							case eDialogYesNo:
								pFun = pThisWin->sProperty.pFunLeftYes;
								User_Delete_Window(pMsg);
								if(pFun) (*pFun)();
								break;
							case eDialogCommon:
								pFun = pThisWin->sProperty.pFunLeftYes;
								User_Delete_Window(pMsg);
								if(pFun) (*pFun)();
								break;
							default:
								break;
						}
						break;
						
					case GUI_KEY_RIGHT_FUN:
						switch(pThisWin->sProperty.nWinType)
						{
							case eDialogYesNo:
								pFun = pThisWin->sProperty.pFunRightNo;
								User_Delete_Window(pMsg);
								if(pFun) (*pFun)();
								break;
							case eDialogCommon:
								pFun = pThisWin->sProperty.pFunRightNo;
								User_Delete_Window(pMsg);
								if(pFun) (*pFun)();
								break;
							default:
								break;
						}
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						switch(pThisWin->sProperty.nWinType)
						{
							case eDialogOK:
								pFun = pThisWin->sProperty.pFunMiddleOK;
								User_Delete_Window(pMsg);
								if(pFun) (*pFun)();
								break;
							default:
								break;
						}
						break;
						
					default:
						break;
				}
			}
			return;

		case WM_WINDOW_TIMER:
			if(-- pThisWin->sProperty.nAutoCloseTime == 0)
			{
				switch(pThisWin->sProperty.nWinType)
				{
					case eDialogOK:
						pFun = pThisWin->sProperty.pFunMiddleOK;
						User_Delete_Window(pMsg);
						if(pFun) (*pFun)();
						break;
					default:
						User_Delete_Window(pMsg);
						break;
				}
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
			GUI_TIMER_Delete(pThisWin->hTimer);
			pThisWin->hTimer = NULL;
			DeleteWindowFromList(pMsg->hWin);
			if((pThisWin->hNext == NULL))
			{
				WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			}
			TRACE_PRINTF("eShowDialog window is deleted!\r\n");
			break;
	}
	
	Exit_Callback_Hook(pMsg);
}


CW_ShowDialog *CW_ShowDialog_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	CW_ShowDialog *pThisWin = NULL;
	eUserWindow WinType = eShowDialog;

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_ShowDialog));

	if(hWin)
	{
		TRACE_PRINTF("eShowDialog windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_ShowDialog *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		pThisWin->sProperty = *(sDialogWindowProperty*)pData;

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eShowDialog window can't be created!\r\n");
	}

	return pThisWin;
}


