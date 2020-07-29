#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"
#include "app.h"

#if USE_GB_T21254_2017

static CW_SelectEnrollUser *pThisWin;

const char s_aAdmin[] = {0xE7,0xAE,0xA1,0xE7,0x90,0x86,0xE5,0x91,0x98,0x00};   /*管理员*/
const char s_aPleaseSelectUser[] = {0xE8,0xAF,0xB7,0xE9,0x80,0x89,0xE6,0x8B,0xA9,0xE7,0x94,0xA8,0xE6,0x88,0xB7,0xE7,0x99,0xBB,0xE5,0xBD,0x95,0x00};   /*请选择用户登录*/

static unsigned char _Add_UserInfor_To_Listbox(void)
{	
	char i,n = 0;
	sPoliceUserInfor s;
	LISTBOX_AddString(pThisWin->hListbox, s_aAdmin);
	Spi_Flash_Init();
	for(i = 0 ; i< FLASH_SECTOR_SIZE / sizeof(sPoliceUserInfor); i++)
	{
		if(Data_Police_User_Infor_Load_ByID(i,&s))
		{
			n++;
			LISTBOX_AddString(pThisWin->hListbox, s.aPoliceName);
			pThisWin->nListIndexLUT[LISTBOX_GetNumItems(pThisWin->hListbox) - 1] = i;
		}
	}
	Spi_Flash_DeInit();
	return n;
}

static bool _Get_UserInfor_By_SelectItem(sPoliceUserInfor *p,unsigned char nListSelIndex)
{	
	unsigned long nAddr;
	
	Spi_Flash_Init();
	nAddr = FLASH_SECTOR_POLICE_USERS_INFOR * FLASH_SECTOR_SIZE + pThisWin->nListIndexLUT[nListSelIndex] * sizeof(sPoliceUserInfor);
	Flash_Read_Array(nAddr, (unsigned char *)p, sizeof(sPoliceUserInfor));
	Spi_Flash_DeInit();

	return true;
}

static void _PassAuthentication(void)
{
	WM_DeleteWindow(pThisWin->wObj.hWin);
	CW_UserMain_Create(GetLastWinFromList(), (void *)1);
}

static void _Create(WM_MESSAGE *pMsg)
{
	pMsg->Data.p = s_aPleaseSelectUser;
	pThisWin->hCaption = Create_Dialog_Caption(pMsg);

	pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hLeftButton, _Previous);

	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hMiddleButton, _OK);

	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hRightButton, _Next);

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

	_Add_UserInfor_To_Listbox();
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
						if(LISTBOX_GetSel(pThisWin->hListbox))
						{
							LISTBOX_AddKey(pThisWin->hListbox,GUI_KEY_UP);
						}
						else
						{
							LISTBOX_SetSel(pThisWin->hListbox,LISTBOX_GetNumItems(pThisWin->hListbox) -1);
						}
						pThisWin->nLisboxReleasedCnt++;
						break;

					case GUI_KEY_MIDDLE_FUN:
						if(LISTBOX_GetNumItems(pThisWin->hListbox))
						{
							char nListSelIndex;
							nListSelIndex = LISTBOX_GetSel(pThisWin->hListbox);
							if(nListSelIndex == 0)
							{
								sInputWindowProperty s;
								s.nWinType = eInputLoginPassword;
								s.aCaptionText = _PleaseInputLoginPassword;
								s.pFunNext = _PassAuthentication;
								s.Data.p = g_sSystemParam.aAdminPassword;
								CW_InputNumberWindow_Create(GetLastWinFromList(),(void *)&s);
								pApp->bLoginAsAdministrator = true;
								memset(&g_sCurrPoliceUserInfor,0,sizeof(g_sCurrPoliceUserInfor));
							}
							else
							{
								sInputWindowProperty s;
								_Get_UserInfor_By_SelectItem(&g_sCurrPoliceUserInfor,nListSelIndex);
								s.nWinType = eInputLoginPassword;
								s.aCaptionText = _PleaseInputLoginPassword;
								s.pFunNext = _PassAuthentication;
								s.Data.p = g_sCurrPoliceUserInfor.aPassword;
								CW_InputNumberWindow_Create(GetLastWinFromList(),(void *)&s);
								pApp->bLoginAsAdministrator = false;
							}
							memcpy(g_sDeviceUserInfo.aPoliceID, g_sCurrPoliceUserInfor.aPoliceID, sizeof(g_sDeviceUserInfo.aPoliceID));
							memcpy(g_sDeviceUserInfo.aPoliceName, g_sCurrPoliceUserInfor.aPoliceName, sizeof(g_sDeviceUserInfo.aPoliceName));
							memcpy(g_sDeviceUserInfo.aDepartment, g_sCurrPoliceUserInfor.aDepartment, sizeof(g_sDeviceUserInfo.aDepartment));
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
			TRACE_PRINTF("eSelectEnrollUser window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}

CW_SelectEnrollUser * CW_SelectEnrollUser_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eSelectEnrollUser;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eSelectEnrollUser window is alreay exist!\r\n");
		return (CW_SelectEnrollUser *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_SelectEnrollUser));

	if(hWin)
	{
		TRACE_PRINTF("eSelectEnrollUser windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_SelectEnrollUser *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eSelectEnrollUser window can't be created!\r\n");
	}

	return pThisWin;
}

#else

CW_SelectEnrollUser * CW_SelectEnrollUser_Create(WM_HWIN hParent, void *pData){ return NULL;}

#endif

