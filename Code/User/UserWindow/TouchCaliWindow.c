#include "platform.h"
#include "Windows.h"
#include "GUI.h"
#include "LCDConf.h"
#include "Buzzer.h"
#include "delay.h"
#include "GUITouchConf.h"
#include "system_misc.h"

#define TOUCH_MAX_OFFSET		300

static CW_TouchCalibrate *pThisWin;

static void Process_Touch_Cali_Data(void)
{
	unsigned short x0, y0; //左上
	unsigned short x1, y1; //左下
	unsigned short x2, y2; //右上
	unsigned short x3, y3; //右下

	unsigned short xAdc, yAdc;
	unsigned short xMin, xMax, yMin, yMax;

	x0 = pThisWin->x0;
	y0 = pThisWin->y0;
	x1 = pThisWin->x1;
	y1 = pThisWin->y1;
	x2 = pThisWin->x2;
	y2 = pThisWin->y2;
	x3 = pThisWin->x3;
	y3 = pThisWin->y3;
	
#if	!GUI_TOUCH_SWAP_XY
	xAdc = abs(x3 - x1 + x2 - x0) / 2;        			//触摸点的间距
	yAdc = abs(y3 - y2 + y1 - y0) / 2;

	xAdc = (xAdc * 10) / (LCD_XSIZE - 20);          	//10个点的ADC差值
	yAdc = (yAdc * 10) / (LCD_YSIZE - 20);          	//10个点的ADC差值

	xMin = abs((x2 + x3) / 2 - xAdc);          			//计算边界
	xMax = (x0 + x1) / 2 + xAdc;
	yMin = abs((y1 + y3) / 2 - yAdc);
	yMax = (y0 + y2) / 2 + yAdc;
#else
	xAdc = (x1 - x3 + x0 - x2) / 2;
	yAdc = (y3 - y2 + y1 - y0) / 2;

	xAdc = (xAdc * 10) / (LCD_XSIZE - 20);           //10个点的ADC差值
	yAdc = (yAdc * 10) / (LCD_YSIZE - 20);		//10个点的ADC差值

	xMin = (x2 + x3) / 2 - xAdc;          			//计算边界
	xMax = (x0 + x1) / 2 + xAdc;
	yMin = (y0 + y2) / 2 - yAdc;
	yMax = (y1 + y3) / 2 + yAdc;
#endif

	GUI_TOUCH_Calibrate(0, 0, 240, xMax, xMin);
	GUI_TOUCH_Calibrate(1, 0, 320, yMin,  yMax);

	Spi_Flash_Init();
	Data_TouchCalibration_Save();
	Spi_Flash_DeInit();
}

static bool GetTouchAdcValue(u16 *x, u16 *y)
{
	int xAdc , yAdc;

	/*等待按下*/
	if((GUI_TOUCH_X_MeasureX() == -1 && GUI_TOUCH_X_MeasureY() == -1))
	{
		return false;
	}

	delay_ms(150);
	
#if GUI_TOUCH_SWAP_XY
	xAdc = GUI_TOUCH_X_MeasureY();
	yAdc = GUI_TOUCH_X_MeasureX();
#else
	xAdc = GUI_TOUCH_X_MeasureX();
	yAdc = GUI_TOUCH_X_MeasureY();
#endif
	
	/*所按下坐标容错性检查*/
	if(pThisWin->nCaliIndex == 0)
	{
		if(abs(xAdc - GUI_TOUCH_AD_LEFT) > TOUCH_MAX_OFFSET || abs(yAdc - GUI_TOUCH_AD_TOP) > TOUCH_MAX_OFFSET)
		{
			return false;
		}
	}
	else if(pThisWin->nCaliIndex == 1)
	{
		if(abs(xAdc - GUI_TOUCH_AD_LEFT) > TOUCH_MAX_OFFSET || abs(yAdc - GUI_TOUCH_AD_BOTTOM) > TOUCH_MAX_OFFSET)
		{
			return false;
		}
	}
	else if(pThisWin->nCaliIndex == 2)
	{
		if(abs(xAdc - GUI_TOUCH_AD_RIGHT) > TOUCH_MAX_OFFSET || abs(yAdc - GUI_TOUCH_AD_TOP) > TOUCH_MAX_OFFSET)
		{
			return false;
		}
	}
	else if(pThisWin->nCaliIndex == 3)
	{
		if(abs(xAdc - GUI_TOUCH_AD_RIGHT) > TOUCH_MAX_OFFSET || abs(yAdc - GUI_TOUCH_AD_BOTTOM) > TOUCH_MAX_OFFSET)
		{
			return false;
		}
	}
	
	Buzzer_Beep(eBUZ_PRESSOK);
	
	/*等待释放*/
	while((GUI_TOUCH_X_MeasureX() != -1) && (GUI_TOUCH_X_MeasureY() != -1));
	
	delay_ms(100);
	
	*x = xAdc;
	*y = yAdc;

	return true;
}

