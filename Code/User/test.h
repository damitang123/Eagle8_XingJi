#ifndef _TEST_H_
#define _TEST_H_

#define 	USE_HEAT_DEVICE					0            /*���ȹ���20150928���*/
#define 	HEAT_CRITICAL_TEMPERATURE		-2		/*�����ٽ������¶�*/
#define   HEAT_UPLEVL_TEMP   				20.0		/*30.0*/    /*�����ٽ������¶�*/
#define   HEAT_SENSOR_CRITICAL_TEMP 		15.0		/*25.0*/ /*���Ȳ��Թ�����sensor�ٽ������¶�*/

#define 	MODE_CALIBRATE					1
#define 	MODE_MANUAL_TEST					2
#define 	MODE_FIRST_TEST					3
#define 	MODE_ROLLING_TEST					4

/*��ⴰ��*/
#define QUICK_TEST_WINDOW_X0				0
#define QUICK_TEST_WINDOW_Y0				55	
#define QUICK_TEST_WINDOW_XWITH			240
#define QUICK_TEST_WINDOW_YHIGH			205


#define 	ADC_PUMP_VOLTAGE_READY			2400	/*���ó�����׼���õ�ֵ*/


typedef enum
{
	eIdle,
	eStart,
	ePrepare,
	eAlcClean,
	eWaitStart,
	eStartTestPrepare,
	eInhaling,
	eWaitBlowing,
	eBlowing,
	eBlowInterrupt,
	ePump,
	eProcessing,
	eFinishTest,
	eDisplayResult,
} eTestState;


typedef enum
{
	eTestForNormal,
	eTestForCalibration,
	eTestForMaintain,
} eAlcTestPurpose;

typedef enum
{
	eTestActive = 0,
	eTestPassive,
	eTestRefuseMode,
	eTestQuickTest,
	eTestAllMode,
	eTestMaintenance = 0xFA,
} eTestMode;


extern volatile eTestState g_eTestState;
extern eTestMode g_eTestMode;
extern eAlcTestPurpose g_eAlcTestPurpose;

extern unsigned char g_nInvalidBlowPressCnt;

unsigned char Get_Curve_Windows_Height(void);
unsigned char Get_Curve_Windows_Width(void);


#endif

