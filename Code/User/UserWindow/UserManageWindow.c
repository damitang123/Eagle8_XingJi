#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"
#include "system_misc.h"

#if USE_GB_T21254_2017

#include "app.h"

static CW_UserManage *pThisWin;

const char s_aAddUser[] = {0xE6,0x96,0xB0,0xE5,0xA2,0x9E,0x00};   		/*新增*/;
const char s_aDeleteUser[] = {0xE5,0x88,0xA0,0xE9,0x99,0xA4,0x00};   	/*删除*/
const char s_aEditUser[] = {0xE7,0xBC,0x96,0xE8,0xBE,0x91,0x00};   		/*编辑*/

WM_HWIN _CreateAddUserButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(2,  LCD_YSIZE - Bottom_Height - Bottom_Height - 5, LEFT_BUTTON_WIDTH, Bottom_Height - 4,
						    pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE, 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Left_Button));
	Config_Button_Property(hWin);
	return hWin;
}

WM_HWIN _CreateDeleteUserButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx((LCD_XSIZE - MIDDLE_BUTTON_WIDTH) >> 1,  LCD_YSIZE - Bottom_Height - Bottom_Height - 5, MIDDLE_BUTTON_WIDTH, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Right_Button));
	Config_Button_Property(hWin);
	return hWin;
}


WM_HWIN _CreateEditUserButton(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	hWin = BUTTON_CreateEx(LCD_XSIZE - RIGHT_BUTTON_WIDTH - 2,  LCD_YSIZE - Bottom_Height - Bottom_Height - 5, RIGHT_BUTTON_WIDTH, Bottom_Height - 4,
						   pMsg->hWin,
						   WM_CF_SHOW | WM_CF_HASTRANS, BUTTON_CF_CIRCLE_ANGLE , 0);
	BUTTON_SetFont(hWin, _GetFont(Font_Right_Button));
	Config_Button_Property(hWin);
	return hWin;
}