static void Touch_Calibrate_Scan(WM_MESSAGE *pMsg)
{
	char aTemp[10];
	signed char nRemin;
	
	nRemin = 30 - ((Get_System_Time() - pThisWin->nStartTime) / 1000);
	sprintf(aTemp, "%02d", nRemin);
	TEXT_SetText(pThisWin->hTextTimer, aTemp);
	
	if(nRemin < 0)
	{
		WM_DeleteWindow(pMsg->hWin);
		return;
	}
	
	switch(pThisWin->nCaliIndex)
	{
		case 0:
			if(pThisWin->bNeedRefeshWindow)
			{
				pThisWin->bNeedRefeshWindow = false;
				WM_InvalidateWindow(pMsg->hWin);
			}

			if(GetTouchAdcValue(&pThisWin->x0, &pThisWin->y0))
			{
				pThisWin->nCaliIndex = 1;
				pThisWin->bNeedRefeshWindow = true;
			}
			break;
			
		case 1:
			if(pThisWin->bNeedRefeshWindow)
			{
				pThisWin->bNeedRefeshWindow = false;
				WM_InvalidateWindow(pMsg->hWin);
			}

			if(GetTouchAdcValue(&pThisWin->x1, &pThisWin->y1))
			{
				pThisWin->nCaliIndex = 3;
				pThisWin->bNeedRefeshWindow = true;
			}
			break;
			
		case 2:
			if(pThisWin->bNeedRefeshWindow)
			{
				pThisWin->bNeedRefeshWindow = false;
				WM_InvalidateWindow(pMsg->hWin);
			}

			if(GetTouchAdcValue(&pThisWin->x2, &pThisWin->y2))
			{
				/*运行到此表明校准完成*/
				Process_Touch_Cali_Data();
				WM_DeleteWindow(pMsg->hWin);
			}
			break;
			
		case 3:
			if(pThisWin->bNeedRefeshWindow)
			{
				pThisWin->bNeedRefeshWindow = false;
				WM_InvalidateWindow(pMsg->hWin);
			}

			if(GetTouchAdcValue(&pThisWin->x3, &pThisWin->y3))
			{
				pThisWin->nCaliIndex = 2;
				pThisWin->bNeedRefeshWindow = true;
			}
			break;
	}
	
}

static void GuiDrawTouchCaliPiont(unsigned int x, unsigned int y)
{
	GUI_DrawCircle(x, y, 5);
	GUI_DrawLine(0, y, LCD_XSIZE - 1, y);
	GUI_DrawLine(x, 0, x, LCD_YSIZE - 1);
}

static void GUI_Timer_Callback(GUI_TIMER_MESSAGE *pTM)
{
	GUI_TIMER_Restart(pThisWin->hTimer);
	WM_SendMessageNoPara((WM_HWIN)pTM->Context, WM_WINDOW_TIMER);
}

static void _Create(WM_MESSAGE *pMsg)
{
	GUI_RECT rect;
	char *pStr = "30";
	WM_HWIN hWin;
	
	pThisWin->nCaliIndex = 0;
	pThisWin->bNeedRefeshWindow = true;
	pThisWin->nStartTime = Get_System_Time();

	/*中间正文提示语*/
	 hWin = TEXT_CreateEx(30, 100, LCD_XSIZE - 60, 100,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, _TouchCalibrateTip);
	TEXT_SetFont(hWin, _GetFont(Font_Content));
	TEXT_SetWrapMode(hWin, GUI_WRAPMODE_WORD);
	TEXT_SetTextColor(hWin, GUI_RED);

	GUI_SetFont(&GUI_FontTahomaBold33);
	GUI_GetTextExtend(&rect, pStr, 50);
	pThisWin->hTextTimer = TEXT_CreateEx((LCD_XSIZE - (rect.x1 - rect.x0)) >> 1, 200, rect.x1 - rect.x0 + 1, rect.y1 - rect.y0 + 1,
	                     pMsg->hWin,
	                     WM_CF_SHOW, 0, 0, pStr);
	TEXT_SetFont(pThisWin->hTextTimer, &GUI_FontTahomaBold33);
	TEXT_SetTextColor(pThisWin->hTextTimer, GUI_DARKGREEN);

	pThisWin->hTimer = GUI_TIMER_Create(GUI_Timer_Callback, Get_System_Time() + 1000, (unsigned long)pMsg->hWin, 0);
	GUI_TIMER_SetPeriod(pThisWin->hTimer, 10);

	WM_SetFocus(pMsg->hWin);
}

static void _Callback(WM_MESSAGE *pMsg)
{
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;

		case WM_PAINT:
			GUI_SetColor(GUI_WHITE);
			GUI_FillRect(0, 0, LCD_XSIZE, LCD_YSIZE);
			GUI_SetColor(GUI_RED);
			switch(pThisWin->nCaliIndex)
			{
				case 0:
					GuiDrawTouchCaliPiont(10, 10);
					break;
				case 1:
					GuiDrawTouchCaliPiont(10, LCD_YSIZE - 10);
					break;
				case 2:
					GuiDrawTouchCaliPiont(LCD_XSIZE - 10, 10);
					break;
				case 3:
					GuiDrawTouchCaliPiont(LCD_XSIZE - 10, LCD_YSIZE - 10);
					break;
			}
			break;
			
		case WM_KEY:
			return;

		case WM_WINDOW_TIMER:
			Touch_Calibrate_Scan(pMsg);
			break;
			
		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pMsg->hWin);
					break;
			}
			break;
	
		case WM_DELETE:
			GUI_TIMER_Delete(pThisWin->hTimer);
			pThisWin->hTimer = NULL;
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			break;
	}
	WM_DefaultProc(pMsg);
}


CW_TouchCalibrate *CW_TouchCalibrate_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = eTouchCalibrate;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("eTouchCalibrate window is alreay exist!\r\n");
		return (CW_TouchCalibrate *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_TouchCalibrate));

	if(hWin)
	{
		TRACE_PRINTF("eTouchCalibrate windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_TouchCalibrate *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eTouchCalibrate window can't be created!\r\n");
	}

	return pThisWin;
}

