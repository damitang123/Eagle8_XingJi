#ifndef _CALIBRATE_H_
#define _CALIBRATE_H_

#include "rtc.h"
#include "sample.h"

#define			ALCOHOL_SENSOR_INTEGRAL_DEBUG		0

typedef struct
{
	unsigned short nBlowRatio;		/*µ¥Î»ÊÇ ml/S */
	unsigned short nDiffAdc;
}__attribute__ ((packed))sPressVsBlowRatio;

typedef struct
{
	signed char nTemperature;
	float  fCalibrateCoefficient;
}__attribute__ ((packed))sZeroIntegralCalibrateCoefficientVsTemperature;

typedef enum
{
	eCaliHighPoint,
	eCaliLowPoint,
	eCaliQuickTest,
}eCaliState;

extern float g_fAlcoholVolume;
extern float g_fAlcoholVolumeOriginal;

extern unsigned char g_nCalibrateUnit;
extern float g_fHStandardAlcoholValue;
extern float g_fLStandardAlcoholValue;
extern float g_fQuickTestStandardAlcoholValue;

void Calibrate_Parameter_Check(sAlcCalibrationParam *pAlcCalibrationParam);
float	Calculate_Alcohol_Concentration(unsigned long nIntegralValue, float fTemperature);
float Temperature_Calibrate(float fAlc, float fTemperature);

#endif
