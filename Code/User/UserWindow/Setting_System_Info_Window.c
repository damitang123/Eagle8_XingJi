#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"
#include "time_calc.h"

static CW_SystemInfo *pThisWin;

static void _Create(WM_MESSAGE *pMsg)
{
	char aTemp[50];
	
	pMsg->Data.p = _SystemInformation;
	pThisWin->hCaption = Create_Dialog_Caption(pMsg);

	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hMiddleButton, _OK);

	pThisWin->hListbox = LISTBOX_CreateEx(0, Caption_Height, LCD_XSIZE, LCD_YSIZE - Caption_Height - Bottom_Height,
	                        pMsg->hWin,
	                        WM_CF_SHOW | WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_BOTTOM, 0, GUI_ID_LISTBOX0,
	                        NULL);
	LISTBOX_SetFont(pThisWin->hListbox, _GetFont(Font_ListboxForSystemInfor));
	LISTBOX_SetItemSpacing(pThisWin->hListbox, LISTBOX_GetItemSpacing(pThisWin->hListbox) + 30);
	LISTBOX_SetAutoScrollV(pThisWin->hListbox, 1);
	LISTBOX_SetTextAlign(pThisWin->hListbox, GUI_TA_LEFT | GUI_TA_VCENTER);
	LISTBOX_SetScrollbarWidth(pThisWin->hListbox, 15);

	sprintf(aTemp, "%s:%s", _Hardware, HW_VERSION);
	LISTBOX_AddString(pThisWin->hListbox, aTemp);

	sprintf(aTemp, "%s:%s", _Software, FW_VERSION);
	LISTBOX_AddString(pThisWin->hListbox, aTemp);

	sprintf(aTemp, "%s:%s", "IMEI    ", g_sSystemParam.aIMEI);
	LISTBOX_AddString(pThisWin->hListbox, aTemp);

	sprintf(aTemp, "%s:%s%06ld", _DeviceNo, g_sDeviceIDConfig.aNamePrefix,g_sDeviceIDConfig.nDeviceID);
	LISTBOX_AddString(pThisWin->hListbox, aTemp);

	sprintf(aTemp, "%s:%04d/%02d/%02d",
	        _CalibrateDate, g_sAlcCalibrationParam.aDateTimeIntergral[0] + 2000,
	        g_sAlcCalibrationParam.aDateTimeIntergral[1], g_sAlcCalibrationParam.aDateTimeIntergral[2]);
	LISTBOX_AddString(pThisWin->hListbox, aTemp);

	WM_SetFocus(pThisWin->hListbox);
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
					case GUI_KEY_MIDDLE_FUN:
						WM_DeleteWindow(pMsg->hWin);
						break;
				}
			}
			return;

		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pThisWin->hListbox);
					break;
			}
			break;
			
		case WM_DELETE:
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eSettingSystemInfo window is deleted!\r\n");
	}
	WM_DefaultProc(pMsg);
}


CW_SystemInfo *CW_Setting_SystemInfo_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSettingSystemInfo;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSettingSystemInfo window is alreay exist!\r\n");
		return (CW_SystemInfo *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_SystemInfo));

	if(hWin)
	{
		TRACE_PRINTF("eSettingSystemInfo windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_SystemInfo *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSettingSystemInfo window can't be created!\r\n");
	}

	return pThisWin;
}

