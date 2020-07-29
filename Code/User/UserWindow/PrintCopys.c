#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"

static CW_PrintCopysWindow *pThisWin;

static unsigned long _Convert_ListIndex_To_Value(unsigned char nIndex)
{
	switch(nIndex)
	{
		case 0:
			return 1;
		case 1:
			return 2;
		case 2:
			return 3;
		case 3:
			return 4;
		case 4:
			return 5;
		default:
			return 1;
	}
}

static unsigned char _Convert_Value_To_ListIndex(unsigned long nValue)
{
	switch(nValue)
	{
		case 1:
			return 0;
		case 2:
			return 1;
		case 3:
			return 2;
		case 4:
			return 3;
		case 5:
			return 4;
		default:
			return 0;
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;
	pMsg->Data.p = _PrinterCopys;
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
	
	LISTBOX_AddString(pThisWin->hListbox, "1");
	LISTBOX_AddString(pThisWin->hListbox, "2");
	LISTBOX_AddString(pThisWin->hListbox, "3");
	LISTBOX_AddString(pThisWin->hListbox, "4");
	LISTBOX_AddString(pThisWin->hListbox, "5");

	LISTBOX_SetSel(pThisWin->hListbox, _Convert_Value_To_ListIndex(g_sPrinterParam.nPrintCopys));

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
						g_sPrinterParam.nPrintCopys = _Convert_ListIndex_To_Value(LISTBOX_GetSel(pThisWin->hListbox));
						Spi_Flash_Init();
						Data_Printer_Parameter_Save(&g_sPrinterParam);
						Data_Printer_Parameter_Save(&g_sPrinterParam);
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


CW_PrintCopysWindow *CW_PrintCopysWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = ePrintCopys;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("ePrintCopys window is alreay exist!\r\n");
		return (CW_PrintCopysWindow *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_PrintCopysWindow));

	if(hWin)
	{
		TRACE_PRINTF("ePrintCopys windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_PrintCopysWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

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

