#include "platform.h"
#include "parameter.h"
#include "voice.h"
#include "CustomDisplayDriver.h"

sTestParam g_sTestParam;
sSystemParam g_sSystemParam;
sPrinterParam g_sPrinterParam;
sAlcCalibrationParam g_sAlcCalibrationParam;
sTemperatureCalibrationParam g_sTemperatureCalibrationParam;
sDeviceIDConfig g_sDeviceIDConfig;
sServerParam g_sServerParam;
sLogoAttribute g_sLogoAttribute;

/*原始参数的范围为0~100*/
unsigned char Get_Actual_Volume_Config(unsigned char nOrignalValue)
{
	unsigned char nVolume;
	nVolume = VOICE_MIN_VOLUME + nOrignalValue * (float)(VOICE_MAX_VOLUME - VOICE_MIN_VOLUME) / (float)100;
	nVolume = nVolume > VOICE_MAX_VOLUME ? VOICE_MAX_VOLUME : nVolume;
	return nVolume;
}

/*原始参数的范围为0~100*/
unsigned char Get_Actual_LcdBackLight_Percent(unsigned char nOrignalValue)
{
	unsigned char nPercent;
	nPercent = LCD_BACK_LIGHT_MIN_PWM_DUTY + nOrignalValue * (float)(LCD_BACK_LIGHT_MAX_PWM_DUTY - LCD_BACK_LIGHT_MIN_PWM_DUTY) / (float)100;
	nPercent = nPercent > LCD_BACK_LIGHT_MAX_PWM_DUTY ? LCD_BACK_LIGHT_MAX_PWM_DUTY : nPercent;
	return nPercent;
}

unsigned short Parameter_Get_MinDifferencePress(sTestParam *pTestParam)
{
#if defined(USE_NEGATIVE_PRESSURE_PIPE)
	if(pTestParam == NULL)
	{
		return 40;
	}

	if(pTestParam->nBlowPress == 0)
	{
		return 40;
	}
	else if(pTestParam->nBlowPress == 1)
	{
		return 40;
	}
	else if(pTestParam->nBlowPress == 2)
	{
		return 40 * 1.8;
	}
	else if(pTestParam->nBlowPress == 3)
	{
		return 40 * 2.6;
	}
	else
	{
		return 40;
	}
#else
	if(pTestParam == NULL)
	{
		return 85;
	}

	if(pTestParam->nBlowPress == 0)
	{
		return 85;
	}
	else if(pTestParam->nBlowPress == 1)
	{
		return 85;
	}
	else if(pTestParam->nBlowPress == 2)
	{
		return 85 * 1.8;
	}
	else if(pTestParam->nBlowPress == 3)
	{
		return 85 * 2.6;
	}
	else
	{
		return 85;
	}
#endif
}

char *Get_Unit_String(unsigned char nTestUnit)
{
	if(0 == nTestUnit)
	{
		return "mg/100ml";
	}
	else if(1 == nTestUnit)
	{
		return "g/100ml";
	}
	else if(2 == nTestUnit)
	{
		return "g/L";
	}
	else if(3 == nTestUnit)
	{
		return "%";
	}
	else if(4 == nTestUnit)
	{
		return "\xE2\x80\xB0";
	}
	else if(5 == nTestUnit)
	{
		return "mg/L";
	}
	else if(6 == nTestUnit)
	{
		return "ug/100ml";
	}
	else if(7 == nTestUnit)
	{
		return "g/210L";
	}

	return "mg/100ml";
}

/*输入参数,酒精浓度为mg/100ml的值,测试单位为序号,转换K值 为实际值,非序列号*/
float Get_Final_Convert_Display_Result(float fAlcValue, unsigned char nTestUnit, unsigned short nKBrACBAC)
{
	switch(nTestUnit)
	{
		case 0: //mg/100ml
			return fAlcValue;

		case 1://g/100ml ,%
			return fAlcValue / 1000;

		case 2://g/L,‰
			return fAlcValue / 100;

		case 3://%
			return fAlcValue / 1000;

		case 4://‰
			return fAlcValue / 100;

		case 5://mg/L
			return (fAlcValue / (float)(nKBrACBAC / 10));

		case 6://ug/100ml
			return (fAlcValue / (float)(nKBrACBAC / 10)) * (float)100;

		case 7://g/210L
			return fAlcValue / 1000;

		default:
			return fAlcValue;
	}
}

