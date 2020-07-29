#include "platform.h"
#include <time.h>
#include "rtc.h"
#include "time_calc.h"

#if 0
struct tm {
　　int tm_sec; /* 秒 – 取值区间为[0,59] */
　　int tm_min; /* 分 - 取值区间为[0,59] */
　　int tm_hour; /* 时 - 取值区间为[0,23] */
　　int tm_mday; /* 一个月中的日期 - 取值区间为[1,31] */
　　int tm_mon; /* 月份（从一月开始，0代表一月） - 取值区间为[0,11] */
　　int tm_year; /* 年份，其值等于实际年份减去1900 */
　　int tm_wday; /* 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推 */
　　int tm_yday; /* 从每年的1月1日开始的天数 – 取值区间为[0,365]，其中0代表1月1日，1代表1月2日，以此类推 */
　　int tm_isdst; /* 夏令时标识符，实行夏令时的时候，tm_isdst为正。不实行夏令时的进候，tm_isdst为0；不了解情况时，tm_isdst()为负。*/
　　};
#endif

/*计算两个日期的时间差,返回秒数或者分钟数或者小时数或者天数*/
signed long Calc_DiffTime(sDateTime *sStart, sDateTime *sEnd, eTimeCalcMode eCalc_mode)
{
	time_t nStart;
	time_t nEnd;
	struct tm tmStart, tmEnd;

	memset(&tmStart, 0, sizeof(tmStart));
	memset(&tmEnd, 0, sizeof(tmStart));

	tmStart.tm_year 	= sStart->wYear + 100;
	tmStart.tm_mon 	= sStart->wMonth - 1;
	tmStart.tm_mday	= sStart->wDay;
	tmStart.tm_hour 	= sStart->wHour;
	tmStart.tm_min 	= sStart->wMinute;
	tmStart.tm_sec 	= sStart->wSecond;


	tmEnd.tm_year 	= sEnd->wYear + 100;
	tmEnd.tm_mon 	= sEnd->wMonth - 1;
	tmEnd.tm_mday 	= sEnd->wDay;
	tmEnd.tm_hour 	= sEnd->wHour;
	tmEnd.tm_min 	= sEnd->wMinute;
	tmEnd.tm_sec 	= sEnd->wSecond;

	nStart = mktime(&tmStart);
	nEnd = mktime(&tmEnd);

	if(eCalc_mode == eTimeCalc_By_Seconds)	/*秒*/
	{
		return difftime(nEnd, nStart);
	}
	else if(eCalc_mode == eTimeCalc_By_Minites)	/*分钟*/
	{
		return difftime(nEnd, nStart) / 60;
	}
	else if(eCalc_mode == eTimeCalc_By_Hours)	/*小时*/
	{
		return difftime(nEnd, nStart) / (3600);
	}
	else if(eCalc_mode == eTimeCalc_By_Days)	/*天数*/
	{
		return difftime(nEnd, nStart) / 86400;
	}

	return difftime(nEnd, nStart);
}

/*获取某个特定日期后或前N天的日期时间.*/
void Calc_NdaysDiff(sDateTime *pResultDate, sDateTime *pStartDate, float fDays)
{
	time_t nDT;
	struct tm tmDT, *p_tmDT;

	memset(&tmDT, 0, sizeof(tmDT));

	tmDT.tm_year		= pStartDate->wYear + 100;
	tmDT.tm_mon 	= pStartDate->wMonth - 1;
	tmDT.tm_mday	= pStartDate->wDay;
	tmDT.tm_hour 	= pStartDate->wHour;
	tmDT.tm_min 		= pStartDate->wMinute;
	tmDT.tm_sec 		= pStartDate->wSecond;

	nDT = mktime(&tmDT);

	nDT += (86400 * fDays);

	p_tmDT = localtime(&nDT);

	pResultDate->wYear		= p_tmDT->tm_year - 100;
	pResultDate->wMonth		= p_tmDT->tm_mon + 1;
	pResultDate->wDay		= p_tmDT->tm_mday;
	pResultDate->wHour		= p_tmDT->tm_hour;
	pResultDate->wMinute	= p_tmDT->tm_min;
	pResultDate->wSecond	= p_tmDT->tm_sec;
}

/*获取某个时间的UTC值.*/
unsigned long Get_UTC_Value(sDateTime *sTime)
{
	struct tm sTime_t;

	sTime_t.tm_year 	= sTime->wYear + 100;
	sTime_t.tm_mon 	= sTime->wMonth - 1;
	sTime_t.tm_mday	= sTime->wDay;
	sTime_t.tm_hour 	= sTime->wHour;
	sTime_t.tm_min 	= sTime->wMinute;
	sTime_t.tm_sec 	= sTime->wSecond;

	return mktime(&sTime_t);
}

