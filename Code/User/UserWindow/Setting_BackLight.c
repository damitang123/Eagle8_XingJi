#include "Windows.h"
#include "CustomDisplayDriver.h"
#include "data.h"

static CW_BackLight *pThisWin;

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	GUI_RECT rect;

	pMsg->Data.p = _BackLight;
	pThisWin->hCaption = Create_Dialog_Caption(pMsg);

	/*创建左键*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);
	/*创建右键*/
	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _Default);
	/*创建中键*/
	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);

	/*背光亮度*/
	GUI_SetFont(_GetFont(Font_SetAlcUnit));
	GUI_GetTextExtend(&rect, _BackLightBrightness, 50);
	hWin = TEXT_CreateEx(10, 75, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1 + 5,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, _BackLightBrightness);
	TEXT_SetTextColor(hWin, GUI_BLACK);
	TEXT_SetWrapMode(hWin, GUI_WRAPMODE_WORD);
	TEXT_SetFont(hWin, _GetFont(Font_SetAlcUnit));
	TEXT_SetTextAlign(hWin, GUI_TA_HCENTER | GUI_TA_VCENTER);

	/*调节滚动条*/
	pThisWin->hSliderValue = SLIDER_CreateEx(10, 115, 200, 35, pMsg->hWin, WM_CF_SHOW, 0, 0);
	SLIDER_SetNumTicks(pThisWin->hSliderValue, 10);
	SLIDER_SetRange(pThisWin->hSliderValue, 0, 100);
	SLIDER_SetValue(pThisWin->hSliderValue, g_sSystemParam.nBackLightPercent);
	SLIDER_SetBkColor(pThisWin->hSliderValue, GUI_INVALID_COLOR);
	

	/*背光超时设置*/
	GUI_SetFont(_GetFont(Font_SetAlcUnit));
	GUI_GetTextExtend(&rect, _BackLightBrightness, 50);
	pThisWin->hButtonBackLightTimeout = BUTTON_CreateEx(10, 190, 200, 26,
	                       pMsg->hWin,
	                       WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(pThisWin->hButtonBackLightTimeout, _GetFont(Font_Left_Button));
	BUTTON_SetBkColor(pThisWin->hButtonBackLightTimeout, 0, _RGB(94, 109, 131));
	BUTTON_SetBkColor(pThisWin->hButtonBackLightTimeout, 1, _RGB(50, 60, 80));
	BUTTON_SetTextColor(pThisWin->hButtonBackLightTimeout, 0, GUI_WHITE);
	BUTTON_SetTextColor(pThisWin->hButtonBackLightTimeout, 1, GUI_YELLOW);
	BUTTON_SetFocussable(pThisWin->hButtonBackLightTimeout, 0);
	BUTTON_SetText(pThisWin->hButtonBackLightTimeout, _BackLightTimeout);

	WM_SetFocus(pThisWin->hSliderValue);
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
						Lcd_Back_Light_Pwm_Pulse_Config(Get_Actual_LcdBackLight_Percent(g_sSystemParam.nBackLightPercent));
						WM_DeleteWindow(pMsg->hWin);
						break;
						
					case GUI_KEY_RIGHT_FUN:
						SLIDER_SetValue(pThisWin->hSliderValue, 5);
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						if(g_sSystemParam.nBackLightPercent != SLIDER_GetValue(pThisWin->hSliderValue))
						{
							g_sSystemParam.nBackLightPercent = SLIDER_GetValue(pThisWin->hSliderValue);
							Spi_Flash_Init();
							Data_System_Parameter_Save(&g_sSystemParam);
							Spi_Flash_DeInit();
						}
						WM_DeleteWindow(pMsg->hWin);
						break;
				}
			}
			return;
			
		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pThisWin->hSliderValue);
					break;

				case WM_NOTIFICATION_VALUE_CHANGED:
					pThisWin->nSliderValue = SLIDER_GetValue(pThisWin->hSliderValue) + 5;
					Lcd_Back_Light_Pwm_Pulse_Config(Get_Actual_LcdBackLight_Percent(pThisWin->nSliderValue));
					break;

				case WM_NOTIFICATION_RELEASED:
					if(pMsg->hWinSrc == pThisWin->hButtonBackLightTimeout)
					{
						CW_Setting_BackLightTimeout_Create(pMsg->hWin, (void *)1);
					}
					break;
			}
			return;
			
		case WM_DELETE:
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("SettingBackLightWindow window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_BackLight *CW_Setting_BackLight_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSettingBacklight;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSettingBacklight window is alreay exist!\r\n");
		return (CW_BackLight *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_BackLight));

	if(hWin)
	{
		TRACE_PRINTF("eSettingBacklight windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_BackLight *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSettingBacklight window can't be created!\r\n");
	}

	return pThisWin;
}

