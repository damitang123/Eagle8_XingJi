#include "platform.h"
#include "Windows.h"
#include "rtc.h"
#include "parameter.h"
#include "data.h"
#include "test.h"
#include "buzzer.h"
#include "printer.h"
#include "system_misc.h"

CW_PrintRecordWindow * pThisWin;

#define	IO_BlueTooth_Power_On()	GPIOE->BSRR=GPIO_Pin_3
#define	IO_BlueTooth_Power_Off()	GPIOE->BRR=GPIO_Pin_3

#define	IO_BlueTooth_Reset_On()		GPIOE->BSRR=GPIO_Pin_4
#define	IO_BlueTooth_Reset_Off()	GPIOE->BRR=GPIO_Pin_4

#define	IO_BlueTooth_Setup_On()	GPIOD->BSRR=GPIO_Pin_2
#define	IO_BlueTooth_Setup_Off()	GPIOD->BRR=GPIO_Pin_2

#define 	IO_BlueTooth_Pair_State()	GPIOD->IDR & GPIO_Pin_6

#define	IO_EX_Printer_Power_On()	GPIOE->BSRR = GPIO_Pin_2
#define	IO_EX_Printer_Power_Off()	GPIOE->BRR = GPIO_Pin_2


/*科运自定义打印机*/
void PrintUnicodeString(const char *Str, char nFontSize)
{
	Printer_Uart_Send(0xbc);
	Printer_Uart_Send(0xfd);
	Printer_Uart_Send(0x05);
	Printer_Uart_Send(strlen(Str) + 2);
	Printer_Uart_Send(0xFF);	/*0x30:空白行,0x31:下划线分隔符,0x32:星号分隔符,0xFF:文字*/
	Printer_Uart_Send(nFontSize == 1 ? 0x20 : 0x30);	/*字体大小,取值:0x10,0x20,0x30*/
	while(*Str)
	{
		Printer_Uart_Send(*Str);
		Str++;
	}
	Printer_Uart_Send(0);
	Printer_Uart_Send(0);
	Printer_Uart_Send(0xAE);
}

static void PrinterStartReq(void)
{
	if(g_Language == eLanguage_Vietnamese)
	{
		Printer_Uart_Send(0xbc);
		Printer_Uart_Send(0xfd);
		Printer_Uart_Send(0x11);	/*0x11,打印请求,0x02停止打印*/
		Printer_Uart_Send(0x02);	/*DATA长度描述*/
		Printer_Uart_Send(0x06);	/*0x02:UNICODE, 0x03:ASCII或UTF8, 0x04:立陶宛语, 0x05:土库曼语, 0x06:越南语*/
		Printer_Uart_Send(45);	/*最大可接收的行数*/
		Printer_Uart_Send(0);	/*校验码,未使用*/
		Printer_Uart_Send(0);
		Printer_Uart_Send(0xAE);
	}
	else
	{
		Printer_Uart_Send(0xbc);
		Printer_Uart_Send(0xfd);
		Printer_Uart_Send(0x11);
		Printer_Uart_Send(0x02);
		Printer_Uart_Send(0x03);	/*0x02:UNICODE, 0x03:ASCII或UTF8, 0x04:立陶宛语, 0x05:土库曼语, 0x06:越南语*/
		Printer_Uart_Send(45);	/*最大可接收的行数*/
		Printer_Uart_Send(0);	/*校验码,未使用*/
		Printer_Uart_Send(0);
		Printer_Uart_Send(0xAE);
	}
}

static void PrinterStopReq(void)
{
	Printer_Uart_Send(0xbc);
	Printer_Uart_Send(0xfd);
	Printer_Uart_Send(0x02);	/*0x11,打印请求,0x02停止打印*/
	Printer_Uart_Send(0x00);	/*DATA长度描述*/
	Printer_Uart_Send(0);	/*校验码,未使用*/
	Printer_Uart_Send(0);
	Printer_Uart_Send(0xAE);
}

