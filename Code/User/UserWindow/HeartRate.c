#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"

static CW_HeartRateWindow *pThisWin;

sHeartRateParam g_sHeartRateParam;
static const char _HeartRate[]={0xE5,0xBF,0x83,0xE8,0xB7,0xB3,0xE9,0xA2,0x91,0xE7,0x8E,0x87,0x28,0x53,0x29,0x00};   /*ÐÄÌøÆµÂÊ(S)*/

static unsigned long _Convert_ListIndex_To_Value(unsigned char nIndex)
{
	switch(nIndex)
	{
		case 0:
			return 10;
		case 1:
			return 20;
		case 2:
			return 30;
		case 3:
			return 40;
		case 4:
			return 60;
		default:
			return 10;
	}
}

static unsigned char _Convert_Value_To_ListIndex(unsigned long nValue)
{
	switch(nValue)
	{
		case 10:
			return 0;
		case 20:
			return 1;
		case 30:
			return 2;
		case 40:
			return 3;
		case 60:
			return 4;
		default:
			return 0;
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	pMsg->Data.p =_HeartRate;// _HeartRateModeSelect;
	Create_Dialog_Caption(pMsg);

	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);

	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmRight_Arrows_Released, hWin));
	BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmRight_Arrows_Pressed, hWin));


	pThisWin->hListbox = LISTBOX_CreateEx(0, Caption_Height, LCD_XSIZE, LCD_YSIZE - Caption_Height - Bottom_Height,
	                        pMsg->hWin,
	                        WM_CF_SHOW | WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_BOTTOM, 0, GUI_ID_LISTBOX0,
	                        NULL);
	LISTBOX_SetFont(pThisWin->hListbox, _GetFont(Font_ListboxNormal));

	LISTBOX_SetItemSpacing(pThisWin->hListbox, LISTBOX_GetItemSpacing(pThisWin->hListbox) + 14);
	LISTBOX_SetAutoScrollV(pThisWin->hListbox, 1);
	LISTBOX_SetTextAlign(pThisWin->hListbox, GUI_TA_LEFT | GUI_TA_VCENTER);
	LISTBOX_SetScrollbarWidth(pThisWin->hListbox, 15);
	
	LISTBOX_AddString(pThisWin->hListbox, "10");
	LISTBOX_AddString(pThisWin->hListbox, "20");
	LISTBOX_AddString(pThisWin->hListbox, "30");
	LISTBOX_AddString(pThisWin->hListbox, "40");
	LISTBOX_AddString(pThisWin->hListbox, "60");

	LISTBOX_SetSel(pThisWin->hListbox, _Convert_Value_To_ListIndex(g_sHeartRateParam.byHeartRate));

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
						g_sHeartRateParam.byHeartRate = _Convert_ListIndex_To_Value(LISTBOX_GetSel(pThisWin->hListbox));
						Spi_Flash_Init();
						Data_HeartRate_Parameter_Save(&g_sHeartRateParam);
						Data_HeartRate_Parameter_Load(&g_sHeartRateParam);
						Spi_Flash_DeInit();
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
			TRACE_PRINTF("ePrintCopys window is deleted!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_HeartRateWindow *CW_HeartRateWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eHeartRateMode;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("ePrintCopys window is alreay exist!\r\n");
		return (CW_HeartRateWindow *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_HeartRateWindow));

	if(hWin)
	{
		TRACE_PRINTF("ePrintCopys windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_HeartRateWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("ePrintCopys window can't be created!\r\n");
	}

	return pThisWin;
}


