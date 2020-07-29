#ifndef _PARAMETER_H_
#define _PARAMETER_H_

#include "platform.h"

typedef struct
{
	unsigned char 	nVersion;				/*配置文件版本*/
	unsigned char 	nTestUnit;				/*酒精测试单位*/	
	unsigned short	nBrACBACK;				/*呼气酒精浓度与血液酒精浓度换算系数*/
	unsigned long		nInhaleTime;			/*标准抽气时间,单位1ms*/
	float				fAlcDrinkThreshold;		/*有酒判定门限,单位mg/100ml*/
	float				fAlcDrunkThreshold;		/*醉酒判定门限,单位mg/100ml*/
	unsigned char		nResultMode;			/*结果模式,定量0(显示数值),定性1(显示有酒,无酒),2定量+定性..*/
	unsigned char		nAdjustCoefficientPeek;		/*酒精测量值调整系数(快测用),取值范围78~122.*/
	unsigned long		nBlowTime;				/*最小吹气时间,单位1ms*/
	unsigned char		nBlowPress;				/*最小压力值,分1,2,3的值.*/
	unsigned char		nAdjustCoefficientIntegral;	/*酒精测量值调整系数(积分用),取值范围78~122.*/
	unsigned char		nAlcAlarmMode;			/*0:关闭,1:仅闪灯,2:仅振动,3:闪灯+振动*/
	unsigned char 	nBlowFlowVolume;		/*吹气最小流量设置,单位是0.1L(得到的数值除以0.1)*/
	unsigned char 	nSampleMethod;			/*测量时采样方法,0表示计时方法,1表示尾气测量方法.*/
	unsigned char		nAlcMinRemapValue;		/*测试出的值小于此参数,则显示为0.*/
	unsigned char		aReserved[3];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sTestParam;

typedef struct
{
	unsigned char 	nVersion;				/*配置文件版本*/
	unsigned char 	nSystemLanguage;		/*0,英文  1中文*/
	char 			aAdminPassword[10];		/*超级管理员密码,仅限数字.*/
	unsigned char		nVoiceSwitch;			/*0不启用语音,1启用语音*/
	unsigned char 	nVoiceVolume;			/*语音音量*/
	unsigned long 		nAutoSleepTime;			/*0是从不,其他单位为ms*/
	unsigned long		nAutoPowerOffTime;		/*0是从不,其他单位为ms,且关机时间要大于休眠时间.*/
	unsigned char		nSystemDisplayTheme; 	/*系统显示主题样式,0自动选择,1为黑底白字,2为白底黑字*/
	unsigned char		nAutoBackLightTuneSwitch; /*自动亮度根据环境光调整使能.0为不启用,1为启用*/
	unsigned char		nBackLightPercent;		/*背光亮度百分比,取值0~100*/
	char				aIMEI[16];
	char				nIMEIValidFlag;			/*IMEI是否有效的标志*/
	unsigned short 	nAlcAlarmDurationTime;	/*酒精测试超标后报警时长,0是一直,其他单位为1ms*/
	signed char 		nTimeZone;				/*时区*/
	bool				bEnableTraceFunction;	/*使能TRACE功能.*/
	signed char 		nFlowVolumeAdjustCoefficient;	/*流量校准参数-->0时保持原值不变.1上调0.05%, -1时下调0.05%*/
	unsigned char		aReserved[16];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sSystemParam;

typedef struct
{
	unsigned char 	nVersion;				/*配置文件版本*/
	unsigned char		nPrinterType;			/*打印机类型*/
	unsigned char		nPrintCopys;				/*打印份数,取值0~5*/	
	unsigned char		aReserved[12];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sPrinterParam;

#if SERVER_USE_XING_JIKONG_GU
typedef struct
{
	unsigned char 	byVersion;				/*???????t°?±?*/
	unsigned short 	byHeartRate;			/*D?ì??μ?ê10-60*/
	unsigned char	a_byReserved[12];
	unsigned char 	byValidDataFlag;
}__attribute__ ((packed)) sHeartRateParam;

extern sHeartRateParam g_sHeartRateParam;
#endif

typedef struct
{
	unsigned char 	nVersion;				/*配置文件版本*/
	
	unsigned char 	aDateTimePeek[6];		/*校准时间*/
	unsigned long 		nPeekCaliValue;			/*峰值单点校准参数*/
	float 			fPeekStandardAlcoholValue;	/*峰值标定时标准溶液值.*/
	float 			fPeekRealTestAlcoholValue;	/*峰值标定时实际测试值.*/
	
	unsigned char 	aDateTimeIntergral[6];	/*校准时间*/
	unsigned long 		nIntergralLowPoint;		/*积分低浓度的校准点*/
	float 			fLStandardAlcoholValue;	/*积分低浓度标准溶液值*/
	float 			fLRealTestAlcoholValue;	/*积分低浓度的实际测试值*/
	
	unsigned long 		nIntergralHighPoint;		/*积分高浓度的校准点*/
	float 			fHStandardAlcoholValue;	/*积分高浓度标准溶液值*/
	float 			fHRealTestAlcoholValue;	/*积分低浓度的实际测试值*/

	unsigned long 		nUserIntergralLowPoint;	/*用户显示值*/
	unsigned long 		nUserIntergralHighPoint;	/*用户显示值*/

	unsigned long 		nZeroIntergralValue;		/*零点校正值*/
	
	unsigned char		aReserved[1];

	unsigned char 	nCurrCalibrteState;		/*0表示低浓度,1表示高浓度,2表示快测*/
	
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sAlcCalibrationParam;

typedef struct
{
	float 			fTemperature;
	float 			fAlcoholValue;
} __attribute__ ((packed))sTemperatureCalibratePoint;

typedef struct
{
	unsigned char 	nVersion;				/*配置文件版本*/
	sTemperatureCalibratePoint    TempCalPoint[12 * 4];
}__attribute__ ((packed)) sTemperatureCalibrationParam;

typedef struct
{
	unsigned short	nStartX0;
	unsigned short	nStartY0;
	unsigned short	nFileWidth;
	unsigned short	nFileHeight;
	unsigned long		nFileSize;
	unsigned char 	nFileType;				/*0是JPEG图片,1是数据,2是BMP图片.*/
	unsigned long 		nFlashStartAddress;
	unsigned char 	nValidDataFlag;
} __attribute__ ((packed)) sLogoAttribute;

typedef struct
{
	unsigned char 	nVersion;				/*配置文件版本*/
	char				aNamePrefix[5];
	unsigned long		nDeviceID;
	char 			aDeviceTypeString[15];
	unsigned char		aReserved[6];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed))sDeviceIDConfig;

typedef struct
{
	unsigned char 	nVersion;				/*配置文件版本*/
	char				aRemoteIP[16];
	char 			aRemoteURL[40];
	unsigned short 	nRemotePort;
	char				nNetAccessMode;		/*0为IP,1为DNS*/
	unsigned char		aReserved[3];
	#if SERVER_USE_XING_JIKONG_GU
	char 			aAPNString[100];			/*APN字符串*/
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

