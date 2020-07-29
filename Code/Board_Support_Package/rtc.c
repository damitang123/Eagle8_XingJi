#include "platform.h"
#include "rtc.h"
#include "Windows.h"
#include "app.h"
#include "ads8320.h"

#define RTC_USE_PCF8563TS	1

sDateTime	g_sDateTime;
unsigned char g_nDayOfWeek;

#define	BCD_TO_DEC(x)				((x>>4)*10+(x&0x0f))

#if RTC_USE_PCF8563TS

#include "i2c.h"

#define PCF8563TS_I2C_ADDR 		0xA2

void Rtc_Gpio_Init(void)
{
	I2C_Hal_Init();
}

static void Rtc_Write(unsigned char nAddr, unsigned char nValue, ERtcValueFormat eValueFormat)
{
	/*数字转BCD码,如果传入的是DEC数字，在配置时间时要写转换成BCD码*/
	/*参数value 一直都是DEC 格式*/
	if(eValueFormat == eBCD_Format)
		nValue = ((nValue / 10) << 4) + ((nValue % 10) & 0x0F);

	I2C_Single_Write(PCF8563TS_I2C_ADDR, nAddr, nValue);
}

void Rtc_Init(void)
{
	Rtc_Gpio_Init();

	I2C_Single_Write(PCF8563TS_I2C_ADDR, 0x00, 0x00);
	I2C_Single_Write(PCF8563TS_I2C_ADDR, 0x01, 0x00);
}

void Rtc_Get_Time(sDateTime *pDateTime, ERtcValueFormat eValueFormat)
{
	unsigned char aTemp[7];
	I2C_Block_Read(PCF8563TS_I2C_ADDR, aTemp, 0x02, 7);
	pDateTime->wYear = aTemp[0x06];
	pDateTime->wMonth = aTemp[0x05] & 0x1f;
	pDateTime->wDay = aTemp[0x03] & 0x3f;

	pDateTime->wHour = aTemp[0x02] & 0x3f;
	pDateTime->wMinute = aTemp[0x01] & 0x7f;
	pDateTime->wSecond = aTemp[0x00] & 0x7f;

	if(eValueFormat == eBCD_Format) return;

	pDateTime->wYear = BCD_TO_DEC(pDateTime->wYear);
	pDateTime->wMonth = BCD_TO_DEC(pDateTime->wMonth);
	pDateTime->wDay = BCD_TO_DEC(pDateTime->wDay);

	pDateTime->wHour = BCD_TO_DEC(pDateTime->wHour);
	pDateTime->wMinute = BCD_TO_DEC(pDateTime->wMinute);
	pDateTime->wSecond = BCD_TO_DEC(pDateTime->wSecond);

	if(pDateTime->wHour >23 && pDateTime->wHour <= 33)
	{
		pDateTime->wHour -= 10;
	}
}


void Rtc_Set_Time(sDateTime *pDateTime)
{
	if(pDateTime == NULL)	return;

	Rtc_Write(8, pDateTime->wYear, eBCD_Format);
	Rtc_Write(7, pDateTime->wMonth, eBCD_Format);
	Rtc_Write(5, pDateTime->wDay, eBCD_Format);

	Rtc_Write(4, pDateTime->wHour, eBCD_Format);
	Rtc_Write(3, pDateTime->wMinute, eBCD_Format);
	Rtc_Write(2, pDateTime->wSecond, eBCD_Format);
}

#endif


#if RTC_USE_RS5C348

#include "spi_sim_driver.h"

#define RS5C348_CMD_W(addr)		(((addr) << 4) | 0x08)	/*单字节写*/
#define RS5C348_CMD_R(addr)			(((addr) << 4) | 0x0c)	/*单字节读*/

#define RS5C348_CSHigh()				(GPIOD->BSRR = GPIO_Pin_12)
#define RS5C348_CSLow()				(GPIOD->BRR = GPIO_Pin_12)

static void Rtc_CS_Init(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	RS5C348_CSLow();
}

void Rtc_Gpio_Init(void)
{
	Rtc_CS_Init();
	Spi_Sim_Init();
}

static unsigned char Rtc_Read(unsigned char nAddr)
{
	unsigned char nValue;
	RS5C348_CSHigh();
	Spi_Sim_Send_Single_Byte(RS5C348_CMD_R(nAddr));
	nValue = Spi_Sim_Get_Single_Byte();
	RS5C348_CSLow();
	return nValue;
}

