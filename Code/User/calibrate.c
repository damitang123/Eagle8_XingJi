#include "platform.h"
#include "parameter.h"
#include "calibrate.h"
#include "Language.h"
#include "Windows.h"
#include "test.h"

/*酒精浓度值*/
float g_fAlcoholVolume = 0.0;
float g_fAlcoholVolumeOriginal;

/*温校结构体*/
sTemperatureCalibrationParam	g_sTemperatureCalData;

#define CALIBRATION_POINT_20			20
#define CALIBRATION_POINT_80			80


#define POINT20_FINE_TUNE				1.0
#define POINT80_FINE_TUNE				1.0


unsigned char g_nCalibrateUnit = 0;
/*输入标准校准溶液值,单位mg/100ml*/
float g_fHStandardAlcoholValue = 80;
float g_fLStandardAlcoholValue = 20;
float g_fQuickTestStandardAlcoholValue = 40;

void Calibrate_Parameter_Check(sAlcCalibrationParam *pAlcCalibrationParam)
{
	if(pAlcCalibrationParam->nIntergralLowPoint < 1000000 || pAlcCalibrationParam->nIntergralLowPoint > 30000000 ||
		pAlcCalibrationParam->nIntergralHighPoint < 1000000 || pAlcCalibrationParam->nIntergralHighPoint > 30000000)
	{
		pAlcCalibrationParam->nIntergralLowPoint = 11000000 / 4;
		pAlcCalibrationParam->nIntergralHighPoint = 11000000;
		pAlcCalibrationParam->nPeekCaliValue = 3160;
		pAlcCalibrationParam->nUserIntergralLowPoint = pAlcCalibrationParam->nIntergralLowPoint;
		pAlcCalibrationParam->nUserIntergralHighPoint = pAlcCalibrationParam->nIntergralHighPoint;
		pAlcCalibrationParam->fLStandardAlcoholValue = 20;
		pAlcCalibrationParam->fHStandardAlcoholValue = 80;
		pAlcCalibrationParam->fPeekStandardAlcoholValue = 40;
		pAlcCalibrationParam->aDateTimeIntergral[0] = 17;
		pAlcCalibrationParam->aDateTimeIntergral[1] = 8;
		pAlcCalibrationParam->aDateTimeIntergral[2] = 01;
		pAlcCalibrationParam->aDateTimeIntergral[3] = 12;
		pAlcCalibrationParam->aDateTimeIntergral[4] = 00;
		pAlcCalibrationParam->aDateTimeIntergral[5] = 00;
	}
}

static float Calibrate_Adjust(float fAlc)
{
	if(g_Language == eLanguage_Chinese)
	{
		if(g_sTestParam.nAdjustCoefficientIntegral >= 78 && g_sTestParam.nAdjustCoefficientIntegral <= 122 )
			return fAlc * ((float)g_sTestParam.nAdjustCoefficientIntegral / 100.0);
		else
			return fAlc * 1.0;
	}
	else
	{
		return fAlc * 1.0;
	}
}

sZeroIntegralCalibrateCoefficientVsTemperature g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[] =
{
	-15,	(float)09.5 / 20,
	-10,	(float)11.5 / 20,
	-5,	(float)13.8 / 20,
	0,	(float)15.8 / 20,
	5,	(float)16.9 / 20,
	10,	(float)18.3 / 20,
	15,	(float)19.0 / 20,
	20,	(float)19.7 / 20,
	30,	(float)20.0 / 20,
	40,	(float)18.9 / 20,
	50,	(float)18.5 / 20,
};

float Get_Zero_Intergral_Calibrate_Coefficient(float fTemperature)
{
	float k;
	unsigned char i;
	float y1, y2, x1, x2;

	if(fTemperature <= g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[0].nTemperature)
	{
		return g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[0].fCalibrateCoefficient;
	}

	if(fTemperature >= g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[ARR_SIZE(g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT) - 1].nTemperature)
	{
		return g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[ARR_SIZE(g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT) - 1].fCalibrateCoefficient;
	}

	for(i = 0; i < ARR_SIZE(g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT); i++)
	{
		if(fTemperature < g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[i].nTemperature)
			break;
	}

	y2 = g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[i].fCalibrateCoefficient;
	y1 = g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[i - 1].fCalibrateCoefficient;
	x2 = g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[i].nTemperature;
	x1 = g_sZeroIntegralCalibrateCoefficientVsTemperatureLUT[i - 1].nTemperature;

	k = (float)(y2 - y1) / (float)(x2 - x1);			/*求斜率*/

	return y1 + k * (fTemperature - x1);
}


