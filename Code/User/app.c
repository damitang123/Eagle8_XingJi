#include "platform.h"
#include "buzzer.h"
#include "uart.h"
#include "app.h"
#include "spi_flash.h"
#include "rtc.h"
#include "public_function.h"
#include "ads8320.h"
#include "adc.h"
#include "gpio.h"
#include "keypad.h"
#include "GUI.h"
#include "WM.h"
#include "LCDConf.h"
#include "Windows.h"
#include "parameter.h"
#include "data.h"
#include "test.h"
#include "time_calc.h"
#include "lock.h"
#include "i2c.h"
#include "gps.h"
#include "gprs.h"
#include "gprs_user.h"
#include "delay.h"
#include "system_misc.h"
#include "uart_process.h"

sSystem g_sSystem;
sSystem * pApp = &g_sSystem;

#if SERVER_USE_XING_JIKONG_GU
cXingJiKongGu g_sXingJiKongGU;
#endif

bool g_bSpiHardwareBusBusy = false;

static void Global_Var_Init(void)
{
	memset(pApp,0,sizeof(sSystem));
	pApp->eWorkState = eWorkNormal;
}

static void InitializeBoard(void)
{
	Internal_Clk_Init();
	
	RCC_Configuration();
	NVIC_Configuration();
	
	Gpio_Init();

	Key_Init();

	Voc_Init();

	Gps_Init();

	Buzzer_Init();

	Setup_USART1();

	ADC1_Configuration();
	ADC2_Configuration();
}

static void Check_System_Clock(void)
{
	Rtc_Init();
	Rtc_Get_Time(&g_sDateTime, eDEC_Format);
	
	if(Is_Valid_DateTime(&g_sDateTime) == 0)
	{
		g_sDateTime.wYear = 17;
		g_sDateTime.wMonth = 1;
		g_sDateTime.wDay = 1;
		g_sDateTime.wHour = 12;
		g_sDateTime.wMinute = 0;
		g_sDateTime.wSecond = 0;
		Rtc_Set_Time(&g_sDateTime);
	}
}

static void Display_Startup_Logo(void)
{
	char szTemp[50];
	
	WM_Deactivate();
	
	if(g_sSystemParam.nSystemLanguage == eLanguage_Chinese)
	{
		GUI_SetFont(&GUI_Font16_UTF8);
	}
	else
	{
		GUI_SetFont(&GUI_FontTahomaBold19);
	}

	GUI_SetColor(GUI_GREEN);
	GUI_SetTextMode(GUI_TM_TRANS);
	sprintf(szTemp, "%s:%s%06ld", _DeviceNo, g_sDeviceIDConfig.aNamePrefix,g_sDeviceIDConfig.nDeviceID);
	GUI_DispStringAt(szTemp, 10, 10);
	sprintf(szTemp, "%s:%s", _Software, FW_VERSION);
	GUI_DispStringAt(szTemp, 10, 35);
	sprintf(szTemp, "%s:%s", _Hardware, HW_VERSION);
	GUI_DispStringAt(szTemp, 10, 60);

	GUI_DrawBitmap(&bmStartUpLogo, (LCD_XSIZE - bmStartUpLogo.XSize) >> 1, ((LCD_YSIZE - Bottom_Height - bmStartUpLogo.YSize) >> 1) + 60);

	WM_Activate();
}

void System_Enter_Sleep_Mode(void)
{
	if(pApp->eWorkState == eWorkNormal)
	{
		pApp->eWorkState = eWorkSleep;
		LCD_Off();
		Gpio_Ctrl(eGpio_System_Work_Led,true);
	}
}

void System_Exit_Sleep_Mode(void)
{
	if(pApp->eWorkState == eWorkSleep)
	{
		pApp->eWorkState = eWorkNormal;
		LCD_On();
		Gpio_Ctrl(eGpio_System_Work_Led,false);
	}
}

