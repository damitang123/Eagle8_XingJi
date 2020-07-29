#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include "platform.h"

typedef struct
{
	unsigned char 	nVersion;				/*�����ļ��汾*/
	unsigned char 	nTestUnit;				/*�ƾ����Ե�λ*/	
	unsigned short	nBrACBACK;				/*�����ƾ�Ũ����ѪҺ�ƾ�Ũ�Ȼ���ϵ��*/
	unsigned long		nInhaleTime;			/*��׼����ʱ��,��λ1ms*/
	float				fAlcDrinkThreshold;		/*�о��ж�����,��λmg/100ml*/
	float				fAlcDrunkThreshold;		/*����ж�����,��λmg/100ml*/
	unsigned char		nResultMode;			/*���ģʽ,����0(��ʾ��ֵ),����1(��ʾ�о�,�޾�),2����+����..*/
	unsigned char		nAdjustCoefficientPeek;		/*�ƾ�����ֵ����ϵ��(�����),ȡֵ��Χ78~122.*/
	unsigned long		nBlowTime;				/*��С����ʱ��,��λ1ms*/
	unsigned char		nBlowPress;				/*��Сѹ��ֵ,��1,2,3��ֵ.*/
	unsigned char		nAdjustCoefficientIntegral;	/*�ƾ�����ֵ����ϵ��(������),ȡֵ��Χ78~122.*/
	unsigned char		nAlcAlarmMode;			/*0:�ر�,1:������,2:����,3:����+��*/
	unsigned char 	nBlowFlowVolume;		/*������С��������,��λ��0.1L(�õ�����ֵ����0.1)*/
	unsigned char 	nSampleMethod;			/*����ʱ��������,0��ʾ��ʱ����,1��ʾβ����������.*/
	unsigned char		nAlcMinRemapValue;		/*���Գ���ֵС�ڴ˲���,����ʾΪ0.*/
	unsigned char		aReserved[3];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sTestParam;

typedef struct
{
	unsigned char 	nVersion;				/*�����ļ��汾*/
	unsigned char 	nSystemLanguage;		/*0,Ӣ��  1����*/
	char 			aAdminPassword[10];		/*��������Ա����,��������.*/
	unsigned char		nVoiceSwitch;			/*0����������,1��������*/
	unsigned char 	nVoiceVolume;			/*��������*/
	unsigned long 		nAutoSleepTime;			/*0�ǴӲ�,������λΪms*/
	unsigned long		nAutoPowerOffTime;		/*0�ǴӲ�,������λΪms,�ҹػ�ʱ��Ҫ��������ʱ��.*/
	unsigned char		nSystemDisplayTheme; 	/*ϵͳ��ʾ������ʽ,0�Զ�ѡ��,1Ϊ�ڵװ���,2Ϊ�׵׺���*/
	unsigned char		nAutoBackLightTuneSwitch; /*�Զ����ȸ��ݻ��������ʹ��.0Ϊ������,1Ϊ����*/
	unsigned char		nBackLightPercent;		/*�������Ȱٷֱ�,ȡֵ0~100*/
	char				aIMEI[16];
	char				nIMEIValidFlag;			/*IMEI�Ƿ���Ч�ı�־*/
	unsigned short 	nAlcAlarmDurationTime;	/*�ƾ����Գ���󱨾�ʱ��,0��һֱ,������λΪ1ms*/
	signed char 		nTimeZone;				/*ʱ��*/
	bool				bEnableTraceFunction;	/*ʹ��TRACE����.*/
	signed char 		nFlowVolumeAdjustCoefficient;	/*����У׼����-->0ʱ����ԭֵ����.1�ϵ�0.05%, -1ʱ�µ�0.05%*/
	unsigned char		aReserved[16];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sSystemParam;

typedef struct
{
	unsigned char 	nVersion;				/*�����ļ��汾*/
	unsigned char		nPrinterType;			/*��ӡ������*/
	unsigned char		nPrintCopys;				/*��ӡ����,ȡֵ0~5*/	
	unsigned char		aReserved[12];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sPrinterParam;

#if SERVER_USE_XING_JIKONG_GU
typedef struct
{
	unsigned char 	byVersion;				/*???????t��?��?*/
	unsigned short 	byHeartRate;			/*D?��??��?��10-60*/
	unsigned char	a_byReserved[12];
	unsigned char 	byValidDataFlag;
}__attribute__ ((packed)) sHeartRateParam;

extern sHeartRateParam g_sHeartRateParam;
#endif

typedef struct
{
	unsigned char 	nVersion;				/*�����ļ��汾*/
	
	unsigned char 	aDateTimePeek[6];		/*У׼ʱ��*/
	unsigned long 		nPeekCaliValue;			/*��ֵ����У׼����*/
	float 			fPeekStandardAlcoholValue;	/*��ֵ�궨ʱ��׼��Һֵ.*/
	float 			fPeekRealTestAlcoholValue;	/*��ֵ�궨ʱʵ�ʲ���ֵ.*/
	
	unsigned char 	aDateTimeIntergral[6];	/*У׼ʱ��*/
	unsigned long 		nIntergralLowPoint;		/*���ֵ�Ũ�ȵ�У׼��*/
	float 			fLStandardAlcoholValue;	/*���ֵ�Ũ�ȱ�׼��Һֵ*/
	float 			fLRealTestAlcoholValue;	/*���ֵ�Ũ�ȵ�ʵ�ʲ���ֵ*/
	
	unsigned long 		nIntergralHighPoint;		/*���ָ�Ũ�ȵ�У׼��*/
	float 			fHStandardAlcoholValue;	/*���ָ�Ũ�ȱ�׼��Һֵ*/
	float 			fHRealTestAlcoholValue;	/*���ֵ�Ũ�ȵ�ʵ�ʲ���ֵ*/

	unsigned long 		nUserIntergralLowPoint;	/*�û���ʾֵ*/
	unsigned long 		nUserIntergralHighPoint;	/*�û���ʾֵ*/

	unsigned long 		nZeroIntergralValue;		/*���У��ֵ*/
	
	unsigned char		aReserved[1];

	unsigned char 	nCurrCalibrteState;		/*0��ʾ��Ũ��,1��ʾ��Ũ��,2��ʾ���*/
	
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sAlcCalibrationParam;

typedef struct
{
	float 			fTemperature;
	float 			fAlcoholValue;
} __attribute__ ((packed))sTemperatureCalibratePoint;

typedef struct
{
	unsigned char 	nVersion;				/*�����ļ��汾*/
	sTemperatureCalibratePoint    TempCalPoint[12 * 4];
}__attribute__ ((packed)) sTemperatureCalibrationParam;

typedef struct
{
	unsigned short	nStartX0;
	unsigned short	nStartY0;
	unsigned short	nFileWidth;
	unsigned short	nFileHeight;
	unsigned long		nFileSize;
	unsigned char 	nFileType;				/*0��JPEGͼƬ,1������,2��BMPͼƬ.*/
	unsigned long 		nFlashStartAddress;
	unsigned char 	nValidDataFlag;
} __attribute__ ((packed)) sLogoAttribute;

typedef struct
{
	unsigned char 	nVersion;				/*�����ļ��汾*/
	char				aNamePrefix[5];
	unsigned long		nDeviceID;
	char 			aDeviceTypeString[15];
	unsigned char		aReserved[6];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed))sDeviceIDConfig;

typedef struct
{
	unsigned char 	nVersion;				/*�����ļ��汾*/
	char				aRemoteIP[16];
	char 			aRemoteURL[40];
	unsigned short 	nRemotePort;
	char				nNetAccessMode;		/*0ΪIP,1ΪDNS*/
	unsigned char		aReserved[3];
	#if SERVER_USE_XING_JIKONG_GU
	char 			aAPNString[100];			/*APN�ַ���*/
	#endif
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sServerParam;


typedef enum
{
	eFormatToInt,
	eFormatToFloat,
}eResultStringFormat;

extern sTestParam g_sTestParam;
extern sSystemParam g_sSystemParam;
extern sPrinterParam g_sPrinterParam;
extern sAlcCalibrationParam g_sAlcCalibrationParam;
extern sTemperatureCalibrationParam g_sTemperatureCalibrationParam;
extern sDeviceIDConfig g_sDeviceIDConfig;
extern sServerParam g_sServerParam;
extern sLogoAttribute g_sLogoAttribute;

unsigned char Get_Actual_Volume_Config(unsigned char nOrignalValue);
unsigned char Get_Actual_LcdBackLight_Percent(unsigned char nOrignalValue);
unsigned short Parameter_Get_MinDifferencePress(sTestParam *pTestParam);
float Get_Final_Convert_Display_Result(float fAlcValue,unsigned char nTestUnit,unsigned short nBrACBACK);
float Get_Convert_Value_From_AnyUnit(float fAlcValue, unsigned char nTestUnit, unsigned short nBrACBACK);
void Get_Format_Alc_Convert_String(float fAlcConvertValue,unsigned char nTestUnit, char * aFormatString, eResultStringFormat eStrFormat);
void Data_Init_Default_TemperatureCalibrate(void);
char *Get_Unit_String(unsigned char nTestUnit);
const char * Get_TestMode_String(unsigned char nTestUnit);

#endif

