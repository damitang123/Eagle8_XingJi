#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"

static CW_SetBlow *pThisWin;

static void _Config_Button_Property(WM_HWIN hWin, unsigned char nRightButton)
{
	BUTTON_SetBkColor(hWin, 0, 0xB59179);
	BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
	BUTTON_SetFocussable(hWin, 0);
	BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
	BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);

	if(nRightButton)
	{
		BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmRight_Arrows_Released, hWin));
		BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmRight_Arrows_Pressed, hWin));
	}
	else
	{
		BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmLeft_Arrows_Released, hWin));
		BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmLeft_Arrows_Pressed, hWin));
	}
}

static void Set_BlowTime_Text(unsigned long nBlowTime)
{
	char aTemp[20];
	sprintf(aTemp, "%.1f %s", ((float)nBlowTime / 1000), _Seconds);
	EDIT_SetText(pThisWin->hBlowTime, aTemp);
}

static void Set_BlowPress_Text(unsigned long nBlowPress)
{
	char aTemp[20];
	sprintf(aTemp, "%d %s", nBlowPress, _Level);
	EDIT_SetText(pThisWin->hBlowPress, aTemp);
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	GUI_RECT rect;
	
	pMsg->Data.p = _BlowParameter;
	Create_Dialog_Caption(pMsg);

	pThisWin->nBlowTime = g_sTestParam.nBlowTime;
	pThisWin->nBlowPress = g_sTestParam.nBlowPress;

	/*吹气时间设置*/
	GUI_SetFont(_GetFont(Font_SetAlcUnit));
	GUI_GetTextExtend(&rect, _BlowTime, 50);
	hWin = TEXT_CreateEx(4, 50, LCD_XSIZE - 8, GUI_GetFontSizeY() * 2 + 3,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, _BlowTime);
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetWrapMode(hWin, GUI_WRAPMODE_WORD);
	TEXT_SetFont(hWin, _GetFont(Font_SetAlcUnit));
	TEXT_SetTextAlign(hWin, GUI_TA_HCENTER | GUI_TA_VCENTER);

	GUI_SetFont(_GetFont(Font_UnitsBlow));
	pThisWin->hBlowTime = EDIT_CreateEx((LCD_XSIZE - 130) >> 1, 90, 130, 27,
	                     pMsg->hWin, WM_CF_SHOW | WM_CF_DISABLED, 0, 0, 20);
	EDIT_SetFont(pThisWin->hBlowTime, _GetFont(Font_UnitsBlow));
	EDIT_SetTextAlign(pThisWin->hBlowTime, GUI_TA_VCENTER | GUI_TA_HCENTER);
	Set_BlowTime_Text(pThisWin->nBlowTime);

	/*调整按钮*/
	pThisWin->hBlowTimeDec = BUTTON_CreateEx(20, 90, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hBlowTimeDec, 0);

	pThisWin->hBlowTimeInc = BUTTON_CreateEx(LCD_XSIZE - 50, 90, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hBlowTimeInc, 1);

	/*吹气压力等级设置*/
	GUI_SetFont(_GetFont(Font_ToolWin_Content));
	GUI_GetTextExtend(&rect, _BlowStrengthLevel, 50);
	hWin = TEXT_CreateEx(4, 150, LCD_XSIZE - 8, GUI_GetFontSizeY() * 2,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, _BlowStrengthLevel);
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetWrapMode(hWin, GUI_WRAPMODE_WORD);
	TEXT_SetFont(hWin, _GetFont(Font_ToolWin_Content));
	TEXT_SetTextAlign(hWin, GUI_TA_HCENTER | GUI_TA_VCENTER);

	GUI_SetFont(_GetFont(Font_UnitsBlow));
	pThisWin->hBlowPress = EDIT_CreateEx((LCD_XSIZE - 130) >> 1, 190, 130, 27,
	                     pMsg->hWin, WM_CF_SHOW | WM_CF_DISABLED, 0, 0, 20);
	EDIT_SetFont(pThisWin->hBlowPress, _GetFont(Font_UnitsBlow));
	EDIT_SetTextAlign(pThisWin->hBlowPress, GUI_TA_VCENTER | GUI_TA_HCENTER);
	Set_BlowPress_Text(pThisWin->nBlowPress);

	/*调整按钮*/
	pThisWin->hBlowPressDec = BUTTON_CreateEx(20, 190, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hBlowPressDec, 0);

	pThisWin->hBlowPressInc = BUTTON_CreateEx(LCD_XSIZE - 50, 190, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hBlowPressInc, 1);


	/*创建按钮*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);

	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _Default);

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
						pThisWin->nBlowTime = 3000;
						pThisWin->nBlowPress = 1;
						Set_BlowTime_Text(pThisWin->nBlowTime);
						Set_BlowPress_Text(pThisWin->nBlowPress);
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						g_sTestParam.nBlowTime = pThisWin->nBlowTime;
						 g_sTestParam.nBlowPress = pThisWin->nBlowPress;
						Spi_Flash_Init();
						Data_Test_Parameter_Save(&g_sTestParam);
						Data_Test_Parameter_Load(&g_sTestParam);
						Spi_Flash_DeInit();
						WM_DeleteWindow(pMsg->hWin);
						break;
				}
			}
			return;
			
		case WM_NOTIFY_PARENT:
			if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
			{
				WM_HWIN hWin;
				hWin = pMsg->hWinSrc;

				if(hWin == pThisWin->hBlowTimeDec)
				{	
					if(pThisWin->nBlowTime > 2000)
					{
						pThisWin->nBlowTime -= 500;
					}
					else
					{
						pThisWin->nBlowTime = 5000;
					}
					Set_BlowTime_Text(pThisWin->nBlowTime);
				}
				else if(hWin == pThisWin->hBlowTimeInc)
				{
					if(pThisWin->nBlowTime < 5000)
					{
						pThisWin->nBlowTime += 500;
					}
					else
					{
						pThisWin->nBlowTime = 2000;
					}
					Set_BlowTime_Text(pThisWin->nBlowTime);
				}
				
				if(hWin == pThisWin->hBlowPressDec)
				{
					if(pThisWin->nBlowPress > 1)
					{
						pThisWin->nBlowPress -= 1;
					}
					else 
					{
						pThisWin->nBlowPress = 3;
					}
					Set_BlowPress_Text(pThisWin->nBlowPress);
					
				}
				else if(hWin == pThisWin->hBlowPressInc)
				{
					if(pThisWin->nBlowPress < 3)
					{
						pThisWin->nBlowPress += 1;
					}
					else 
					{
						pThisWin->nBlowPress = 1;
					}
					Set_BlowPress_Text(pThisWin->nBlowPress);
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
			TRACE_PRINTF("eSetBlow window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	WM_DefaultProc(pMsg);
}

CW_SetBlow *CW_SetBlow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSetBlow;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSetBlow window is alreay exist!\r\n");
		return (CW_SetBlow *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_SetBlow));

	if(hWin)
	{
		TRACE_PRINTF("eSetBlow windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_SetBlow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSetBlow window can't be created!\r\n");
	}

	return pThisWin;
}