void Printer_Print_Line_Blank(void)
{
	Printer_Uart_Send(0xbc);
	Printer_Uart_Send(0xfd);
	Printer_Uart_Send(0x05);
	Printer_Uart_Send(0x02);
	Printer_Uart_Send(0x30);
	Printer_Uart_Send(0x20);
	Printer_Uart_Send(0);
	Printer_Uart_Send(0);
	Printer_Uart_Send(0xAE);
}

void Printer_Print_Line_Underline(void)
{
	Printer_Uart_Send(0xbc);
	Printer_Uart_Send(0xfd);
	Printer_Uart_Send(0x05);
	Printer_Uart_Send(0x02);
	Printer_Uart_Send(0x31);
	Printer_Uart_Send(0x20);
	Printer_Uart_Send(0);
	Printer_Uart_Send(0);
	Printer_Uart_Send(0xAE);
}

void Printer_Print_Line_Dot(void)
{
	Printer_Uart_Send(0xbc);
	Printer_Uart_Send(0xfd);
	Printer_Uart_Send(0x05);
	Printer_Uart_Send(0x02);
	Printer_Uart_Send(0x32);
	Printer_Uart_Send(0x20);
	Printer_Uart_Send(0);
	Printer_Uart_Send(0);
	Printer_Uart_Send(0xAE);
}

void Delete_Char_In_String(const char * aSrc, char *aDest, char nChar)
{	
	char *pDest = aDest;
	const char *pSrc = aSrc;
	
	while(*pSrc)
	{
		if(*pSrc == nChar)
		{
			pSrc++;
			continue;
		}
		*pDest = *pSrc;
		pSrc++;
		pDest++;
	}
}

#if defined(CUST_CHINA_SZJAZN)

