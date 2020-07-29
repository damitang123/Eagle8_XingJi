#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"
#include "stm32f10x.h"

unsigned char g_bChagneLanguage = 0;

static unsigned char s_nLisboxReleasedCnt;

static const char *strLanguageString[] =
{
	"English",
	"Chinese",
	"Russian",
	"Vietnamese",
	"Spanish",
};

static unsigned char Get_System_Language_Index(WM_HWIN hWin)
{
	char  aTemp[25];

	LISTBOX_GetItemText(hWin, LISTBOX_GetSel(hWin), aTemp, sizeof(aTemp) - 1);

	if(strcmp(aTemp, strLanguageString[0]) == 0)
	{
		return 0;
	}

	if(strcmp(aTemp, strLanguageString[1]) == 0)
	{
		return 1;
	}

	if(strcmp(aTemp, strLanguageString[2]) == 0)
	{
		return 2;
	}

	if(strcmp(aTemp, strLanguageString[3]) == 0)
	{
		return 3;
	}

	if(strcmp(aTemp, strLanguageString[4]) == 0)
	{
		return 4;
	}

	return 0;
}

static unsigned char Set_Language_Listbox_Index(WM_HWIN hWin)
{
	char  aTemp[25];
	unsigned char i, nMax;

	nMax = LISTBOX_GetNumItems(hWin);

	for(i = 0; i < nMax ; i++)
	{
		LISTBOX_GetItemText(hWin, i, aTemp, sizeof(aTemp) - 1);
		if(strcmp(aTemp, strLanguageString[g_Language]) == 0)
		{
			return i;
		}
	}

	return 0;
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;

	Create_Dialog_Caption(pMsg);

	/*创建左键*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmCancel_Released, hWin));
	BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmCancel_Pressed, hWin));

	/*创建右键*/
	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmRight_Arrows_Released, hWin));
	BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmRight_Arrows_Pressed, hWin));

	/*创建中键*/
	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);


	/*语言列表*/
	hWin = LISTBOX_CreateEx(0, Caption_Height, LCD_XSIZE, LCD_YSIZE - Caption_Height - Bottom_Height,
	                        pMsg->hWin,
	                        WM_CF_SHOW | WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_BOTTOM, 0, GUI_ID_LISTBOX0,
	                        NULL);
	LISTBOX_SetFont(hWin, _GetFont(Font_ListboxNormal));

	LISTBOX_SetItemSpacing(hWin, LISTBOX_GetItemSpacing(hWin) + 14);
	LISTBOX_SetAutoScrollV(hWin, 1);
	LISTBOX_SetTextAlign(hWin, GUI_TA_LEFT | GUI_TA_VCENTER);

	LISTBOX_AddString(hWin, "English");
	#if !CUST_VIETNAM_DNTIC
	LISTBOX_AddString(hWin, "Chinese");
	#endif
//	LISTBOX_AddString(hWin, "Russian");
	#if CUST_VIETNAM_DNTIC
	LISTBOX_AddString(hWin, "Vietnamese");
	#endif

	/*设置列表框拥有焦点*/
	WM_SetFocus(hWin);

	s_nLisboxReleasedCnt = 0;
	LISTBOX_SetSel(hWin, Set_Language_Listbox_Index(hWin));
}


static void _Callback(WM_MESSAGE *pMsg)
{
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			TRACE_PRINTF("SettingLanguageWindow is created!hwin=0x%04X\r\n", pMsg->hWin);
			break;
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				WM_HWIN hWin;
				hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX0);
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:
						_USER_Close_Window(pMsg->hWin);
						break;
					case GUI_KEY_RIGHT_FUN:
						if(LISTBOX_GetSel(hWin) + 1 == LISTBOX_GetNumItems(hWin))
						{
							LISTBOX_SetSel(hWin, 0);
						}
						else
						{
							LISTBOX_AddKey(hWin, GUI_KEY_DOWN);
						}
						s_nLisboxReleasedCnt++;
						break;
					case GUI_KEY_MIDDLE_FUN :
						g_Language = (eSystemLanguage)Get_System_Language_Index(hWin);

						if(g_sParam.SystemLanguage != g_Language)
						{
							g_sParam.SystemLanguage = g_Language;
							Spi_Flash_Init();
							Data_Parameter_Save(&g_sParam);
							Data_Parameter_Load(&g_sParam);
							Spi_Flash_DeInit();
							OSTimeDly(100);
//							NVIC_SystemReset();
							g_bChagneLanguage = 1;
							Windows_Change_Language();
							g_bChagneLanguage = 0;
						}
						else
						{
							_USER_Close_Window(pMsg->hWin);
						}

						break;
				}
			}
			return;
		case WM_NOTIFY_PARENT:
			if(pMsg->hWinSrc == WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX0))
			{
				if(pMsg->Data.v == WM_NOTIFICATION_SEL_CHANGED)
				{
					s_nLisboxReleasedCnt = 0;
					return;
				}

				if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
				{
					s_nLisboxReleasedCnt++;
					if(s_nLisboxReleasedCnt >= 2)
					{
						GUI_StoreKeyMsg(GUI_KEY_MIDDLE_FUN, 0);
						WM_SendMessageNoPara(pMsg->hWin, WM_KEY);
					}
				}
			}
			break;
		case WM_CHANGE_LANGUAGE:
			_USER_Close_Window(pMsg->hWin);
			CW_SettingLanguage_Create(0x01);
			return;
		case WM_DELETE:
			TRACE_PRINTF("SettingLanguageWindow will be destroyed!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_SettingLanguage *CW_SettingLanguage_Create(unsigned short ID)
{
	CW_SettingLanguage *pCW;
	WM_HWIN hBackWin;

	/*1.检查是否允许重建*/
	if(IS_WinManageExisting(eSettingLanguage)) return (CW_SettingLanguage *)0;

	/*2.申请内存空间*/
	pCW = (CW_SettingLanguage *)UserMalloc(sizeof(CW_SettingLanguage));

	if(pCW == NULL) return (CW_SettingLanguage *)0;

	hBackWin = _USER_Create_Window();
	if(hBackWin == 0) return (CW_SettingLanguage *)0;

	pCW->w_obj.Win_ID = ID;
	pCW->w_obj.hWin = hBackWin;
	pCW->w_obj.hWin_Focus = hBackWin;
	pCW->w_obj.Win_Type = eSettingLanguage;

	/*3.添加到链表中*/
	_AddNewWindowToList((sWinManageList *)pCW);

	/*4.设置新窗体的回调函数.并发送WM_CREATE消息*/
	WM_SetCallback(hBackWin, _Callback);
	{
		const char *p[3];
		WM_MESSAGE Msg;

		p[0] = _Language;

		Msg.MsgId = WM_CREATE;
		Msg.Data.p = &p;
		WM_SendMessage(hBackWin, &Msg);
	}

	return pCW;
}