void System_Power_Down_Sub(void)
{
#if ALWAYS_SAVE_TEST_RESULT
	Data_Record_Full_Save();
#endif

#if SERVER_USE_XING_JIKONG_GU
	g_sXingJiKongGU.bDeviceState=false; //为什么没有低电量关机
	Device_Close_Packet_Upload();
#endif

	WM_Deactivate();
	GUI_Clear();
	GUI_DrawBitmap(&bmShutDown, (LCD_XSIZE-bmShutDown.XSize)>>1,(LCD_YSIZE-Bottom_Height-bmShutDown.YSize)>>1);
	delay_ms(1000);
	Gpio_Ctrl(eGpio_Power_On,false);
	TRACE_PRINTF("System power down!\r\n");
	__disable_irq();
	while(1);
}

void System_Power_Down_Check(void)
{
	/*按键持续2秒后关机且当超标时所有信息均录入完毕才可手动关机*/
#if SERVER_USE_XING_JIKONG_GU
	if(Get_Key_PressDurationTime(eKey_OK) > 2000 && g_bLongKeyEvent[KeyValueToKeyIndex(eKey_OK)]&&(g_sXingJiKongGU.bAlcValue_Excessive_Flag==false))//g_nAlcValue_Excessive_Flag==false按键控制关机   20180821
#else
	if(Get_Key_PressDurationTime(eKey_OK) > 2000 && g_bLongKeyEvent[KeyValueToKeyIndex(eKey_OK)])
#endif
	{		
		Buzzer_Beep(eBUZ_DEVICE_OK);
		System_Power_Down_Sub();
	}

	/*自动关机检查*/  //空闲时间且当超标时所有信息均录入完毕才可自动关机 20180821
#if SERVER_USE_XING_JIKONG_GU
	if(g_sSystemParam.nAutoPowerOffTime && (Get_System_Time() > pApp->nStartIdleTime + g_sSystemParam.nAutoPowerOffTime)&&(g_sXingJiKongGU.bAlcValue_Excessive_Flag==false)) //g_nAlcValue_Excessive_Flag==false
#else
	if(g_sSystemParam.nAutoPowerOffTime && (Get_System_Time() > pApp->nStartIdleTime + g_sSystemParam.nAutoPowerOffTime))
#endif
	{
		System_Power_Down_Sub();
	}
}

void Low_Power_Alarm(void)
{
	#define LOW_POWER_TIP_INTERVAL	60

	sDialogWindowProperty s;
	static unsigned long nCnt = LOW_POWER_TIP_INTERVAL;
	
	if(++nCnt > LOW_POWER_TIP_INTERVAL && Adc_Get_Battery_Volume() == 0)
	{
		nCnt = 0;
		s.nWinType = eDialogOK;
		s.pFont = _GetFont(Font_Content);
		s.pContent = _LowPowerTip;
		s.nContentColor = GUI_RED;
		s.nBackGroundColor = GUI_LIGHTGRAY;
		s.nAutoCloseTime = 3;
		s.pFunMiddleOK = NULL;
		CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
		Voc_Cmd(eVOC_LOW_POWER);
	}
	
	if(nCnt == 10)
	{
		Voc_Close();
	}
}

void Loop_10ms_Hook(void)
{
	int x,y;
	static char i= 0;
	if(++i == 2)
	{
		i = 0;
		GUI_TOUCH_Exec();
		GUI_TOUCH_GetUnstable(&x,&y);
		if(x > 0 && y > 0)
		{
			pApp->nStartIdleTime = Get_System_Time();
		}
	}
}

void Loop_50ms_Hook(void)
{
	#if SERVER_USE_XING_JIKONG_GU
	GPRS_HeartRate_Packet_Upload();
	#endif
}

void Loop_100ms_Hook(void)
{
	if(g_bSpiHardwareBusBusy == false)
	{
		Rtc_Gpio_Init();
		Rtc_Get_Time(&g_sDateTime, eDEC_Format);
	}
	WM_SendMessageNoPara(GetLastWinFromList(), WM_UPDATE_TIME);


	if(pApp->eWorkState == eWorkNormal)
	{
		if(g_sSystemParam.nAutoSleepTime && Get_System_Time() > pApp->nStartIdleTime + g_sSystemParam.nAutoSleepTime)
		{
			System_Enter_Sleep_Mode();
		}
	}
	else if(pApp->eWorkState == eWorkSleep)
	{
		if(g_sSystemParam.nAutoSleepTime && Get_System_Time() < pApp->nStartIdleTime + g_sSystemParam.nAutoSleepTime)
		{
			System_Exit_Sleep_Mode();
		}
	}

	/*自动关机检查*/
	System_Power_Down_Check();//自动
}