static unsigned char _Add_UserInfor_To_Listbox(void)
{	
	char i,n = 0;
	sPoliceUserInfor s;
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

static bool _Delete_UserInfor_By_SelectItem(unsigned char nListSelIndex)
{	
	int i,t = 0;
	sPoliceUserInfor s;
	unsigned long nAddr, nAddrTemp;

	Spi_Flash_Init();
	memset(&s,0,sizeof(s));
	nAddr = FLASH_SECTOR_POLICE_USERS_INFOR * FLASH_SECTOR_SIZE + pThisWin->nListIndexLUT[nListSelIndex] * sizeof(sPoliceUserInfor);
	Flash_Write_Array(nAddr, (unsigned char *)&s, sizeof(s));

	nAddrTemp = FLASH_SECTOR_TEMP_SWAP * FLASH_SECTOR_SIZE;
	Flash_Sector_Erase(nAddrTemp);

	/*转移数据到临时分区*/
	for(i = 0 ; i < FLASH_SECTOR_SIZE / sizeof(sPoliceUserInfor); i++)
	{
		if(Data_Police_User_Infor_Load_ByID(i,&s))
		{
			Flash_Write_Array(nAddrTemp + (t++ *  sizeof(sPoliceUserInfor)), (unsigned char *)&s, sizeof(sPoliceUserInfor));
		}
	}
	nAddr = FLASH_SECTOR_POLICE_USERS_INFOR * FLASH_SECTOR_SIZE;
	Flash_Sector_Erase(nAddr);

	Flash_Content_Copy(nAddrTemp, nAddr, t * sizeof(sPoliceUserInfor));
	
	Spi_Flash_DeInit();

	return true;
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

static void _Update_Listbox_Content(void)
{
	int i,n;
	
	n = LISTBOX_GetNumItems(pThisWin->hListbox);
	for(i = 0; i < n; i++)
	{
		LISTBOX_DeleteItem(pThisWin->hListbox,0);
	}

	_Add_UserInfor_To_Listbox();

	LISTBOX_SetSel(pThisWin->hListbox,0);
	pThisWin->nLisboxReleasedCnt = 0;
}

static void _ConfirmNewRegisterYes(void)
{
	sInputWindowProperty s;
	s.nWinType = eInputLoginName;
	s.aCaptionText = _PleaseInputName;
	CW_InputWindow_Create(GetLastWinFromList(),(void *)&s);
}

static void _ConfirmNewRegisterNo(void)
{
	WM_DeleteWindow(pThisWin->wObj.hWin);
}

static void _ConfirmDeleteUserYes(void)
{
	_Delete_UserInfor_By_SelectItem(LISTBOX_GetSel(pThisWin->hListbox));
	_Update_Listbox_Content();
}

static void _Create(WM_MESSAGE *pMsg)
{
	pMsg->Data.p = _UserManage;
	pThisWin->hCaption = Create_Dialog_Caption(pMsg);

	pThisWin->hLeftButton = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hLeftButton, __Exit);

	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hMiddleButton, _OK);

	pThisWin->hRightButton = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 0, BtnSetBmp(bmRight_Arrows_Released, pThisWin->hRightButton));
	BUTTON_SetBitmapEx(pThisWin->hRightButton, 1, BtnSetBmp(bmRight_Arrows_Pressed, pThisWin->hRightButton));

	pThisWin->hAddUserButton = _CreateAddUserButton(pMsg);
	BUTTON_SetText(pThisWin->hAddUserButton, s_aAddUser);
	
	pThisWin->hDeleteUserButton = _CreateDeleteUserButton(pMsg);
	BUTTON_SetText(pThisWin->hDeleteUserButton, s_aDeleteUser);
	
	pThisWin->hListbox = LISTBOX_CreateEx(0, Caption_Height, LCD_XSIZE, LCD_YSIZE - Caption_Height - Bottom_Height - Bottom_Height -10,
	                        pMsg->hWin,
	                        WM_CF_SHOW | WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_BOTTOM, 0, GUI_ID_LISTBOX0,
	                        NULL);
	LISTBOX_SetFont(pThisWin->hListbox, _GetFont(Font_ListboxNormal));
	LISTBOX_SetItemSpacing(pThisWin->hListbox, LISTBOX_GetItemSpacing(pThisWin->hListbox) + 18);
	LISTBOX_SetAutoScrollV(pThisWin->hListbox, 1);
	LISTBOX_SetTextAlign(pThisWin->hListbox, GUI_TA_LEFT | GUI_TA_VCENTER);
	LISTBOX_SetScrollbarWidth(pThisWin->hListbox, 15);

	if(_Add_UserInfor_To_Listbox() == 0)
	{
		sDialogWindowProperty s;
		s.nWinType = eDialogYesNo;
		s.pFont = _GetFont(Font_Content);
		s.pContent = _AlertRegisterUserInfor;
		s.nContentColor = GUI_RED;
		s.nBackGroundColor = GUI_LIGHTGRAY;
		s.nAutoCloseTime = 10;
		s.pFunLeftYes = _ConfirmNewRegisterYes;
		s.pFunRightNo = _ConfirmNewRegisterNo;
		CW_ShowDialog_Create(pMsg->hWin,(void *)&s);
		return;
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
						if(LISTBOX_GetNumItems(pThisWin->hListbox))
						{
							char aTemp[300];
							sPoliceUserInfor s;
							_Get_UserInfor_By_SelectItem(&s,LISTBOX_GetSel(pThisWin->hListbox));
							sprintf(aTemp,"%s:\r\n%s%s\r\n%s%s\r\n%s%s",
									_UserBasicInfor,
									_R_PoliceID, s.aPoliceID,
									_R_PoliceName, s.aPoliceName,
									_R_Department, s.aDepartment);
							{
								sDialogWindowProperty s;
								s.nWinType = eDialogOK;
								s.pFont = &GUI_Font16_UTF8;
								s.pContent = aTemp;
								s.nContentColor = GUI_RED;
								s.nBackGroundColor = GUI_LIGHTGRAY;
								s.nAutoCloseTime = 10;
								s.pFunMiddleOK = NULL;
								CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
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

				case WM_NOTIFICATION_RELEASED:
					if(pMsg->hWinSrc == pThisWin->hAddUserButton)
					{
						sInputWindowProperty s;
						s.nWinType = eInputLoginName;
						s.aCaptionText = _PleaseInputName;
						CW_InputWindow_Create(GetLastWinFromList(),(void *)&s);
						
					}
					else if(pMsg->hWinSrc == pThisWin->hDeleteUserButton && LISTBOX_GetNumItems(pThisWin->hListbox))
					{	
						sDialogWindowProperty s;
						s.nWinType = eDialogYesNo;
						s.pFont = _GetFont(Font_Content);
						s.pContent = _ConfirmDeleteAllUserInfor;
						s.nContentColor = GUI_RED;
						s.nBackGroundColor = GUI_LIGHTGRAY;
						s.nAutoCloseTime = 10;
						s.pFunLeftYes = _ConfirmDeleteUserYes;
						s.pFunRightNo = NULL;
						CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
					}
					break;
			}
			return;

		case WM_DELETE:
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eWinUserManage window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_UserManage  *CW_UserManage_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eWinUserManage;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eWinUserManage window is alreay exist!\r\n");
		return (CW_UserManage *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_UserManage));

	if(hWin)
	{
		TRACE_PRINTF("eWinUserManage windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_UserManage *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eWinUserManage window can't be created!\r\n");
	}

	g_sXingJiKongGU.bAdministration=true;  //moved by lxl 20181108
	
	return pThisWin;
}

#else

CW_UserManage  *CW_UserManage_Create(WM_HWIN hParent, void *pData){return NULL;}

#endif

