#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"
#include "app.h"

static CW_SettingMain *pThisWin;

const static sListProcessTable s_ListCallbackTable[]= 
{
	eBackLight ,				(pFunUser)CW_Setting_BackLight_Create,
	eAutoOff ,				(pFunUser)CW_AutoShutdownTimeConfig_Create,
	eTimeDateSet ,			(pFunUser)CW_SetTime_Create,
	ePrinterCopys ,			(pFunUser)CW_PrintCopysWindow_Create,
	#if SERVER_USE_XING_JIKONG_GU
	eHeartRateSelect ,					(pFunUser)CW_HeartRateWindow_Create,
	#endif
	eTouchScreenCalibrate ,	(pFunUser)CW_ShowDialog_Create,
	eRestoreSet ,				(pFunUser)CW_ShowDialog_Create,
	eSystemInformation ,		(pFunUser)CW_Setting_SystemInfo_Create,
#if USE_GB_T21254_2017
	eMaintainMode ,			(pFunUser)CW_InputNumberWindow_Create,
#else
	eAdmin ,					(pFunUser)CW_InputNumberWindow_Create,
#endif
};

static void _ConfirmRestorParameter(void)
{
	sSystemParam s;
	s = g_sSystemParam;
	Flash_Sector_Erase(FLASH_SECTOR_TEST_PARAM * FLASH_SECTOR_SIZE);
	Data_Test_Parameter_Load(&g_sTestParam);
	Flash_Sector_Erase(FLASH_SECTOR_SYSTEM_PARAM * FLASH_SECTOR_SIZE);
	Data_System_Parameter_Load(&g_sSystemParam);
	memcpy(g_sSystemParam.aIMEI, s.aIMEI, sizeof(s.aIMEI));
	Data_System_Parameter_Save(&g_sSystemParam);
	Flash_Sector_Erase(FLASH_SECTOR_PRINTER_PARAM * FLASH_SECTOR_SIZE);
	Data_Printer_Parameter_Load(&g_sPrinterParam);

	#if SERVER_USE_XING_JIKONG_GU
	Data_HeartRate_Parameter_Load(&g_sHeartRateParam);
	#endif
}

static void _ConfirmDoTouchCalibrate(void)
{
	CW_TouchCalibrate_Create(pThisWin->wObj.hWin, (void *)1);
}

static void _PassAdminAuthentication(void)
{
	CW_Administrator_Create(pThisWin->wObj.hWin, (void *)1);
}

static void _Create(WM_MESSAGE *pMsg)
{
	pMsg->Data.p = _Setting;
	pThisWin->hCaption = Create_Dialog_Caption(pMsg);

	pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hLeftButton, __Exit);

	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hMiddleButton, _OK);

#if USE_TEXT_REPLACE_ICON
	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hRightButton, _Next);
#else
	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 0, BtnSetBmp(bmRight_Arrows_Released, pThisWin->hRightButton));
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 1, BtnSetBmp(bmRight_Arrows_Pressed, pThisWin->hRightButton));
#endif

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

#if USE_GB_T21254_2017
	for(pThisWin->i = 0; pThisWin->i <  ARR_SIZE(s_ListCallbackTable); pThisWin->i ++ )
	{
		if(s_ListCallbackTable[pThisWin->i].nStringIndex != eMaintainMode || pApp->bLoginAsAdministrator)
		{	
			LISTBOX_AddString(pThisWin->hListbox, Get_Language_String(s_ListCallbackTable[pThisWin->i].nStringIndex));
		}
	}
#else
	for(pThisWin->i = 0; pThisWin->i <  ARR_SIZE(s_ListCallbackTable); pThisWin->i ++ )
	{    
		LISTBOX_AddString(pThisWin->hListbox, Get_Language_String(s_ListCallbackTable[pThisWin->i].nStringIndex));
	}
#endif
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
								if(strcmp(g_aCommonBuffer, Get_Language_String(eTouchScreenCalibrate)) == 0)
								{
									sDialogWindowProperty s;
									s.nWinType = eDialogYesNo;
									s.pFont = _GetFont(Font_Content);
									s.pContent = _EnsureNeedTouchCalibrate;
									s.nContentColor = GUI_RED;
									s.nBackGroundColor = GUI_LIGHTGRAY;
									s.nAutoCloseTime = 10;
									s.pFunLeftYes = _ConfirmDoTouchCalibrate;
									s.pFunRightNo = NULL;
									CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
								}
								else if(strcmp(g_aCommonBuffer, Get_Language_String(eRestoreSet)) == 0)
								{
									sDialogWindowProperty s;
									s.nWinType = eDialogYesNo;
									s.pFont = _GetFont(Font_Content);
									s.pContent = _RestoreSetTip;
									s.nContentColor = GUI_RED;
									s.nBackGroundColor = GUI_LIGHTGRAY;
									s.nAutoCloseTime = 10;
									s.pFunLeftYes = _ConfirmRestorParameter;
									s.pFunRightNo = NULL;
									CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
								}
							#if USE_GB_T21254_2017
								else if(strcmp(g_aCommonBuffer, Get_Language_String(eMaintainMode)) == 0)
								{
									if(pApp->bLoginAsAdministrator)
									{
										CW_Administrator_Create(pThisWin->wObj.hWin, (void *)1);
									}
									else
									{
										sInputWindowProperty s;
										s.nWinType = eInputAdminPassword;
										s.aCaptionText = _PleaseInputPassword;
										s.pFunNext = _PassAdminAuthentication;
										s.Data.p = g_sSystemParam.aAdminPassword;
										CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
									}
								}
							#else
								else if(strcmp(g_aCommonBuffer, Get_Language_String(eAdmin)) == 0)
								{
									sInputWindowProperty s;
									s.nWinType = eInputAdminPassword;
									s.aCaptionText = _PleaseInputPassword;
									s.pFunNext = _PassAdminAuthentication;
									s.Data.p = g_sSystemParam.aAdminPassword;
									CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
								}
							#endif
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
			TRACE_PRINTF("eSettingMain window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_SettingMain *CW_SettingMain_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSettingMain;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSettingMain window is alreay exist!\r\n");
		return (CW_SettingMain *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_SettingMain));

	if(hWin)
	{
		TRACE_PRINTF("eSettingMain windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_SettingMain *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSettingMain window can't be created!\r\n");
	}

	return pThisWin;
}