void Loop_1000ms_Hook(void)
{
	Gps_Data_Parse();
	Low_Power_Alarm();
}

#if SERVER_USE_XING_JIKONG_GU
static void GPRS_Variable_Init(void)
{
	GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__); //开机就启动GPRS 
	g_sXingJiKongGU.bReveiceAckFromServer=false;//g_ReveiceAckFromServer=false;
	g_sXingJiKongGU.bResult_Packet_Flag=false;// g_Result_Packet_Flag=false;
	g_sXingJiKongGU.bSingal_Strength=false;//g_Singal_Strength=false;
	g_sXingJiKongGU.bAlcValue_Excessive_Flag=false;//g_nAlcValue_Excessive_Flag=false;//added by lxl 20180821
	g_sXingJiKongGU.bDeviceState=true;
	g_sXingJiKongGU.bHeartRate=false;
	g_sXingJiKongGU.bDeviceStart=true;
	g_sXingJiKongGU.bOpenDevice=true;//已开机 added by lxl 20181107
	g_sXingJiKongGU.bAdministration=false;//已开机 added by lxl 20181107
}
#endif

void  App_Start (void)
{
	InitializeBoard();
 
	Global_Var_Init();

	Setup_USART1();
	
	System_Tick_Init();

	delay_ms(500);

	Gpio_Ctrl(eGpio_Power_On,true);
	Gpio_Ctrl(eGpio_System_Work_Led,true);

	Data_Init();

	Check_System_Clock();
	
	GUI_Init();
	GUI_UC_SetEncodeUTF8();
	WM_SetCreateFlags(WM_CF_MEMDEV);
	
	Upload_Record_Var_Init();

	LCD_On();
	Display_Startup_Logo();		

	Buzzer_Beep(eBUZ_DEVICE_OK);

	/*如果按键不释放,就一直显示开机的画面.*/
	while(KeyMiddleIsPressed())
	{
		if(KeyRightIsPressed())
		{
			pApp->bTouchCalibrateAtStartUp = true;
		}
	}

	delay_ms(100);

	pApp->bSystemPowerOn = true;

	Gpio_Ctrl(eGpio_System_Work_Led,false);

#if USE_GB_T21254_2017
	CW_SelectEnrollUser_Create(NULL, (void *)1);
	if(pApp->bTouchCalibrateAtStartUp)
	{
		CW_TouchCalibrate_Create(GetLastWinFromList(), (void *)1);
	}
#else
	CW_UserMain_Create(NULL,(void *)1);
	if(pApp->bTouchCalibrateAtStartUp)
	{
		CW_TouchCalibrate_Create(GetLastWinFromList(), (void *)1);
	}
#endif

#if SERVER_USE_XING_JIKONG_GU
	GPRS_Variable_Init();
#endif

	while(1)
	{
		GUI_Exec();
		
		if(pApp->bSystemLoginIn)
		{
			Exec_GPRS_User_Loop();
		}

		Uart_Usb_Cmd_Parser();

		if(g_s10msTimer.bIntervalEvent)
		{
			g_s10msTimer.bIntervalEvent = false;
			Loop_10ms_Hook();
		}

		if(g_s50msTimer.bIntervalEvent)
		{
			g_s50msTimer.bIntervalEvent = false;
			Loop_50ms_Hook();
		}
		
		if(g_s100msTimer.bIntervalEvent)
		{
			g_s100msTimer.bIntervalEvent = false;
			Loop_100ms_Hook();
		}

		if(g_s1000msTimer.bIntervalEvent)
		{
			g_s1000msTimer.bIntervalEvent = false;
			Loop_1000ms_Hook();
		}
	}
}

