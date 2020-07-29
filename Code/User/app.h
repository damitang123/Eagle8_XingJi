#ifndef 	_APP_H_
#define	_APP_H_

#include "stm32f10x.h"

typedef enum
{
	eWorkSleep = 0,
	eWorkNormal,
}eWorkState;

typedef struct
{
	eWorkState eWorkState;
	unsigned long nStartIdleTime;
	unsigned long nSystemStartTime;
	unsigned long nStartLowPowerAlarmTime;
	unsigned long nLowPowerDurationTime;

	bool bSystemLoginIn;			
	bool bSystemPowerOn;		/*ϵͳ�Ƿ��������*/
	bool bNewDataNeedSave;		/*�Ƿ���������δ����*/
	bool bLoginAsAdministrator;	/*�Ƿ��Թ���Ա��ݵ�½*/
	bool bTouchCalibrateAtStartUp;
}sSystem;

#if SERVER_USE_XING_JIKONG_GU

typedef struct
{
	bool bResult_Packet_Flag;
	bool bReveiceAckFromServer;
	bool bSingal_Strength;
	bool bAlcValue_Excessive_Flag;
	bool bDeviceState;//�豸����״̬ 0 :���� 1������
	bool bHeartRate;
	bool bDeviceStart;
	bool bOpenDevice;
	bool bAdministration;
}cXingJiKongGu;

extern cXingJiKongGu g_sXingJiKongGU;
#endif

extern sSystem * pApp;
extern bool g_bSpiHardwareBusBusy;

void  App_Start (void);

#endif