/*输入参数为其他单位,返回值单位是mg/100ml的值,测试单位为序号,转换K值 为实际值,非序列号*/
float Get_Convert_Value_From_AnyUnit(float fAlcValue, unsigned char nTestUnit, unsigned short nKBrACBAC)
{
	switch(nTestUnit)
	{
		case 0: //mg/100ml
			return fAlcValue;

		case 1://g/100ml ,%
			return fAlcValue * 1000;

		case 2://g/L,‰
			return fAlcValue * 100;

		case 3://%
			return fAlcValue * 1000;

		case 4://‰
			return fAlcValue * 100;

		case 5://mg/L
			return (fAlcValue * (float)(nKBrACBAC / 10));

		case 6://ug/100ml
			return (fAlcValue * (float)(nKBrACBAC / 10)) / (float)100;

		case 7://g/210L
			return fAlcValue * 1000;

		default:
			return fAlcValue;
	}
}

/*获得单位对应的字符串*/
const char * Get_TestMode_String(unsigned char nTestUnit)
{
	switch(nTestUnit)
	{
		case 0: //mg/100ml
		case 1://g/100ml ,%
		case 2://g/L,‰
		case 3://%
		case 4://‰---UTF8编码
			return "BAC";
			
		case 5://mg/L
		case 6://ug/100ml
		case 7://g/210L
			return "BrAC";
			
		default:
			return "BAC";
	}
}

#if USE_GB_T21254_2017

/*从转换结果,单位,得到格式化的数值*/
/*除mg/100ml 只要小数点1位,其他保留3位小数.*/
void Get_Format_Alc_Convert_String(float fAlcConvertValue,unsigned char nTestUnit, char * aFormatString, eResultStringFormat eStrFormat)
{
	if(nTestUnit)
	{
		sprintf(aFormatString,"%.3f",fAlcConvertValue);
	}
	else
	{
		if(eStrFormat == eFormatToInt)
		{
			sprintf(aFormatString,"%d",(int)fAlcConvertValue);
		}
		else if(eStrFormat == eFormatToFloat)
		{
			sprintf(aFormatString,"%.1f",fAlcConvertValue);
		}
	}
}

#else

/*从转换结果,单位,得到格式化的数值*/
/*除mg/100ml 只要小数点1位,其他保留3位小数.*/
void Get_Format_Alc_Convert_String(float fAlcConvertValue,unsigned char nTestUnit, char * aFormatString,eResultStringFormat eStrFormat)
{
	if(nTestUnit)
	{
		sprintf(aFormatString,"%.3f",fAlcConvertValue);
	}
	else
	{
		sprintf(aFormatString,"%.1f",fAlcConvertValue);
	}
}

#endif