static void PrintData(sRecord *pRecord)
{
	float fTemp = 0;
	char aTemp[100] = {0}, afValue[20] = {0};
	
	/*设备名称*/
	sprintf(aTemp, "       %s", g_sDeviceIDConfig.aDeviceTypeString);
	PrintUnicodeString(aTemp, 1);

	sprintf(aTemp, "%s%s%06ld", _P_DeviceID, g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	PrintUnicodeString(aTemp, 1);

	/*记录号*/
	sprintf(aTemp, "%s%08ld", _P_RecordNumber, pRecord->nRecordIndex);
	PrintUnicodeString(aTemp, 1);
	
	/*记录日期*/
	sprintf(aTemp, "%s%04d/%02d/%02d", _P_Date, pRecord->aDateTime[0] + 2000, pRecord->aDateTime[1], pRecord->aDateTime[2]);
	PrintUnicodeString(aTemp, 1);

	/*记录时间*/
	sprintf(aTemp, "%s%02d:%02d", _P_Time, pRecord->aDateTime[3], pRecord->aDateTime[4]);
	PrintUnicodeString(aTemp, 1);

	/*归零*/
	sprintf(aTemp, "%s0.000", _P_Zero);
	PrintUnicodeString(aTemp, 1);
		
	/*测试模式*/
	if(pRecord->nTestMode == eTestActive)
	{
		sprintf(aTemp, "%s%s", _P_TestMode, _P_ActiveTest);
	}
	else if(pRecord->nTestMode == eTestPassive)
	{
		sprintf(aTemp, "%s%s", _P_TestMode, _P_PassiveTest);
	}
	else if(pRecord->nTestMode == eTestQuickTest)
	{
		sprintf(aTemp, "%s%s", _P_TestMode, _P_QucikTest);
	}
	else if(pRecord->nTestMode == eTestRefuseMode)
	{
		sprintf(aTemp, "%s", _P_RefuseTest);
	}
	PrintUnicodeString(aTemp, 1);

	/*呼气或者血液酒精浓度*/
	sprintf(aTemp, "%s", pRecord->nTestUnit > 4 ? _P_BrAC : _P_BAC);
	PrintUnicodeString(aTemp, 1);
	
	/*测试值*/
	fTemp = Get_Final_Convert_Display_Result(pRecord->fAlcValue,pRecord->nTestUnit, pRecord->nKBrACBAC);
	Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit, afValue,eFormatToInt);
	sprintf(aTemp, "%s%s", afValue, Get_Unit_String(pRecord->nTestUnit));
	PrintUnicodeString(aTemp, 2);

	if(pRecord->nTestMode != eTestRefuseMode)
	{
		if(pRecord->fAlcValue < 20)
			sprintf(aTemp, "%s%s",_P_TestResult, _P_TestRestltNormal);
		else if(pRecord->fAlcValue >= 20 && pRecord->fAlcValue < 80)
			sprintf(aTemp, "%s%s", _P_TestResult, _P_TestRestltDrink);
		else if(pRecord->fAlcValue >= 80)
			sprintf(aTemp, "%s%s", _P_TestResult, _P_TestRestltDrunk);
	}
	else
	{
		sprintf(aTemp, "%s%s", _P_TestResult, _P_RefuseTest);
	}
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*地点*/
	sprintf(aTemp, "%s", _P_Location);
	PrintUnicodeString(aTemp, 1);
	if(GUI__GetNumChars((char *)pRecord->aLocation) > 11)
	{
		int nBytes;
		nBytes = GUI_UC__NumChars2NumBytes((char *)pRecord->aLocation,11);
		strncpy(aTemp, (char *)pRecord->aLocation,nBytes);
		PrintUnicodeString(aTemp, 1);
		sprintf(aTemp, "%s ", pRecord->aLocation + nBytes);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}
	else
	{
		sprintf(aTemp, "%s ", pRecord->aLocation);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}

	/*被测试人*/
	sprintf(aTemp, "%s", "\xE8\xA2\xAB\xE6\xB5\x8B\xE8\xAF\x95\xE4\xBA\xBA\xE5\xA7\x93\xE5\x90\x8D\x3A" /*被测试人姓名:*/);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aTestee);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*车牌号*/
	sprintf(aTemp, "%s", _P_PlateNumber);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aPlateNumber);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*驾驶证号*/
	sprintf(aTemp, "%s", "\xE8\xAF\x81\xE4\xBB\xB6\xE5\x8F\xB7\x3A" /*证件号:*/);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aDriverLicense);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*被测人无异议签名*/
	sprintf(aTemp, "%s", _P_SubjectSignature);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", "");
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*执勤民警警号*/
	sprintf(aTemp, "%s", _P_PoliceID);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aPoliceID);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*执勤民警名字*/
	sprintf(aTemp, "%s", _P_PoliceName);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aPoliceName);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*所属单位*/
	sprintf(aTemp, "%s", _P_Department);
	PrintUnicodeString(aTemp, 1);
	if(GUI__GetNumChars((char *)pRecord->aDepartment) > 11)
	{
		int nBytes;
		nBytes = GUI_UC__NumChars2NumBytes((char *)pRecord->aDepartment,11);
		strncpy(aTemp, (char *)pRecord->aDepartment,nBytes);
		PrintUnicodeString(aTemp, 1);
		sprintf(aTemp, "%s ", pRecord->aDepartment + nBytes);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}
	else
	{
		sprintf(aTemp, "%s ", pRecord->aDepartment);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}
	
	Printer_Print_Line_Blank();
	Printer_Print_Line_Blank();
	Printer_Print_Line_Dot();
	
	Printer_Print_Line_Blank();
	Printer_Print_Line_Blank();
	Printer_Print_Line_Blank();
	Printer_Print_Line_Blank();
}


#else

