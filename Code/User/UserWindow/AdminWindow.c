#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"
#include "Test.h"

static CW_Administrator *pThisWin;

const static sListProcessTable s_ListCallbackTable[]= 
{
#if USE_GB_T21254_2017
	eCalibration ,				(pFunUser)CW_Calibration_Create,
	#if (SERVER_USE_XING_JIKONG_GU)//added by lxl 20180927
	eAlcoholMaintainTest,		(pFunUser)CW_TestMain_Create,
	#endif
	eUserManage,				(pFunUser)CW_UserManage_Create,
	eAlcUnit,					(pFunUser)CW_SetALCUnit_Create,
	eBlowParameter,			(pFunUser)CW_SetBlow_Create,
	eSampleModeSelect,		(pFunUser)CW_Sample_Mode_Config_Create,
	eTimeZoneSetting,			(pFunUser)CW_TimeZoneWindow_Create,
	eAPN,					NULL,
	eServer,					(pFunUser)CW_ShowDialog_Create,
	eDeviceDebug,			(pFunUser)CW_Debug_Window_Create,
#else
	eCalibration ,				(pFunUser)CW_Calibration_Create,
	eAlcUnit,					(pFunUser)CW_SetALCUnit_Create,
	eBlowParameter,			(pFunUser)CW_SetBlow_Create,
	eSampleModeSelect,		(pFunUser)CW_Sample_Mode_Config_Create,
	eTimeZoneSetting,			(pFunUser)CW_TimeZoneWindow_Create,
	eAPN,					NULL,
	eServer,					(pFunUser)CW_ShowDialog_Create,
	eDeviceDebug,			(pFunUser)CW_Debug_Window_Create,
#endif
};

static void _Create(WM_MESSAGE *pMsg)
{
#if USE_GB_T21254_2017
	pMsg->Data.p = _MaintainMode;
#else
	pMsg->Data.p = _Admin;
#endif
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
	LISTBOX_SetSel(pThisWin->hListbox,0);

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
						
					case GUI_KEY_ENTER:
						LISTBOX_GetItemText(pThisWin->hListbox, LISTBOX_GetSel(pThisWin->hListbox), g_aCommonBuffer, 50);
						for(pThisWin->i = 0; pThisWin->i < ARR_SIZE(s_ListCallbackTable); pThisWin->i++)
						{
							if(strcmp(g_aCommonBuffer, Get_Language_String(s_ListCallbackTable[pThisWin->i].nStringIndex)) == 0 && s_ListCallbackTable[pThisWin->i].pFun)
							{	
								if(strcmp(g_aCommonBuffer, Get_Language_String(eServer)) == 0)
								{
									char aTemp[100];
									sDialogWindowProperty s;
									sprintf(aTemp,"Server Config:\n\n%s:%s\n\nPort:%d\n",
										g_sServerParam.nNetAccessMode ? "URL" : "IP",
										g_sServerParam.nNetAccessMode ?	g_sServerParam.aRemoteURL : g_sServerParam.aRemoteIP,
										g_sServerParam.nRemotePort);
									s.nWinType = eDialogOK;
									s.pFont = _GetFont(Font_Content);
									s.pContent = aTemp;
									s.nContentColor = GUI_DARKBLUE;
									s.nBackGroundColor = GUI_LIGHTGRAY;
									s.nAutoCloseTime = 10;
									s.pFunMiddleOK = NULL;
									CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
								}
								else if(strcmp(g_aCommonBuffer, Get_Language_String(eAlcoholMaintainTest)) == 0)
								{
									g_eAlcTestPurpose = eTestForMaintain;
									CW_TestMain_Create(pMsg->hWin,(void *)eTestActive);
								}
								else
								{
									(* s_ListCallbackTable[pThisWin->i].pFun)(pMsg->hWin,(void *)1);
								}
							}
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
			TRACE_PRINTF("eAdministor window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}

CW_Administrator *CW_Administrator_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eAdministor;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eAdministor window is alreay exist!\r\n");
		return (CW_Administrator *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_Administrator));

	if(hWin)
	{
		TRACE_PRINTF("eAdministor windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_Administrator *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eAdministor window can't be created!\r\n");
	}

	return pThisWin;
}