void Data_Init_Default_TemperatureCalibrate(void)
{
	g_sTemperatureCalibrationParam.TempCalPoint[0].fTemperature = -15;
	g_sTemperatureCalibrationParam.TempCalPoint[1].fTemperature = -15;
	g_sTemperatureCalibrationParam.TempCalPoint[2].fTemperature = -15;
	g_sTemperatureCalibrationParam.TempCalPoint[3].fTemperature = -15;
	g_sTemperatureCalibrationParam.TempCalPoint[0].fAlcoholValue = 9.5;
	g_sTemperatureCalibrationParam.TempCalPoint[1].fAlcoholValue = 24.5;
	g_sTemperatureCalibrationParam.TempCalPoint[2].fAlcoholValue = 58.5;
	g_sTemperatureCalibrationParam.TempCalPoint[3].fAlcoholValue = 114.5;

	g_sTemperatureCalibrationParam.TempCalPoint[4].fTemperature = -10;
	g_sTemperatureCalibrationParam.TempCalPoint[5].fTemperature = -10;
	g_sTemperatureCalibrationParam.TempCalPoint[6].fTemperature = -10;
	g_sTemperatureCalibrationParam.TempCalPoint[7].fTemperature = -10;
	g_sTemperatureCalibrationParam.TempCalPoint[4].fAlcoholValue = 11.5;
	g_sTemperatureCalibrationParam.TempCalPoint[5].fAlcoholValue = 28.5;
	g_sTemperatureCalibrationParam.TempCalPoint[6].fAlcoholValue = 62.4;
	g_sTemperatureCalibrationParam.TempCalPoint[7].fAlcoholValue = 120.8;

	g_sTemperatureCalibrationParam.TempCalPoint[8].fTemperature = -5;
	g_sTemperatureCalibrationParam.TempCalPoint[9].fTemperature = -5;
	g_sTemperatureCalibrationParam.TempCalPoint[10].fTemperature = -5;
	g_sTemperatureCalibrationParam.TempCalPoint[11].fTemperature = -5;
	g_sTemperatureCalibrationParam.TempCalPoint[8].fAlcoholValue =  13.8;
	g_sTemperatureCalibrationParam.TempCalPoint[9].fAlcoholValue =  32.1;
	g_sTemperatureCalibrationParam.TempCalPoint[10].fAlcoholValue = 68.2;
	g_sTemperatureCalibrationParam.TempCalPoint[11].fAlcoholValue = 132.3;

	g_sTemperatureCalibrationParam.TempCalPoint[12].fTemperature = 0;
	g_sTemperatureCalibrationParam.TempCalPoint[13].fTemperature = 0;
	g_sTemperatureCalibrationParam.TempCalPoint[14].fTemperature = 0;
	g_sTemperatureCalibrationParam.TempCalPoint[15].fTemperature = 0;
	g_sTemperatureCalibrationParam.TempCalPoint[12].fAlcoholValue = 15.8;
	g_sTemperatureCalibrationParam.TempCalPoint[13].fAlcoholValue = 34.1;
	g_sTemperatureCalibrationParam.TempCalPoint[14].fAlcoholValue = 71.4;
	g_sTemperatureCalibrationParam.TempCalPoint[15].fAlcoholValue = 136.1;

	g_sTemperatureCalibrationParam.TempCalPoint[16].fTemperature = 5;
	g_sTemperatureCalibrationParam.TempCalPoint[17].fTemperature = 5;
	g_sTemperatureCalibrationParam.TempCalPoint[18].fTemperature = 5;
	g_sTemperatureCalibrationParam.TempCalPoint[19].fTemperature = 5;
	g_sTemperatureCalibrationParam.TempCalPoint[16].fAlcoholValue = 16.9;
	g_sTemperatureCalibrationParam.TempCalPoint[17].fAlcoholValue = 35.7;
	g_sTemperatureCalibrationParam.TempCalPoint[18].fAlcoholValue = 74.1;
	g_sTemperatureCalibrationParam.TempCalPoint[19].fAlcoholValue = 139.7;

	g_sTemperatureCalibrationParam.TempCalPoint[20].fTemperature = 10;
	g_sTemperatureCalibrationParam.TempCalPoint[21].fTemperature = 10;
	g_sTemperatureCalibrationParam.TempCalPoint[22].fTemperature = 10;
	g_sTemperatureCalibrationParam.TempCalPoint[23].fTemperature = 10;
	g_sTemperatureCalibrationParam.TempCalPoint[20].fAlcoholValue = 18.3;
	g_sTemperatureCalibrationParam.TempCalPoint[21].fAlcoholValue = 37.4;
	g_sTemperatureCalibrationParam.TempCalPoint[22].fAlcoholValue = 74.9;
	g_sTemperatureCalibrationParam.TempCalPoint[23].fAlcoholValue = 142.8;

	g_sTemperatureCalibrationParam.TempCalPoint[24].fTemperature = 15;
	g_sTemperatureCalibrationParam.TempCalPoint[25].fTemperature = 15;
	g_sTemperatureCalibrationParam.TempCalPoint[26].fTemperature = 15;
	g_sTemperatureCalibrationParam.TempCalPoint[27].fTemperature = 15;
	g_sTemperatureCalibrationParam.TempCalPoint[24].fAlcoholValue = 19.0;
	g_sTemperatureCalibrationParam.TempCalPoint[25].fAlcoholValue = 38.3;
	g_sTemperatureCalibrationParam.TempCalPoint[26].fAlcoholValue = 77.8;
	g_sTemperatureCalibrationParam.TempCalPoint[27].fAlcoholValue = 147.9;

	g_sTemperatureCalibrationParam.TempCalPoint[28].fTemperature = 20;
	g_sTemperatureCalibrationParam.TempCalPoint[29].fTemperature = 20;
	g_sTemperatureCalibrationParam.TempCalPoint[30].fTemperature = 20;
	g_sTemperatureCalibrationParam.TempCalPoint[31].fTemperature = 20;
	g_sTemperatureCalibrationParam.TempCalPoint[28].fAlcoholValue = 19.7;
	g_sTemperatureCalibrationParam.TempCalPoint[29].fAlcoholValue = 39.3;
	g_sTemperatureCalibrationParam.TempCalPoint[30].fAlcoholValue = 79.5;
	g_sTemperatureCalibrationParam.TempCalPoint[31].fAlcoholValue = 149.5;

	g_sTemperatureCalibrationParam.TempCalPoint[32].fTemperature = 25;
	g_sTemperatureCalibrationParam.TempCalPoint[33].fTemperature = 25;
	g_sTemperatureCalibrationParam.TempCalPoint[34].fTemperature = 25;
	g_sTemperatureCalibrationParam.TempCalPoint[35].fTemperature = 25;
	g_sTemperatureCalibrationParam.TempCalPoint[32].fAlcoholValue = 20;
	g_sTemperatureCalibrationParam.TempCalPoint[33].fAlcoholValue = 40;
	g_sTemperatureCalibrationParam.TempCalPoint[34].fAlcoholValue = 80;
	g_sTemperatureCalibrationParam.TempCalPoint[35].fAlcoholValue = 150;

	g_sTemperatureCalibrationParam.TempCalPoint[36].fTemperature = 30;
	g_sTemperatureCalibrationParam.TempCalPoint[37].fTemperature = 30;
	g_sTemperatureCalibrationParam.TempCalPoint[38].fTemperature = 30;
	g_sTemperatureCalibrationParam.TempCalPoint[39].fTemperature = 30;
	g_sTemperatureCalibrationParam.TempCalPoint[36].fAlcoholValue = 20.0;
	g_sTemperatureCalibrationParam.TempCalPoint[37].fAlcoholValue = 40;
	g_sTemperatureCalibrationParam.TempCalPoint[38].fAlcoholValue = 80;
	g_sTemperatureCalibrationParam.TempCalPoint[39].fAlcoholValue = 150;

	g_sTemperatureCalibrationParam.TempCalPoint[40].fTemperature = 35;
	g_sTemperatureCalibrationParam.TempCalPoint[41].fTemperature = 35;
	g_sTemperatureCalibrationParam.TempCalPoint[42].fTemperature = 35;
	g_sTemperatureCalibrationParam.TempCalPoint[43].fTemperature = 35;
	g_sTemperatureCalibrationParam.TempCalPoint[40].fAlcoholValue = 19;
	g_sTemperatureCalibrationParam.TempCalPoint[41].fAlcoholValue = 38;
	g_sTemperatureCalibrationParam.TempCalPoint[42].fAlcoholValue = 76;
	g_sTemperatureCalibrationParam.TempCalPoint[43].fAlcoholValue = 148;

	g_sTemperatureCalibrationParam.TempCalPoint[44].fTemperature = 40;
	g_sTemperatureCalibrationParam.TempCalPoint[45].fTemperature = 40;
	g_sTemperatureCalibrationParam.TempCalPoint[46].fTemperature = 40;
	g_sTemperatureCalibrationParam.TempCalPoint[47].fTemperature = 40;
	g_sTemperatureCalibrationParam.TempCalPoint[44].fAlcoholValue = 18.9;
	g_sTemperatureCalibrationParam.TempCalPoint[45].fAlcoholValue = 37.4;
	g_sTemperatureCalibrationParam.TempCalPoint[46].fAlcoholValue = 75.7;
	g_sTemperatureCalibrationParam.TempCalPoint[47].fAlcoholValue = 146.0;
}

