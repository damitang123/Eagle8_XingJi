#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"

static CW_SetALCUnits *pThisWin;

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

static void Set_CurUnitType_Text(unsigned long nCurUnitType)
{
	char aTemp[20];
	sprintf(aTemp, "%s", pThisWin->nCurUnitType == 0 ? _BAC : _BrAC);
	EDIT_SetText(pThisWin->hCurUnitType, aTemp);
}

static void Set_TestUnit_Text(unsigned long nTestUnit)
{
	char aTemp[20];
	sprintf(aTemp, "%s", Get_Unit_String(nTestUnit));
	EDIT_SetText(pThisWin->hTestUnit, aTemp);
}

static void Set_BrACBACK_Text(unsigned long nBrACBACK)
{
	char aTemp[20];
	sprintf(aTemp, "%d", nBrACBACK);
	EDIT_SetText(pThisWin->hBrACBACK, aTemp);
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	GUI_RECT rect;
	
	pMsg->Data.p = _AlcUnit;
	Create_Dialog_Caption(pMsg);

	pThisWin->nTestUnit = g_sTestParam.nTestUnit;
	pThisWin->nBrACBACK = g_sTestParam.nBrACBACK;
	pThisWin->nCurUnitType = g_sTestParam.nTestUnit <= 4 ? 0 : 1;

	/*测试单位类型*/
	GUI_SetFont(_GetFont(Font_SetAlcUnit));
	GUI_GetTextExtend(&rect, _AlcUnitCategory, 50);
	hWin = TEXT_CreateEx(4, 40, LCD_XSIZE - 8, GUI_GetFontSizeY() * 2 + 3,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, _AlcUnitCategory);
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetWrapMode(hWin, GUI_WRAPMODE_WORD);
	TEXT_SetFont(hWin, _GetFont(Font_SetAlcUnit));
	TEXT_SetTextAlign(hWin, GUI_TA_HCENTER | GUI_TA_VCENTER);

	GUI_SetFont(_GetFont(Font_UnitsBlow));
	pThisWin->hCurUnitType = EDIT_CreateEx((LCD_XSIZE - 130) >> 1, 80, 130, 27,
	                     pMsg->hWin, WM_CF_SHOW | WM_CF_DISABLED, 0, 0, 10);
	EDIT_SetFont(pThisWin->hCurUnitType, _GetFont(Font_UnitsBlow));
	EDIT_SetTextAlign(pThisWin->hCurUnitType, GUI_TA_VCENTER | GUI_TA_HCENTER);
	Set_CurUnitType_Text(pThisWin->nCurUnitType);

	/*调整按钮*/
	pThisWin->hCurUnitTypeDec = BUTTON_CreateEx(20, 80, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hCurUnitTypeDec, 0);

	pThisWin->hCurUnitTypeInc = BUTTON_CreateEx(LCD_XSIZE - 50, 80, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hCurUnitTypeInc, 1);

	/*显示单位*/
	GUI_SetFont(_GetFont(Font_SetAlcUnit));
	GUI_GetTextExtend(&rect, _AlcUnitType, 50);
	if(g_Language == eLanguage_English || g_Language == eLanguage_Chinese)
	{
		hWin = TEXT_CreateEx(4, 130, LCD_XSIZE - 8, GUI_GetFontSizeY() + 3,
		                     pMsg->hWin,
		                     WM_CF_SHOW, 0, 0, _AlcUnitType);
	}
	else if(g_Language == eLanguage_Russian || g_Language == eLanguage_Vietnamese || g_Language == eLanguage_Spanish)
	{
		hWin = TEXT_CreateEx(4, 112, LCD_XSIZE - 8, GUI_GetFontSizeY() * 2 + 3,
		                     pMsg->hWin,
		                     WM_CF_SHOW, 0, 0, _AlcUnitType);
		TEXT_SetWrapMode(hWin, GUI_WRAPMODE_WORD);
	}
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetFont(hWin, _GetFont(Font_SetAlcUnit));
	TEXT_SetTextAlign(hWin, GUI_TA_HCENTER | GUI_TA_VCENTER);
	

	GUI_SetFont(_GetFont(Font_UnitsBlow));
	pThisWin->hTestUnit = EDIT_CreateEx((LCD_XSIZE - 130) >> 1, 160, 130, 27,
	                     pMsg->hWin, WM_CF_SHOW | WM_CF_DISABLED, 0, 0, 10);
	EDIT_SetFont(pThisWin->hTestUnit, _GetFont(Font_UnitsBlow));
	EDIT_SetTextAlign(pThisWin->hTestUnit, GUI_TA_VCENTER | GUI_TA_HCENTER);
	Set_TestUnit_Text(pThisWin->nTestUnit);

	/*调整按钮*/
	pThisWin->hTestUnitDec = BUTTON_CreateEx(20, 160, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hTestUnitDec, 0);

	pThisWin->hTestUnitInc = BUTTON_CreateEx(LCD_XSIZE - 50, 160, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hTestUnitInc, 1);

	/*单位转换系数*/
	GUI_SetFont(_GetFont(Font_SetAlcUnit));
	GUI_GetTextExtend(&rect, _AlcConvFactor, 100);
	if(g_Language == eLanguage_English || g_Language == eLanguage_Chinese)
	{
		hWin = TEXT_CreateEx(4, 200, LCD_XSIZE - 8, GUI_GetFontSizeY() * 2 + 5,
		                     pMsg->hWin,
		                     WM_CF_SHOW, 0, 0, _AlcConvFactor);
	}
	else if(g_Language == eLanguage_Russian || g_Language == eLanguage_Vietnamese || g_Language == eLanguage_Spanish)
	{
		hWin = TEXT_CreateEx(4, 186, LCD_XSIZE - 8, GUI_GetFontSizeY() * 3 + 5,
		                     pMsg->hWin,
		                     WM_CF_SHOW, 0, 0, _AlcConvFactor);
	}
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetWrapMode(hWin, GUI_WRAPMODE_WORD);
	TEXT_SetFont(hWin, _GetFont(Font_SetAlcUnit));
	TEXT_SetTextAlign(hWin, GUI_TA_HCENTER | GUI_TA_VCENTER);

	GUI_SetFont(_GetFont(Font_UnitsBlow));
	pThisWin->hBrACBACK = EDIT_CreateEx((LCD_XSIZE - 130) >> 1, 250, 130, 27,
	                     pMsg->hWin, WM_CF_SHOW | WM_CF_DISABLED, 0, 0, 10);
	EDIT_SetFont(pThisWin->hBrACBACK, _GetFont(Font_UnitsBlow));
	EDIT_SetTextAlign(pThisWin->hBrACBACK, GUI_TA_VCENTER | GUI_TA_HCENTER);	
	Set_BrACBACK_Text(pThisWin->nBrACBACK);
	
	/*调整按钮*/
	pThisWin->hBrACBACKDec = BUTTON_CreateEx(20, 250, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hBrACBACKDec, 0);

	pThisWin->hBrACBACKInc = BUTTON_CreateEx(LCD_XSIZE - 50, 250, 30, 27,
	                       pMsg->hWin,
	                       WM_CF_SHOW, 1, 0);
	_Config_Button_Property(pThisWin->hBrACBACKInc, 1);

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
						pThisWin->nTestUnit = 0;
						pThisWin->nBrACBACK = 2200;
						pThisWin->nCurUnitType = 0;
						Set_CurUnitType_Text(pThisWin->nCurUnitType);
						Set_TestUnit_Text(pThisWin->nTestUnit);
						Set_BrACBACK_Text(pThisWin->nBrACBACK);
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						g_sTestParam.nTestUnit = pThisWin->nTestUnit;
						g_sTestParam.nBrACBACK = pThisWin->nBrACBACK;
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

				if(hWin == pThisWin->hCurUnitTypeDec || hWin == pThisWin->hCurUnitTypeInc)
				{	
					if(pThisWin->nCurUnitType == 0)
					{
						pThisWin->nCurUnitType = 1;
						pThisWin->nTestUnit = 5;
					}
					else
					{
						pThisWin->nCurUnitType = 0;
						pThisWin->nTestUnit = 0;
					}
					Set_CurUnitType_Text(pThisWin->nCurUnitType);
					Set_TestUnit_Text(pThisWin->nTestUnit);
				}
				
				if(hWin == pThisWin->hTestUnitDec)
				{
					if(pThisWin->nCurUnitType == 0)
					{
						if(pThisWin->nTestUnit)
						{
							pThisWin->nTestUnit--;
						}
						else
						{
							pThisWin->nTestUnit = 5;
						}
					}
					else if(pThisWin->nCurUnitType == 1)
					{
						if(pThisWin->nTestUnit > 5)
						{
							pThisWin->nTestUnit--;
						}
						else
						{
							pThisWin->nTestUnit = 7;
						}
					}
					Set_TestUnit_Text(pThisWin->nTestUnit);
				}
				else if(hWin == pThisWin->hTestUnitInc)
				{
					if(pThisWin->nCurUnitType == 0)
					{
						if(pThisWin->nTestUnit < 4)
						{
							pThisWin->nTestUnit++;
						}
						else
						{
							pThisWin->nTestUnit = 0;
						}
					}
					else if(pThisWin->nCurUnitType == 1)
					{
						if(pThisWin->nTestUnit < 7)
						{
							pThisWin->nTestUnit++;
						}
						else
						{
							pThisWin->nTestUnit = 5;
						}
					}
					Set_TestUnit_Text(pThisWin->nTestUnit);
				}
				
				if(hWin == pThisWin->hBrACBACKDec)
				{
					if(pThisWin->nBrACBACK > 2000)
					{
						pThisWin->nBrACBACK -= 50;
					}
					else
					{
						pThisWin->nBrACBACK = 2300;
					}
					Set_BrACBACK_Text(pThisWin->nBrACBACK);
				}
				else if(hWin == pThisWin->hBrACBACKInc)
				{
					if(pThisWin->nBrACBACK < 2300)
					{
						pThisWin->nBrACBACK += 50;
					}
					else
					{
						pThisWin->nBrACBACK = 2000;
					}
					Set_BrACBACK_Text(pThisWin->nBrACBACK);
				}
			}
			else if(pMsg->Data.v == WM_NOTIFICATION_WIN_DELETE)
			{
				WM_SetFocus(pMsg->hWin);
			}
			break;
			
		case WM_DELETE:
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eSetALCUnit window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_SetALCUnits *CW_SetALCUnit_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSetALCUnit;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSetALCUnit window is alreay exist!\r\n");
		return (CW_SetALCUnits *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_SetALCUnits));

	if(hWin)
	{
		TRACE_PRINTF("eSetALCUnit windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_SetALCUnits *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSetALCUnit window can't be created!\r\n");
	}

	return pThisWin;
}

