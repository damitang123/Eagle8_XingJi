#ifndef _TIME_CALC_H_
#define _TIME_CALC_H_

#include "rtc.h"

/*
typedef struct
{
	unsigned char wYear;
	unsigned char wMonth;
	unsigned char wDay;
	unsigned char wHour;
	unsigned char wMinute;
	unsigned char wSecond;
}__attribute__ ((packed))sDateTime;
*/

typedef enum
{
	eTimeCalc_By_Days,
	eTimeCalc_By_Hours,
	eTimeCalc_By_Minites,
	eTimeCalc_By_Seconds,
} eTimeCalcMode;

signed long Calc_DiffTime(sDateTime *sStart, sDateTime *sEnd, eTimeCalcMode eCalc_mode);
void Calc_NdaysDiff(sDateTime *pResultDate, sDateTime *pStartDate, float fDays);
unsigned long Get_UTC_Value(sDateTime *sTime);
sDateTime Get_LocalTime_From_UTC(unsigned long nUTCValue);
unsigned char Is_Valid_DateTime(sDateTime *sDT);
unsigned char Is_Leapyear(int nYear);
unsigned char Get_Weekday(char nDay, char nMonth, char nYear);
unsigned short Get_ElapseDaysFromJan1st(sDateTime *pCurDate);
unsigned short Get_RemainDaysToEndOfThisYear(sDateTime *pCurDate);
void Calc_NdaysDiff_Date(sDateTime *pResultDate, sDateTime *pStartDate, signed short nDays);


#endif
