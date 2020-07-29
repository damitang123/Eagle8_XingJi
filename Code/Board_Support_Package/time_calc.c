#include "platform.h"
#include <time.h>
#include "rtc.h"
#include "time_calc.h"

#if 0
struct tm {
����int tm_sec; /* �� �C ȡֵ����Ϊ[0,59] */
����int tm_min; /* �� - ȡֵ����Ϊ[0,59] */
����int tm_hour; /* ʱ - ȡֵ����Ϊ[0,23] */
����int tm_mday; /* һ�����е����� - ȡֵ����Ϊ[1,31] */
����int tm_mon; /* �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11] */
����int tm_year; /* ��ݣ���ֵ����ʵ����ݼ�ȥ1900 */
����int tm_wday; /* ���� �C ȡֵ����Ϊ[0,6]������0���������죬1��������һ���Դ����� */
����int tm_yday; /* ��ÿ���1��1�տ�ʼ������ �C ȡֵ����Ϊ[0,365]������0����1��1�գ�1����1��2�գ��Դ����� */
����int tm_isdst; /* ����ʱ��ʶ����ʵ������ʱ��ʱ��tm_isdstΪ������ʵ������ʱ�Ľ���tm_isdstΪ0�����˽����ʱ��tm_isdst()Ϊ����*/
����};
#endif

/*�����������ڵ�ʱ���,�����������߷���������Сʱ����������*/
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

	if(eCalc_mode == eTimeCalc_By_Seconds)	/*��*/
	{
		return difftime(nEnd, nStart);
	}
	else if(eCalc_mode == eTimeCalc_By_Minites)	/*����*/
	{
		return difftime(nEnd, nStart) / 60;
	}
	else if(eCalc_mode == eTimeCalc_By_Hours)	/*Сʱ*/
	{
		return difftime(nEnd, nStart) / (3600);
	}
	else if(eCalc_mode == eTimeCalc_By_Days)	/*����*/
	{
		return difftime(nEnd, nStart) / 86400;
	}

	return difftime(nEnd, nStart);
}

/*��ȡĳ���ض����ں��ǰN�������ʱ��.*/
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

/*��ȡĳ��ʱ���UTCֵ.*/
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

/*��UTC�õ���Ӧ��ʱ��*/
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

/*�ж�һ�������Ƿ���Ч.*/
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

/*�ж�һ������Ƿ�Ϊ����*/
unsigned char Is_Leapyear(int nYear)
{
	if((nYear % 400 == 0 ) || (nYear % 4 == 0 && nYear % 100 != 0))
		return 1;
	else
		return 0;
}

/*���ĳ���������*/
unsigned char Get_Weekday(char nDay, char nMonth, char nYear)
{
	if(nMonth <= 2)
	{
		nMonth += 12;
		nYear--;
	}

	/*��һΪ1,����Ϊ7*/
	return (nDay + 2 * nMonth + 3 * (nMonth + 1) / 5 + nYear + nYear / 4 - nYear / 100 + nYear / 400) % 7 + 1;
}

/* ���ĳһ�ض����ڴӵ�ǰ1��1������˶�����*/
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

/* ���ĳһ�ض����ڵĵ��껹ʣ�¶�����*/
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

/*���ش�ָ�����ڿ�ʼ������ǰ�����������*/
/*����nDaysΪ��Ϊ��������,nDaysΪ��Ϊ��ǰ������*/
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

