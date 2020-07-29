#include "platform.h"
#include "parameter.h"
#include "rtc.h"
#include "time_calc.h"
#include "platform.h"
#include "Windows.h"
#include "language.h"
#include "lock.h"

unsigned char g_nCalibrationState;
unsigned short g_nCalibrattionLockDays;

unsigned char g_nServiceState;
unsigned short g_nServiceLockDays;

unsigned char g_nFirstTestState;
unsigned short g_nFirstTestLockDays;

unsigned char g_nRollingTestState;
unsigned short g_nRollingTestLockDays;



/*标定检查*/
void CheckCalibrationState(void)
{
	signed long nDiffDays;
	nDiffDays = Calc_DiffTime((sDateTime *)g_sAlcCalibrationParam.aDateTimeIntergral, &g_sDateTime, eTimeCalc_By_Days);
	if(nDiffDays > 365)
	{
		#if CUST_VIETNAM_DNTIC
		if(g_Language == eLanguage_Vietnamese)
		{
			const char sStr_Tip_Calibrate[] = {0x47, 0x69, 0xC3, 0xA1, 0x20, 0x74, 0x72, 0xE1, 0xBB, 0x8B, 0x20, 0x68, 0x69, 0xE1, 0xBB, 0x87, 0x75, 0x20, 0x63, 0x68, 0x75, 0xE1, 0xBA, 0xA9, 0x6E, 0x20, 0xC4, 0x91, 0xC3, 0xA3, 0x20, 0x68, 0xE1, 0xBA, 0xBF, 0x74, 0x20, 0x68, 0xE1, 0xBA, 0xA1, 0x6E, 0x21, 0x00}; /*Giá tr? hi?u chu?n ?? h?t h?n!*/
			CW_ShowDialog_Create(5, sStr_Tip_Calibrate, GUI_DARKRED, NULL, NULL, _OK, 0);
		}
		else if(g_Language == eLanguage_English)
		{
			const char sStr_Tip_Calibrate[] = "Require calibration!";
			char aTemp[50];
			sprintf(aTemp, "%s(%ld)", sStr_Tip_Calibrate, nDiffDays);
			CW_ShowDialog_Create(5, aTemp, GUI_DARKRED, NULL, NULL, _OK, 0);
		}
		#endif
	}
}

#if 0
void DisplayCalibrationState(void)
{
	if(g_nCalibrationState != STATE_OK)
	{
		if(g_nCalibrationState == STATE_ALARM)
		{
			char aTemp[100];
			sprintf(aTemp, _Tip_Alarm_Clibration, g_nCalibrattionLockDays);
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), aTemp, 5);
			Windows_Pend_Wait_Event();
		}
		else if(g_nCalibrationState == STATE_LOCKED)
		{
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), _Tip_Lock_Clibration, 5);
			Windows_Pend_Wait_Event();
		}
	}
}


/*服务时间检查*/
void CheckServiceState(void)
{
	signed long nDiffDays;
	nDiffDays = Calc_DiffTime(&g_sParam.tmServ, &g_sDateTime, RETURN_DAYS);
	if(nDiffDays >= g_sParam.nServDays)
	{
		if(nDiffDays < (g_sParam.nServDays + g_sParam.nServLockDays))
		{
			g_nServiceState = STATE_ALARM;
			g_nServiceLockDays = g_sParam.nServDays + g_sParam.nServLockDays - nDiffDays;
		}
		else
		{
			g_nServiceState = STATE_LOCKED;
		}
	}
	else
	{
		g_nServiceState = STATE_OK;
	}
}


void DisplayServiceState(void)
{
	if(g_nServiceState != STATE_OK)
	{
		if(g_nServiceState == STATE_ALARM)
		{
			char aTemp[100];
			sprintf(aTemp, _Tip_Alarm_Service, g_nServiceLockDays);
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), aTemp, 5);
			Windows_Pend_Wait_Event();
		}
		else if(g_nServiceState == STATE_LOCKED)
		{
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), _Tip_Lock_Service, 5);
			Windows_Pend_Wait_Event();
		}
	}
}


/*初始测试检查*/
void CheckFirstTestState(void)
{
	if(g_sParam.nInitialtestLockCount)
	{
		if(g_sParam.nInitialtestFailCount >= g_sParam.nInitialtestLockCount)
		{
			signed long nDiffDays;
			nDiffDays = Calc_DiffTime(&g_sParam.tmInitialtestFail, &g_sDateTime, RETURN_DAYS);
			if(nDiffDays < 0)
			{
				g_nFirstTestState = STATE_OK;	/*人为修改系统时间*/
			}
			else if(nDiffDays <= g_sParam.nInitialtestFailLockDays)
			{
				g_nFirstTestState = STATE_ALARM;
				g_nFirstTestLockDays = g_sParam.nInitialtestFailLockDays - nDiffDays;
			}
			else
			{
				g_nFirstTestState = STATE_LOCKED;
			}
		}
		else
		{
			g_nFirstTestState = STATE_OK;
		}
	}
	else
	{
		g_nFirstTestState = STATE_OK;
	}
}


void DisplayFirstTestState(void)
{
	if(g_nFirstTestState != STATE_OK)
	{
		if(g_nFirstTestState == STATE_ALARM)
		{
			char aTemp[100];
			sprintf(aTemp, _Tip_Alarm_FirstTest, g_nFirstTestLockDays);
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), aTemp, 5);
			Windows_Pend_Wait_Event();
		}
		else if(g_nFirstTestState == STATE_LOCKED)
		{
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), _Tip_Lock_FirstTest, 5);
			Windows_Pend_Wait_Event();
		}
	}
}


/*随机测试检查*/
void CheckRollingTestState(void)
{
	if(g_sParam.nRetestFailCount >= g_sParam.nRetestFailLockCount)
	{
		signed long nDiffDays;
		nDiffDays = Calc_DiffTime(&g_sParam.tmRetestFail, &g_sDateTime, RETURN_DAYS);
		if(nDiffDays < 0)
		{
			g_nRollingTestState = STATE_OK;
		}
		else if(nDiffDays < g_sParam.nRetestFailLockDays)
		{
			g_nRollingTestState = STATE_ALARM;
			g_nRollingTestLockDays = g_sParam.nRetestFailLockDays - nDiffDays;
		}
		else
		{
			g_nRollingTestState = STATE_LOCKED;
		}
	}
	else
	{
		g_nRollingTestState = STATE_OK;
	}
}


void DisplayRollingTestState(void)
{
	if(g_nRollingTestState != STATE_OK)
	{
		if(g_nRollingTestState == STATE_ALARM)
		{
			char aTemp[100];
			sprintf(aTemp, _Tip_Alarm_RollingTest, g_nRollingTestLockDays);
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), aTemp, 5);
			Windows_Pend_Wait_Event();
		}
		else if(g_nRollingTestState == STATE_LOCKED)
		{
			CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), _Tip_Lock_RollingTest, 5);
			Windows_Pend_Wait_Event();
		}
	}
}


void Dispay_Contact_PhoneNumber(void)
{
	if(g_nCalibrationState != STATE_OK || g_nServiceState != STATE_OK || g_nFirstTestState != STATE_OK || g_nRollingTestState != STATE_OK)
	{
		char aTemp[100];
		sprintf(aTemp, _Tip_EmergencyPhone, g_sParam.szPhone1, g_sParam.szPhone2);
		CW_ShowTip_Create(0x02, _Alarm, "", _OK, _GetFont(Font_ShowTipContent), _RGB(255, 100, 0), aTemp, 5);
		Windows_Pend_Wait_Event();
	}
}
#endif

