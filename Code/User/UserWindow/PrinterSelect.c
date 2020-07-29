#include "platform.h"
#include "Windows.h"
#include "Parameter.h"
#include "data.h"

static unsigned char s_nLisboxReleasedCnt;

static const char *strPrinter[] =
{
	"Integration",
	"KY-9000",
	"KY-9001",
};

static unsigned char Get_System_Printer_Index(WM_HWIN hWin)
{
	char  aTemp[25];

	LISTBOX_GetItemText(hWin, LISTBOX_GetSel(hWin), aTemp, sizeof(aTemp) - 1);

	if(strstr(aTemp, strPrinter[0]))
	{
		return 0;
	}

	if(strstr(aTemp, strPrinter[1]))
	{
		return 1;
	}

	if(strstr(aTemp, strPrinter[2]))
	{
		return 2;
	}

	return 0;
}

static unsigned char Set_Printer_Listbox_Index(WM_HWIN hWin)
{
	char  aTemp[25];
	unsigned char i, nMax;

	nMax = LISTBOX_GetNumItems(hWin);

	for(i = 0; i < nMax ; i++)
	{
		LISTBOX_GetItemText(hWin, i, aTemp, sizeof(aTemp) - 1);
		if(strstr(aTemp, strPrinter[g_sParam.PrinterType]))
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

	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, __Exit);

	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);

	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmRight_Arrows_Released, hWin));
	BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmRight_Arrows_Pressed, hWin));


	hWin = LISTBOX_CreateEx(0, Caption_Height, LCD_XSIZE, LCD_YSIZE - Caption_Height - Bottom_Height,
	                        pMsg->hWin,
	                        WM_CF_SHOW | WM_SF_ANCHOR_LEFT | WM_SF_ANCHOR_TOP | WM_SF_ANCHOR_RIGHT | WM_SF_ANCHOR_BOTTOM, 0, GUI_ID_LISTBOX0,
	                        NULL);
	LISTBOX_SetFont(hWin, _GetFont(Font_ListboxNormal));

	LISTBOX_SetItemSpacing(hWin, LISTBOX_GetItemSpacing(hWin) + 14);
	LISTBOX_SetAutoScrollV(hWin, 1);
	LISTBOX_SetTextAlign(hWin, GUI_TA_LEFT | GUI_TA_VCENTER);
	LISTBOX_SetScrollbarWidth(hWin, 15);

	LISTBOX_AddString(hWin, "0:Integration");
	LISTBOX_AddString(hWin, "1:KY-9000");
	LISTBOX_AddString(hWin, "2:KY-9001");
	if(g_sParam.PrinterType <= 2)
	{
		LISTBOX_SetSel(hWin, g_sParam.PrinterType);
	}
	else
	{
		LISTBOX_SetSel(hWin, 0);
		g_sParam.PrinterType = 0;
	}

	/*设置列表框拥有焦点*/
	s_nLisboxReleasedCnt = 0;
	WM_SetFocus(hWin);

	LISTBOX_SetSel(hWin, Set_Printer_Listbox_Index(hWin));
}


static void _Callback(WM_MESSAGE *pMsg)
{
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			TRACE_PRINTF("PirnterSelectWindow is created!hwin=0x%04X\r\n", pMsg->hWin);
			break;
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				WM_HWIN hWin;
				hWin = WM_GetDialogItem(pMsg->hWin, GUI_ID_LISTBOX0);
				switch (WM_Key_Code)
				{
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
					case GUI_KEY_LEFT_FUN:
						_USER_Close_Window(pMsg->hWin);
						break;
					case GUI_KEY_MIDDLE_FUN:
						if(g_sParam.PrinterType != Get_System_Printer_Index(hWin))
						{
							g_sParam.PrinterType = Get_System_Printer_Index(hWin);
							Spi_Flash_Init();
							Data_Parameter_Save(&g_sParam);
							Data_Parameter_Load(&g_sParam);
							Spi_Flash_DeInit();
						}
						_USER_Close_Window(pMsg->hWin);
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
			return;
		case WM_DELETE:
			TRACE_PRINTF("PirnterSelectWindow will be destroyed!\r\n");
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_PrinterSelectWindow *CW_PrinterSelectWindow_Create(unsigned short ID)
{
	CW_PrinterSelectWindow *pCW;
	WM_HWIN hBackWin;

	/*1.检查是否允许重建*/
	if(IS_WinManageExisting(ePrinterSelectWin)) return (CW_PrinterSelectWindow *)0;

	/*2.申请内存空间*/
	pCW = (CW_PrinterSelectWindow *)UserMalloc(sizeof(CW_PrinterSelectWindow));

	if(pCW == NULL) return (CW_PrinterSelectWindow *)0;

	hBackWin = _USER_Create_Window();
	if(hBackWin == 0) return (CW_PrinterSelectWindow *)0;

	pCW->w_obj.Win_ID = ID;
	pCW->w_obj.hWin = hBackWin;
	pCW->w_obj.hWin_Focus = hBackWin;
	pCW->w_obj.Win_Type = ePrinterSelectWin;

	/*3.添加到链表中*/
	_AddNewWindowToList((sWinManageList *)pCW);

	/*4.设置新窗体的回调函数.并发送WM_CREATE消息*/
	WM_SetCallback(hBackWin, _Callback);
	{
		const char *p[3];
		WM_MESSAGE Msg;

		p[0] = _PleaseSelectPrinter;

		Msg.MsgId = WM_CREATE;
		Msg.Data.p = &p;
		WM_SendMessage(hBackWin, &Msg);
	}

	return pCW;
}