float	Calculate_Alcohol_Concentration(unsigned long nIntegralValue,float fTemperature)
{
	float k,fRet;
	float x1, x2, y1, y2;
	unsigned long nZeroIntergralValue;

	nZeroIntergralValue = Get_Zero_Intergral_Calibrate_Coefficient(fTemperature) * (float)g_sAlcCalibrationParam.nZeroIntergralValue;

	if(nIntegralValue < nZeroIntergralValue)
	{
		fRet = 0;
	}
	else if(nIntegralValue < g_sAlcCalibrationParam.nIntergralLowPoint)
	{
		y1 = 0;
		y2 = CALIBRATION_POINT_20;
		x1 = g_sAlcCalibrationParam.nZeroIntergralValue;
		x2 = g_sAlcCalibrationParam.nIntergralLowPoint * POINT20_FINE_TUNE;

		k = (float)(y2 - y1) / (float)(x2 - x1);

		fRet = y1 + k * (nIntegralValue - x1);
	}
	else
	{
		y1 = CALIBRATION_POINT_20;
		y2 = CALIBRATION_POINT_80;
		x1 = g_sAlcCalibrationParam.nIntergralLowPoint * POINT20_FINE_TUNE;
		x2 = g_sAlcCalibrationParam.nIntergralHighPoint * POINT80_FINE_TUNE;

		k = (float)(y2 - y1) / (float)(x2 - x1);

		fRet = y1 + k * (nIntegralValue - x1);
	}

	fRet = Calibrate_Adjust(fRet);

	if(fRet >= 57 && (g_eAlcTestPurpose == eTestForNormal || g_eAlcTestPurpose == eTestForMaintain))
	{
		x1 = 57;
		x2 = 150.0;
		y1 = 1.0;
		y2 = 1.02;

		k = (y2 - y1) / (x2 - x1);

		k = y1 + k * (fRet - x1);

		fRet = fRet * k;
	}

#if USE_GB_T21254_2017
	if(fRet < 6)
	{
		fRet = 0;
	}
#else
	if(fRet < 6)
	{
		fRet = 0;
	}
#endif

	return fRet;
}



#define Cali_Debug	0

static float GetRealAlcValue(float fAlc, unsigned char nSeqTemperature)
{
	/*判断值的范围*/
	float ret = 0, k = 0;
	unsigned char nIndex = nSeqTemperature * 4;

	float Y0 = 20.0, Y1 = 40.0, Y2 = 80.0, Y3 = 150.0;

	if(fAlc <= g_sTemperatureCalibrationParam.TempCalPoint[nIndex].fAlcoholValue)
	{
		ret = (fAlc * Y0) / g_sTemperatureCalibrationParam.TempCalPoint[nIndex].fAlcoholValue;
		#if Cali_Debug
		Debug_Printf("1 ret=%f\r\n", ret);
		#endif
	}
	else if(fAlc >= g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 3].fAlcoholValue)
	{
		k = (Y3 - Y2) / (g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 3].fAlcoholValue - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 2].fAlcoholValue);
		ret = k * (fAlc - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 3].fAlcoholValue) + Y3;
		#if Cali_Debug
		Debug_Printf("1 k=%f\r\n", k);
		Debug_Printf("2 ret=%f\r\n", ret);
		#endif
	}
	else
	{
		/*定位*/
		unsigned i;
		for(i = 0; i < 4 - 1; i++)
		{
			if((fAlc >= g_sTemperatureCalibrationParam.TempCalPoint[nIndex + i].fAlcoholValue) && (fAlc <= g_sTemperatureCalibrationParam.TempCalPoint[nIndex + i + 1].fAlcoholValue))
				break;
		}

		if(i == 0)
		{
			k = (Y1 - Y0) / (g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 1].fAlcoholValue - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 0].fAlcoholValue);
			ret = k * (fAlc - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 1].fAlcoholValue) + Y1;
			#if Cali_Debug
			Debug_Printf("3 k=%f\r\n", k);
			Debug_Printf("3 ret=%f\r\n", ret);
			#endif
		}
		else if(i == 1)
		{
			k = (Y2 - Y1) / (g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 2].fAlcoholValue - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 1].fAlcoholValue);
			ret = k * (fAlc - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 2].fAlcoholValue) + Y2;
			#if Cali_Debug
			Debug_Printf("4 k=%f\r\n", k);
			Debug_Printf("4 ret=%f\r\n", ret);
			#endif
		}
		else if(i == 2)
		{
			k = (Y3 - Y2) / (g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 3].fAlcoholValue - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 2].fAlcoholValue);
			ret = k * (fAlc - g_sTemperatureCalibrationParam.TempCalPoint[nIndex + 3].fAlcoholValue) + Y3;
			#if Cali_Debug
			Debug_Printf("5 k=%f\r\n", k);
			Debug_Printf("5 ret=%f\r\n", ret);
			#endif
		}
	}

	return ret;
}