static void PrintData(sRecord *pRecord)
{
	float fTemp = 0;
	char aTemp[100] = {0}, afValue[20] = {0};
	char strLongitude[15], strLatitude[15];
	float fLongitude, fLatitude;
	
	/*设备名称*/
	sprintf(aTemp, "       %s", g_sDeviceIDConfig.aDeviceTypeString);
	PrintUnicodeString(aTemp, 1);

	sprintf(aTemp, "%s%s%06ld", _P_DeviceID, g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	PrintUnicodeString(aTemp, 1);

	/*记录号*/
	sprintf(aTemp, "%s%08ld", _P_RecordNumber, pRecord->nRecordIndex);
	PrintUnicodeString(aTemp, 1);
	
	/*记录日期*/
	sprintf(aTemp, "%s%04d/%02d/%02d", _P_Date, pRecord->aDateTime[0] + 2000, pRecord->aDateTime[1], pRecord->aDateTime[2]);
	PrintUnicodeString(aTemp, 1);

	/*记录时间*/
	sprintf(aTemp, "%s%02d:%02d:%02d", _P_Time, pRecord->aDateTime[3], pRecord->aDateTime[4],pRecord->aDateTime[5]);
	PrintUnicodeString(aTemp, 1);

	/*归零*/
	sprintf(aTemp, "%s0.000", _P_Zero);
	PrintUnicodeString(aTemp, 1);
		
	/*测试模式*/
	if(pRecord->nTestMode == eTestActive)
	{
		sprintf(aTemp, "%s%s", _P_TestMode, _P_ActiveTest);
	}
	else if(pRecord->nTestMode == eTestPassive)
	{
		sprintf(aTemp, "%s%s", _P_TestMode, _P_PassiveTest);
	}
	else if(pRecord->nTestMode == eTestQuickTest)
	{
		sprintf(aTemp, "%s%s", _P_TestMode, _P_QucikTest);
	}
	else if(pRecord->nTestMode == eTestRefuseMode)
	{
		sprintf(aTemp, "%s", _P_RefuseTest);
	}
	else if(pRecord->nTestMode == eTestMaintenance)
	{
		sprintf(aTemp, "%s\n", "\xE7\xBB\xB4\xE6\x8A\xA4\xE6\xB5\x8B\xE8\xAF\x95" /*维护测试*/);
	}
	PrintUnicodeString(aTemp, 1);

	/*呼气或者血液酒精浓度*/
	sprintf(aTemp, "%s", pRecord->nTestUnit > 4 ? _P_BrAC : _P_BAC);
	PrintUnicodeString(aTemp, 1);
	
	/*测试值*/
	if(pRecord->fAlcValue > (pRecord->nTestMode != eTestQuickTest ? MAX_DISPLAY_ALC_VALUE : MAX_QUICK_TEST_DISPLAY_ALC_VALUE))
	{
		afValue[0] = '>';
		fTemp = Get_Final_Convert_Display_Result((pRecord->nTestMode != eTestQuickTest ? MAX_DISPLAY_ALC_VALUE : MAX_QUICK_TEST_DISPLAY_ALC_VALUE),pRecord->nTestUnit, pRecord->nKBrACBAC);
		Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit, &afValue[1],eFormatToInt);
	}
	else
	{
		fTemp = Get_Final_Convert_Display_Result(pRecord->fAlcValue,pRecord->nTestUnit, pRecord->nKBrACBAC);
		Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit, afValue,eFormatToInt);
	}
	sprintf(aTemp, "%s%s", afValue, pRecord->nTestUnit == 4 ? "\xE2\x80\xB0":Get_Unit_String(pRecord->nTestUnit));
	PrintUnicodeString(aTemp, 2);

	if(pRecord->nTestMode != eTestRefuseMode)
	{
		if(pRecord->fAlcValue < 20)
			sprintf(aTemp, "%s%s",_P_TestResult, _P_TestRestltNormal);
		else if(pRecord->fAlcValue >= 20 && pRecord->fAlcValue < 80)
			sprintf(aTemp, "%s%s", _P_TestResult, _P_TestRestltDrink);
		else if(pRecord->fAlcValue >= 80)
			sprintf(aTemp, "%s%s", _P_TestResult, _P_TestRestltDrunk);
	}
	else
	{
		sprintf(aTemp, "%s%s", _P_TestResult, _P_RefuseTest);
	}
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*被测试人*/
	sprintf(aTemp, "%s", _P_Testee);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aTestee);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*地点*/
	sprintf(aTemp, "%s", _P_Location);
	PrintUnicodeString(aTemp, 1);
	if(GUI__GetNumChars((char *)pRecord->aLocation) > 11)
	{
		int nBytes;
		nBytes = GUI_UC__NumChars2NumBytes((char *)pRecord->aLocation,11);
		strncpy(aTemp, (char *)pRecord->aLocation,nBytes);
		PrintUnicodeString(aTemp, 1);
		sprintf(aTemp, "%s ", pRecord->aLocation + nBytes);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}
	else
	{
		sprintf(aTemp, "%s ", pRecord->aLocation);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}

	/*经度,纬度*/
	sprintf(aTemp, "%s", _P_LongitudeLatitude);
	PrintUnicodeString(aTemp, 1);
	if(pRecord->fLongitude != 0 || pRecord->fLatitude != 0)
	{
		/*经度*/
		fLongitude = pRecord->fLongitude >= 0 ? pRecord->fLongitude : -pRecord->fLongitude;
		fTemp = (fLongitude - (int)fLongitude) * 60;
		sprintf(strLongitude, "%d\xC2\xB0%d'%d.%d\"",
		        (int)fLongitude / 100, (int)fLongitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100));
		sprintf(aTemp, "%s %s", pRecord->fLongitude > 0 ? _P_Longitude_East : _P_Longitude_West, strLongitude);
		PrintUnicodeString(aTemp, 1);
		
		/*纬度*/
		fLatitude = pRecord->fLatitude >= 0 ? pRecord->fLatitude : -pRecord->fLatitude;
		fTemp = (fLatitude - (int)fLatitude) * 60;
		sprintf(strLatitude, "%d\xC2\xB0%d'%d.%d\"",
		        (int)fLatitude / 100, (int)fLatitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100));
		sprintf(aTemp, "%s %s", pRecord->fLatitude > 0 ? _P_Latitude_North : _P_Latitude_South, strLatitude);
		PrintUnicodeString(aTemp, 1);
	}
	else
	{
		Printer_Print_Line_Blank();
		Printer_Print_Line_Blank();
	}
	Printer_Print_Line_Underline();

	/*被测人无异议签名*/
	sprintf(aTemp, "%s", _P_SubjectSignature);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", "");
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*车牌号*/
	sprintf(aTemp, "%s", _P_PlateNumber);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aPlateNumber);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*驾驶证号*/
	sprintf(aTemp, "%s", _P_DriverLicense);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aDriverLicense);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*执勤民警名字*/
	sprintf(aTemp, "%s", _P_PoliceName);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aPoliceName);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();
	
	/*执勤民警警号*/
	sprintf(aTemp, "%s", _P_PoliceID);
	PrintUnicodeString(aTemp, 1);
	sprintf(aTemp, "%s ", pRecord->aPoliceID);
	PrintUnicodeString(aTemp, 1);
	Printer_Print_Line_Underline();

	/*所属单位*/
	sprintf(aTemp, "%s", _P_Department);
	PrintUnicodeString(aTemp, 1);
	if(GUI__GetNumChars((char *)pRecord->aDepartment) > 11)
	{
		int nBytes;
		nBytes = GUI_UC__NumChars2NumBytes((char *)pRecord->aDepartment,11);
		strncpy(aTemp, (char *)pRecord->aDepartment,nBytes);
		PrintUnicodeString(aTemp, 1);
		sprintf(aTemp, "%s ", pRecord->aDepartment + nBytes);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}
	else
	{
		sprintf(aTemp, "%s ", pRecord->aDepartment);
		PrintUnicodeString(aTemp, 1);
		Printer_Print_Line_Underline();
	}
	
	//Printer_Print_Line_Blank(); //deleted date 20200602
	//Printer_Print_Line_Blank();
	Printer_Print_Line_Dot();
	
	Printer_Print_Line_Blank();
	Printer_Print_Line_Blank();
	Printer_Print_Line_Blank();
	//Printer_Print_Line_Blank(); //deleted date 20200602
}

