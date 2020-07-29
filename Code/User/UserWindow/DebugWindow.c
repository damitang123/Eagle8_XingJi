#include "platform.h"
#include "Windows.h"
#include "adc.h"
#include "ads8320.h"
#include "gpio.h"
#include "system_misc.h"

static CW_Debug *pThisWin;

static void GUI_Timer_Debug_Callback(GUI_TIMER_MESSAGE *pTM)
{
	GUI_TIMER_Restart(pThisWin->hTMDebug);
	WM_SendMessageNoPara((WM_HWIN)pTM->Context, WM_ALCOHOL_TEST);
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	pMsg->Data.p = _DeviceDebug;
	Create_Dialog_Caption(pMsg);

	pThisWin->bInhalTest = false;
	pThisWin->nInhalTestCnt = 0;

	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);

	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _Inhal);

	hWin = LISTBOX_CreateEx(0, Caption_Height, LCD_XSIZE, LCD_YSIZE - Caption_Height - Bottom_Height,
	                        pMsg->hWin,
	                        WM_CF_SHOW | WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_BOTTOM, 0, GUI_ID_LISTBOX0,
	                        NULL);
	LISTBOX_SetFont(hWin,  &GUI_FontTahoma23);
	LISTBOX_SetItemSpacing(hWin, LISTBOX_GetItemSpacing(hWin) + 13);
	LISTBOX_SetAutoScrollV(hWin, 1);
	LISTBOX_SetTextAlign(hWin, GUI_TA_LEFT | GUI_TA_VCENTER);

	LISTBOX_AddString(hWin, (char *)"");
	LISTBOX_AddString(hWin, (char *)"");
	LISTBOX_AddString(hWin, (char *)"");
	LISTBOX_AddString(hWin, (char *)"");
	LISTBOX_AddString(hWin, (char *)"");
	LISTBOX_AddString(hWin, (char *)"");
	LISTBOX_AddString(hWin, (char *)"");

	Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,false);
	Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl,false);

	pThisWin->hTMDebug = GUI_TIMER_Create(GUI_Timer_Debug_Callback, Get_System_Time(), (unsigned long)pMsg->hWin, 0);
	GUI_TIMER_SetPeriod(pThisWin->hTMDebug, 300);

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

					case GUI_KEY_MIDDLE_FUN:
						break;
						
					case GUI_KEY_RIGHT_FUN:
						pThisWin->bInhalTest = true;
						pThisWin->nInhalTestCnt = 0;
						Gpio_Ctrl(eGpio_Pump_Charge_Sw_Ctrl,true);
						Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,false);
						break;
				}
			}
			return;
	
		case WM_ALCOHOL_TEST:
			/*每秒更新一次ADC传感器的值*/
			{
				char aTemp[50];
				WM_HWIN hWin;
				char TemperatureUnit[] = {0xE2, 0x84, 0x83, 0x00}; /*℃*/

				hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX0);

				sprintf(aTemp, "Pump       :%d", Adc_Get_PumpVoltage());
				LISTBOX_SetString(hWin, (char *)aTemp, 0);

				sprintf(aTemp, "Press        :%d", Adc_Get_PressSensorValue());
				LISTBOX_SetString(hWin, (char *)aTemp, 1);

				sprintf(aTemp, "SENSOR    :%d, %.2f%s", Adc_Get_Alc_Sensor_Temprature_Value(), Adc_Get_Sensor_Temperature(), TemperatureUnit);
				LISTBOX_SetString(hWin, (char *)aTemp, 2);

				sprintf(aTemp, "TC1047     :%d, %.2f%s", Adc_Get_TC1047_Value(), Adc_Get_Environment_Temperature(), TemperatureUnit);
				LISTBOX_SetString(hWin, (char *)aTemp, 3);

				sprintf(aTemp, "Battery     :%d, %.2fV", Adc_Get_Battery_Value(), Adc_Get_Battery_Voltage());
				LISTBOX_SetString(hWin, (char *)aTemp, 4);

				sprintf(aTemp, "Fuel Cell    :%d", Adc_Get_Alcohol_Sensor_Value());
				LISTBOX_SetString(hWin, (char *)aTemp, 5);
			}

			if(pThisWin->bInhalTest)
			{
				if(pThisWin->nInhalTestCnt == 0 && Adc_Is_Pump_Ready())
				{
					pThisWin->nInhalTestCnt++;
					Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,true);
				}
				else
				{
					if(pThisWin->nInhalTestCnt)
					{
						if(++pThisWin->nInhalTestCnt > 12)
						{
							pThisWin->bInhalTest = false;
							pThisWin->nInhalTestCnt = 0;
							Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,false);
							Gpio_Ctrl(eGpio_Pump_Charge_Sw_Ctrl,false);
						}
					}
				}
			}
			break;

		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX0));
					break;
			}
			return;
			
		case WM_DELETE:
			Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl,true);
			Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl,true);
			Gpio_Ctrl(eGpio_Pump_Charge_Sw_Ctrl,false);
			Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl,false);
			GUI_TIMER_Delete(pThisWin->hTMDebug);
			pThisWin->hTMDebug = NULL;
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eDebug will be destroyed!\r\n");
			break;
	}
	WM_DefaultProc(pMsg);
}

CW_Debug *CW_Debug_Window_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eDebug;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eDebug window is alreay exist!\r\n");
		return (CW_Debug *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_Debug));

	if(hWin)
	{
		TRACE_PRINTF("eDebug windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_Debug *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eDebug window can't be created!\r\n");
	}

	return pThisWin;
}