float Temperature_Calibrate(float fAlc, float fTemperature)
{
	/*先从温度进行定位*/
	unsigned char nSize;
	/*每个温度校准点有4个标准测试点.*/
	unsigned char nCaliPointPecentTemperature = 4;

	float fAlc_LeftCaliValue = 0, fAlc_RightCaliValue = 0;

	nSize = (sizeof(g_sTemperatureCalibrationParam) / sizeof(g_sTemperatureCalibrationParam.TempCalPoint[0])) / nCaliPointPecentTemperature;

	/*温度小于第一个温度点*/
	if(fTemperature <= g_sTemperatureCalibrationParam.TempCalPoint[0].fTemperature)
	{
		fAlc_LeftCaliValue = GetRealAlcValue(fAlc, 0);

		#if Cali_Debug
		Debug_Printf("fAlc_LeftCaliValue=%f\r\n", fAlc_LeftCaliValue);
		#endif

		return fAlc_LeftCaliValue;
	}
	/*温度大于最后一个温度点*/
	else if(fTemperature >= g_sTemperatureCalibrationParam.TempCalPoint[nSize * 4 - 1].fTemperature)
	{
		/*判断值的范围*/
		fAlc_RightCaliValue = GetRealAlcValue(fAlc, nSize - 1);

		#if Cali_Debug
		Debug_Printf("fAlc_LeftCaliValue=%f\r\n", fAlc_RightCaliValue);
		#endif

		return fAlc_RightCaliValue;
	}
	else
	{
		/*对温度点进行定位*/
		float k = 0;
		unsigned char i = 0;

		for(i = 0; i < nSize - 1; i++)
		{
			if((fTemperature >= g_sTemperatureCalibrationParam.TempCalPoint[i * 4].fTemperature) && (fTemperature <= g_sTemperatureCalibrationParam.TempCalPoint[(i + 1) * 4].fTemperature))
				break;
		}

		fAlc_LeftCaliValue = GetRealAlcValue(fAlc, i);
		fAlc_RightCaliValue = GetRealAlcValue(fAlc, i + 1);

		#if Cali_Debug
		Debug_Printf("fAlc_LeftCaliValue=%f\r\n", fAlc_LeftCaliValue);
		Debug_Printf("fAlc_RightCaliValue=%f\r\n", fAlc_RightCaliValue);
		#endif

		k = (fAlc_RightCaliValue - fAlc_LeftCaliValue) / (g_sTemperatureCalibrationParam.TempCalPoint[(i + 1) * 4].fTemperature - g_sTemperatureCalibrationParam.TempCalPoint[i * 4].fTemperature);

		#if Cali_Debug
		Debug_Printf("__ k ret=%f\r\n", k);
		#endif

		return k * (fTemperature - g_sTemperatureCalibrationParam.TempCalPoint[i * 4].fTemperature) + fAlc_LeftCaliValue;
	}
}