#endif

void Display_Cur_PrinterState(const char *s)
{
	if(pThisWin->hContent)
	{
		TEXT_SetText(pThisWin->hContent, s);
	}
	else
	{
		pThisWin->hContent = TEXT_CreateEx(5, Caption_Height + 80, LCD_XSIZE - 10, LCD_YSIZE - Bottom_Height - (Caption_Height + 80),
		                     pThisWin->wObj.hWin,
		                     WM_CF_SHOW, 0, 0, s);
		TEXT_SetWrapMode(pThisWin->hContent, GUI_WRAPMODE_WORD);
		TEXT_SetTextColor(pThisWin->hContent, GUI_BLUE);
		TEXT_SetTextAlign(pThisWin->hContent, GUI_TA_CENTER);
		TEXT_SetFont(pThisWin->hContent, _GetFont(Font_Content));
	}
}

void Scan_Printer_State(void)
{
	switch(pThisWin->ePrinterState)
	{
		case ePrinterStart:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				Printer_Init();
				
				Display_Cur_PrinterState(_ConnectPrinter);
				pThisWin->nStartTime = Get_System_Time();
			}
			
			if(Get_System_Time() > pThisWin->nStartTime + 200)
			{
				pThisWin->ePrinterState = ePrinterReset;
				pThisWin->bNeedRefreshState = true;
			}
			break;
			
		case ePrinterReset:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				pThisWin->nStartTime = Get_System_Time();
			}

			if(Get_System_Time() > pThisWin->nStartTime + 100)
			{
				pThisWin->nRetryCnt = 3;
				pThisWin->ePrinterState = ePrinterSendPirntReq;
				pThisWin->bNeedRefreshState = true;
			}
			break;
			
		case ePrinterWaitEnterPairMode:
			break;
			
		case ePrinterWaitPairSuccessful:
			break;
			
		case ePrinterPairConnectFail:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				Display_Cur_PrinterState(_ConnectPrinterFail);
			}
			break;
			
		case ePairSuccessfulDelay:
			break;
			
		case ePrinterSendPirntReq:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				PrinterStartReq();
				pThisWin->nStartTime = Get_System_Time();
			}

			if(Get_System_Time() > pThisWin->nStartTime + 1000)
			{
				if(pThisWin->nRetryCnt)
				{
					pThisWin->nRetryCnt --;
					pThisWin->bNeedRefreshState = true;
				}
				else
				{
					pThisWin->ePrinterState = ePrinterPairConnectFail;
					pThisWin->bNeedRefreshState = true;
				}
			}

			if(g_sPrinter.bReadyOK)
			{
				pThisWin->ePrinterState = ePrinterSendPrintData;
				pThisWin->bNeedRefreshState = true;
			}
			break;
			
		case ePrinterSendPrintData:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				PrintData(&g_sRecord);
				PrinterStopReq();
				Display_Cur_PrinterState(_Printing);
				pThisWin->ePrinterState = ePrinterWaitPrintFinish;
			}
			break;
			
		case ePrinterWaitPrintFinish:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				PrintData(&g_sRecord);
				PrinterStopReq();
				pThisWin->ePrinterState = ePrinterWaitPrintFinish;
			}

			
			if(g_sPrinter.bOutOfPaper)
			{
				pThisWin->ePrinterState = ePrinterOutOfPaper;
				pThisWin->bNeedRefreshState = true;
			}
			else if(g_sPrinter.bFinishPrint)
			{
				if(--pThisWin->nPrintCopys)
				{
					g_sPrinter.bFinishPrint = false;
					pThisWin->ePrinterState = ePrinterSendPirntReq;
					pThisWin->bNeedRefreshState = true;
				}
				else
				{
					pThisWin->ePrinterState = ePrinterFinish;
					pThisWin->bNeedRefreshState = true;
				}
			}
			break;
			
		case ePrinterOutOfPaper:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				Voc_Cmd(eVOC_PRINT_NOPAPER);
				Display_Cur_PrinterState(_PrinterNoPaper);
				BUTTON_SetText(pThisWin->hMiddleButton, _OK);
				Printer_Shutdown();
			}
			break;
			
		case ePrinterFinish:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				Display_Cur_PrinterState(_PrintFinish);
				BUTTON_SetText(pThisWin->hMiddleButton, _OK);
				Printer_Shutdown();
			}
			break;
			
		case ePrinterExit:
			if(pThisWin->bNeedRefreshState)
			{
				pThisWin->bNeedRefreshState = false;
				Display_Cur_PrinterState(_ShutdownPrinter);
				pThisWin->nStartTime = Get_System_Time();
			}

			if(Get_System_Time() > pThisWin->nStartTime + 500)
			{
				WM_DeleteWindow(pThisWin->wObj.hWin);
			}
			break;
	}
}

