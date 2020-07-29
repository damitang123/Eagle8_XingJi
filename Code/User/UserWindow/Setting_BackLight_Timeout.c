#include "Windows.h"
#include "Parameter.h"
#include "data.h"
#include "stm32f10x.h"

static CW_SettingMain *pThisWin;

const static sListProcessTable s_ListCallbackTable[]= 
{
	eNeverSleep ,				NULL,
	eS15Seconds ,			NULL,
	eS30Seconds ,			NULL,
	eS1Minutes ,				NULL,
	eS3Minutes ,				NULL,
	eS5Minutes,				NULL,
};

static unsigned long _Convert_ListIndex_To_Value(unsigned char nIndex)
{
	switch(nIndex)
	{
		case 0:
			return 0;
		case 1:
			return 15 * 1000;
		case 2:
			return 30 * 1000;
		case 3:
			return 60 * 1000;
		case 4:
			return 180 * 1000;
		case 5:
			return 300 * 1000;
		default:
			return 0;
	}
}

static unsigned char _Convert_Value_To_ListIndex(unsigned long nValue)
{
	switch(nValue)
	{
		case 0:
			return 0;
		case 15 * 1000:
			return 1;
		case 30 * 1000:
			return 2;
		case 60 * 1000:
			return 3;
		case 180 * 1000:
			return 4;
		case 300 * 1000:
			return 5;
		default:
			return 0;
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	pMsg->Data.p = _BackLightTimeout;
	pThisWin->hCaption = Create_Dialog_Caption(pMsg);

	pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hLeftButton, __Exit);

	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hMiddleButton, _OK);

	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 0, BtnSetBmp(bmRight_Arrows_Released, pThisWin->hRightButton));
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 1, BtnSetBmp(bmRight_Arrows_Pressed, pThisWin->hRightButton));


	pThisWin->hListbox = LISTBOX_CreateEx(0, Caption_Height, LCD_XSIZE, LCD_YSIZE - Caption_Height - Bottom_Height,
	                        pMsg->hWin,
	                        WM_CF_SHOW | WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_BOTTOM, 0, GUI_ID_LISTBOX0,
	                        NULL);
	LISTBOX_SetFont(pThisWin->hListbox, _GetFont(Font_ListboxNormal));

	if(g_Language == eLanguage_Chinese)
	{
		LISTBOX_SetItemSpacing(pThisWin->hListbox, LISTBOX_GetItemSpacing(pThisWin->hListbox) + 18);
	}
	else
	{
		LISTBOX_SetItemSpacing(pThisWin->hListbox, LISTBOX_GetItemSpacing(pThisWin->hListbox) + 14);
	}

	LISTBOX_SetAutoScrollV(pThisWin->hListbox, 1);
	LISTBOX_SetTextAlign(pThisWin->hListbox, GUI_TA_LEFT | GUI_TA_VCENTER);
	LISTBOX_SetScrollbarWidth(pThisWin->hListbox, 15);

	for(pThisWin->i = 0; pThisWin->i <  ARR_SIZE(s_ListCallbackTable); pThisWin->i ++ )
	{
		LISTBOX_AddString(pThisWin->hListbox, Get_Language_String(s_ListCallbackTable[pThisWin->i].nStringIndex));
	}
	LISTBOX_SetSel(pThisWin->hListbox, _Convert_Value_To_ListIndex(g_sSystemParam.nAutoSleepTime));

	pThisWin->nLisboxReleasedCnt = 0;
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
					case GUI_KEY_UP:
						pThisWin->nLisboxReleasedCnt++;
						break;
						
					case GUI_KEY_RIGHT_FUN:
						if(LISTBOX_GetSel(pThisWin->hListbox) < LISTBOX_GetNumItems(pThisWin->hListbox) - 1)
						{
							LISTBOX_AddKey(pThisWin->hListbox,GUI_KEY_DOWN);
						}
						else
						{
							LISTBOX_SetSel(pThisWin->hListbox,0);
						}
						pThisWin->nLisboxReleasedCnt++;
						break;
						
					case GUI_KEY_LEFT_FUN:
						WM_DeleteWindow(pMsg->hWin);
						break;

					case GUI_KEY_MIDDLE_FUN:
						LISTBOX_GetItemText(pThisWin->hListbox, LISTBOX_GetSel(pThisWin->hListbox), g_aCommonBuffer, 50);
						for(pThisWin->i = 0; pThisWin->i < ARR_SIZE(s_ListCallbackTable); pThisWin->i++)
						{
							if(strcmp(g_aCommonBuffer, Get_Language_String(s_ListCallbackTable[pThisWin->i].nStringIndex)) == 0 && s_ListCallbackTable[pThisWin->i].pFun)
							{
								(* s_ListCallbackTable[pThisWin->i].pFun)(pMsg->hWin,(void *)1);
							}
							g_sSystemParam.nAutoSleepTime = _Convert_ListIndex_To_Value(LISTBOX_GetSel(pThisWin->hListbox));
							Data_System_Parameter_Save(&g_sSystemParam);
							Data_System_Parameter_Load(&g_sSystemParam);
							WM_DeleteWindow(pMsg->hWin);
						}	
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

				case WM_NOTIFICATION_SEL_CHANGED:
					if(pMsg->hWinSrc == pThisWin->hListbox)
					{
						pThisWin->nLisboxReleasedCnt = 0;
						return;
					}
					break;

				case WM_NOTIFICATION_CLICKED:
					if(pMsg->hWinSrc == pThisWin->hListbox)
					{
						pThisWin->nLisboxReleasedCnt++;
						if(pThisWin->nLisboxReleasedCnt >= 2)
						{
							GUI_StoreKeyMsg(GUI_KEY_MIDDLE_FUN, 0);
							WM_SendMessageNoPara(pMsg->hWin, WM_KEY);
						}
						return;
					}
					break;
			}
			return;

		case WM_DELETE:
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eSettingBacklightTimeout window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_BacklightTimeout *CW_Setting_BackLightTimeout_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSettingBacklightTimeout;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSettingBacklightTimeout window is alreay exist!\r\n");
		return (CW_BacklightTimeout *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_BacklightTimeout));

	if(hWin)
	{
		TRACE_PRINTF("eSettingBacklightTimeout windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_BacklightTimeout *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSettingBacklightTimeout window can't be created!\r\n");
	}

	return pThisWin;
}

