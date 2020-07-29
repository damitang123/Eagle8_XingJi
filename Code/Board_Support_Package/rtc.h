#ifndef	_RTC_H_
#define	_RTC_H_


typedef struct
{
	unsigned char wYear;
	unsigned char wMonth;
	unsigned char wDay;
	unsigned char wHour;
	unsigned char wMinute;
	unsigned char wSecond;
} __attribute__ ((packed))sDateTime; /*每一项为BCD码编码形式*/

extern sDateTime		g_sDateTime;
extern unsigned char 	g_nDayOfWeek;	/*星期*/


typedef enum
{
	eBCD_Format,
	eDEC_Format
} ERtcValueFormat;

void RTC_CS_Enable(void);
void RTC_CS_Disable(void);

void Rtc_Gpio_Init(void);
void Rtc_Init(void);
void Rtc_Get_Time(sDateTime *pDateTime, ERtcValueFormat valueFormat);
unsigned char *GetTimeString(sDateTime *pDateTime, unsigned char *strDest);
void Rtc_Set_Time(sDateTime *pDateTime);
unsigned char *MonthToString(unsigned char x);
const char *GetDayOfWeek(char weekday);
unsigned char  Update_System_Time(void);

void Task_RTC(void *p_arg);

#endif