void Scan_Printer_Pair_State(void)
{
	
}

static void GUI_Timer_Printer_Callback(GUI_TIMER_MESSAGE *pTM)
{
	GUI_TIMER_Restart(pThisWin->hTMPrinter);
	WM_SendMessageNoPara((WM_HWIN)pTM->Context, WM_CHECK_PRINTER_STATE);
}

static void _Create(WM_MESSAGE *pMsg)
{
	if(pThisWin->wObj.Property == 0)
	{
		pMsg->Data.p = _Print;
	}
	else
	{
		pMsg->Data.p = _PrinterPair;
	}
	Create_Dialog_Caption(pMsg);

	pThisWin->bNeedRefreshState = true;
	pThisWin->ePrinterState = ePrinterStart;
	pThisWin->nPrintCopys = g_sPrinterParam.nPrintCopys;

	/*创建中键*/
	pThisWin->hMiddleButton = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(pThisWin->hMiddleButton, _Cancel);

	/*创建扫描定时器*/
	pThisWin->hTMPrinter = GUI_TIMER_Create(GUI_Timer_Printer_Callback, Get_System_Time(), (unsigned long)pMsg->hWin, 0);
	GUI_TIMER_SetPeriod(pThisWin->hTMPrinter, 10);

	WM_SetFocus(pMsg->hWin);
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
					case GUI_KEY_LEFT_FUN:
						break;
						
					case GUI_KEY_RIGHT_FUN:
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						if(pThisWin->ePrinterState != ePrinterExit)
						{
							pThisWin->ePrinterState = ePrinterExit;
							pThisWin->bNeedRefreshState = true;
						}
						break;
				}
			}
			return;
			
		case WM_CHECK_PRINTER_STATE:
			if(pThisWin->wObj.Property == 0)
			{
				Scan_Printer_State();
			}
			else
			{
				Scan_Printer_Pair_State();
			}
			Printer_Process_Command();
			break;

		case WM_NOTIFY_PARENT:
			switch(pMsg->Data.v)
			{
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pMsg->hWin);
					break;
			}
			return;
			
		case WM_DELETE:
			Printer_Shutdown();	
			GUI_TIMER_Delete(pThisWin->hTMPrinter);
			pThisWin->hTMPrinter = NULL;
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("ePrintRecord window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_PrintRecordWindow *CW_PrintRecordWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	eUserWindow WinType = ePrintRecord;

	if(GetWinHandleByWinType(WinType))
	{
		TRACE_PRINTF("ePrintRecord window is alreay exist!\r\n");
		return (CW_PrintRecordWindow *)0;
	}

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_PrintRecordWindow));

	if(hWin)
	{
		TRACE_PRINTF("ePrintRecord windows is created!hWin = 0x%04X\r\n",hWin);
		
		pThisWin = (CW_PrintRecordWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("ePrintRecord window can't be created!\r\n");
	}

	return pThisWin;
}