static void Rtc_Write(unsigned char nAddr, unsigned char nValue, ERtcValueFormat eValueFormat)
{
	/*数字转BCD码,如果传入的是DEC数字，在配置时间时要写转换成BCD码*/
	/*参数value 一直都是DEC 格式*/
	if(eValueFormat == eBCD_Format)
		nValue = ((nValue / 10) << 4) + ((nValue % 10) & 0x0F);

	RS5C348_CSHigh();
	Spi_Sim_Send_Single_Byte(RS5C348_CMD_W(nAddr));
	Spi_Sim_Send_Single_Byte(nValue);
	RS5C348_CSLow();
}

void Rtc_Init(void)
{
	Rtc_Gpio_Init();

	Rtc_Write(0x0f, 0x08, eDEC_Format);
	delay_ms(50);
	Rtc_Write(0x0e, 0x30, eDEC_Format);
}


void Rtc_Get_Time(sDateTime *pDateTime, ERtcValueFormat eValueFormat)
{
	pDateTime->wYear = Rtc_Read(0x06);
	pDateTime->wYear = Rtc_Read(0x06);
	pDateTime->wMonth = Rtc_Read(0x05) & 0x1f;
	pDateTime->wDay = Rtc_Read(0x04) & 0x3f;

	pDateTime->wHour = Rtc_Read(0x02) & 0x3f;
	pDateTime->wMinute = Rtc_Read(0x01) & 0x7f;
	pDateTime->wSecond = Rtc_Read(0x00) & 0x7f;

	if(eValueFormat == eBCD_Format) return;

	pDateTime->wYear = BCD_TO_DEC(pDateTime->wYear);
	pDateTime->wMonth = BCD_TO_DEC(pDateTime->wMonth);
	pDateTime->wDay = BCD_TO_DEC(pDateTime->wDay);

	pDateTime->wHour = BCD_TO_DEC(pDateTime->wHour);
	pDateTime->wMinute = BCD_TO_DEC(pDateTime->wMinute);
	pDateTime->wSecond = BCD_TO_DEC(pDateTime->wSecond);

	if(pDateTime->wHour >23 && pDateTime->wHour <= 33)
	{
		pDateTime->wHour -= 10;
	}
}


void Rtc_Set_Time(sDateTime *pDateTime)
{
	if(pDateTime == NULL)	return;

	Rtc_Write(6, pDateTime->wYear, eBCD_Format);
	Rtc_Write(6, pDateTime->wYear, eBCD_Format);
	Rtc_Write(5, pDateTime->wMonth, eBCD_Format);
	Rtc_Write(4, pDateTime->wDay, eBCD_Format);

	Rtc_Write(2, pDateTime->wHour, eBCD_Format);
	Rtc_Write(1, pDateTime->wMinute, eBCD_Format);
	Rtc_Write(0, pDateTime->wSecond, eBCD_Format);
}

#endif

char Calculate_Weekday(char day, char month, char year) //日、月、年
{
	if(month <= 2)
	{
		month += 12;
		year--;
	}

	/*周一为1,周日为7*/
	return (day + 2 * month + 3 * (month + 1) / 5 + year + year / 4 - year / 100 + year / 400) % 7 + 1;
}

unsigned char *MonthToString(unsigned char x)
{
	switch(x)
	{
		case 1:
			return "Jan";
		case 2:
			return "Feb";
		case 3:
			return "Mar";
		case 4:
			return "Apr";
		case 5:
			return "May";
		case 6:
			return "Jun";
		case 7:
			return "July";
		case 8:
			return "Aug";
		case 9:
			return "Sep";
		case 10:
			return "Oct";
		case 11:
			return "Nov";
		case 12:
			return "Dec";
	}

	return "Jan";
}


unsigned char *GetTimeString(sDateTime *pDateTime, unsigned char *strDest)
{
	sprintf((char *)strDest, "%02d/%02d/%04d %02d:%02d:%02d",
	        pDateTime->wDay,
	        pDateTime->wMonth,
	        pDateTime->wYear + 2000,
	        pDateTime->wHour,
	        pDateTime->wMinute,
	        pDateTime->wSecond);

	return	strDest;
}

const char *GetDayOfWeek(char weekday)
{
	switch(weekday)
	{
		case 1:
			return _Monday;
		case 2:
			return _Tuesday;
		case 3:
			return _Wednesday;
		case 4:
			return _Thursday;
		case 5:
			return _Friday;
		case 6:
			return _Saturday;
		case 7:
			return _Sunday;
		default:
			return _Sunday;
	}
}

unsigned char  Update_System_Time(void)
{
	Rtc_Init();
	Rtc_Get_Time(&g_sDateTime, eDEC_Format);
	g_nDayOfWeek = Calculate_Weekday(g_sDateTime.wDay, g_sDateTime.wMonth, g_sDateTime.wYear);
	return 1;
}