/*从UTC得到相应的时间*/
sDateTime Get_LocalTime_From_UTC(unsigned long nUTCValue)
{
	struct tm *pTime_t;
	sDateTime sDateTimeTemp;

	pTime_t = localtime((const time_t *)&nUTCValue);

	sDateTimeTemp.wYear = pTime_t -> tm_year  - 100;
	sDateTimeTemp.wMonth = pTime_t -> tm_mon  + 1;
	sDateTimeTemp.wDay = pTime_t -> tm_mday;
	sDateTimeTemp.wHour = pTime_t -> tm_hour;
	sDateTimeTemp.wMinute = pTime_t -> tm_min;
	sDateTimeTemp.wSecond = pTime_t -> tm_sec;
	//	sDateTimeTemp.wWeekday = pTime_t -> tm_wday;

	return sDateTimeTemp;
}

/*判断一个日期是否有效.*/
unsigned char Is_Valid_DateTime(sDateTime *sDT)
{
	time_t nDT;
	struct tm tmDT, tmDT1, *p_tmDT;

	memset(&tmDT, 0, sizeof(tmDT));

	tmDT.tm_year		= sDT->wYear + 100;
	tmDT.tm_mon 	= sDT->wMonth - 1;
	tmDT.tm_mday	= sDT->wDay;
	tmDT.tm_hour 	= sDT->wHour;
	tmDT.tm_min 		= sDT->wMinute;
	tmDT.tm_sec 		= sDT->wSecond;

	tmDT1 = tmDT;

	nDT = mktime(&tmDT);

	p_tmDT = localtime(&nDT);

	if(p_tmDT->tm_year != tmDT1.tm_year) return 0;

	if(p_tmDT->tm_mon != tmDT1.tm_mon) return 0;

	if(p_tmDT->tm_mday != tmDT1.tm_mday) return 0;

	if(p_tmDT->tm_hour != tmDT1.tm_hour) return 0;

	if(p_tmDT->tm_min != tmDT1.tm_min) return 0;

	if(p_tmDT->tm_sec != tmDT1.tm_sec) return 0;

	return 1;
}

/*判断一个年份是否为闰年*/
unsigned char Is_Leapyear(int nYear)
{
	if((nYear % 400 == 0 ) || (nYear % 4 == 0 && nYear % 100 != 0))
		return 1;
	else
		return 0;
}

/*获得某天的星期数*/
unsigned char Get_Weekday(char nDay, char nMonth, char nYear)
{
	if(nMonth <= 2)
	{
		nMonth += 12;
		nYear--;
	}

	/*周一为1,周日为7*/
	return (nDay + 2 * nMonth + 3 * (nMonth + 1) / 5 + nYear + nYear / 4 - nYear / 100 + nYear / 400) % 7 + 1;
}

/* 获得某一特定日期从当前1月1日起过了多少天*/
unsigned short Get_ElapseDaysFromJan1st(sDateTime *pCurDate)
{
	unsigned short ret;
	sDateTime sDateTemp;
	sDateTemp = *pCurDate;
	sDateTemp.wMonth = 1;
	sDateTemp.wDay = 1;
	ret = (unsigned short)Calc_DiffTime(&sDateTemp, pCurDate, eTimeCalc_By_Days);
	return ret;
}

/* 获得某一特定日期的当年还剩下多少天*/
unsigned short Get_RemainDaysToEndOfThisYear(sDateTime *pCurDate)
{
	unsigned short ret;
	sDateTime sDateTemp;
	sDateTemp = *pCurDate;
	sDateTemp.wYear += 1;
	sDateTemp.wMonth = 1;
	sDateTemp.wDay = 1;
	ret = (unsigned short)Calc_DiffTime(&sDateTemp, pCurDate, eTimeCalc_By_Days);
	return ret;
}

/*返回从指定日期开始算起后或前多少天的日期*/
/*其中nDays为正为向后多少天,nDays为负为向前多少天*/
void Calc_NdaysDiff_Date(sDateTime *pResultDate, sDateTime *pStartDate, signed short nDays)
{
	time_t t;
	struct tm tm_start = {0}, * p_tm_result = NULL;

	tm_start.tm_year = pStartDate->wYear + 100;
	tm_start.tm_mon = pStartDate->wMonth - 1;
	tm_start.tm_mday = pStartDate->wDay;
	tm_start.tm_hour = pStartDate->wHour;
	tm_start.tm_min = pStartDate->wMinute;
	tm_start.tm_sec = pStartDate->wSecond;

	t = mktime(&tm_start);

	t += (nDays * 24 * 60 * 60);

	p_tm_result = gmtime(&t);

	pResultDate->wYear = p_tm_result->tm_year - 100 + 1900 - 2000;
	pResultDate->wMonth = p_tm_result->tm_mon + 1;
	pResultDate->wDay = p_tm_result->tm_mday;
	pResultDate->wHour = p_tm_result->tm_hour;
	pResultDate->wMinute = p_tm_result->tm_min;
	pResultDate->wSecond = p_tm_result->tm_sec;
}

