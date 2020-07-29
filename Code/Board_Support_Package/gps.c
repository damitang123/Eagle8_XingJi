#include "platform.h"
#include "time_calc.h"
#include "data.h"
#include "app.h"

float g_fLongitude, g_fLatitude;
unsigned char g_bGPS_Location_Valid = 0xff;

static char s_aGps_Rec_Buffer[100];
static unsigned char s_nGps_Buffer_Cnt =0;
static bool s_bNewGPRMCDataReceived;
static unsigned char Hour, Min, Sec, Day, Month, Year, Longitude_Dir, Latitude_Dir;
static unsigned char s_bDateTimeValid = 0;
static unsigned char s_bGPS_Sync_SystemTime;

#define GPS_CALC_ORIGINAL	0
#define GPS_CALC_ATOF		1

/*
    GPS定位信息,北纬为正值,东经为正值.
*/

void Gps_Data_Receive(unsigned char nData)
{
	if(s_bNewGPRMCDataReceived == false)
	{
		if(nData == '$')
		{
			s_nGps_Buffer_Cnt = 0;
		}
		else if(nData == 0x0A)
		{
			if(strstr(s_aGps_Rec_Buffer,"RMC"))
			{
				s_bNewGPRMCDataReceived = true;
			}
		}
		else
		{
			s_aGps_Rec_Buffer[s_nGps_Buffer_Cnt++] = nData;
			if(s_nGps_Buffer_Cnt >= sizeof(s_aGps_Rec_Buffer))
			{
				s_nGps_Buffer_Cnt = 0;
			}
		}
	}
}

void USART2_IRQHandler(void)
{
	if(USART_GetFlagStatus(USART2, USART_IT_RXNE) == SET)
	{
		Gps_Data_Receive(USART_ReceiveData(USART2));
	}
}

static void Gps_Hw_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	/* Configure USARTx_Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USARTx_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	//串口接收中断开启
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); 

	/* Enable the USARTx */
	USART_Cmd(USART2, ENABLE);
}

void Gps_Init(void)
{
	g_bGPS_Location_Valid = 0xff;
	s_nGps_Buffer_Cnt = 0;
	s_bGPS_Sync_SystemTime = false;
	Gps_Hw_Init();
}



void GPRMC_Data_Parse(void)
{
	float l_g;
	unsigned char i, i1 = 0, uf = 0;
	for(i = 0; i < s_nGps_Buffer_Cnt; i++)
	{
		if(s_aGps_Rec_Buffer[i] == 0x2c)
		{
			i1++;
			uf = 0;
		}

		if(i1 == 1 && uf == 0)
		{
			if(s_aGps_Rec_Buffer[i + 1] >= '0' && s_aGps_Rec_Buffer[i + 1] <= '9')
			{
				Hour = (s_aGps_Rec_Buffer[i + 1] - '0') * 10 + (s_aGps_Rec_Buffer[i + 2] - '0');
				Min = (s_aGps_Rec_Buffer[i + 3] - '0') * 10 + (s_aGps_Rec_Buffer[i + 4] - '0');
				Sec = (s_aGps_Rec_Buffer[i + 5] - '0') * 10 + (s_aGps_Rec_Buffer[i + 6] - '0');
				s_bDateTimeValid = 1;
			}
			else
				s_bDateTimeValid = 0;
			i += 6;
			uf = 1;
		}
		else if(i1 == 2 && uf == 0)
		{
			if(s_aGps_Rec_Buffer[i + 1] == 'A')
				g_bGPS_Location_Valid = 1;
			else
			{
				g_bGPS_Location_Valid = 0;
				s_bDateTimeValid = 0;
			}
			i++;
			uf = 1;
		}
		else if(i1 == 3 && uf == 0)
		{
			if(s_aGps_Rec_Buffer[i + 1] == 0x2c)
				g_fLatitude = 0;
			else
			{
				#if	GPS_CALC_ORIGINAL
				g_fLatitude = ((s_aGps_Rec_Buffer[i + 1] - '0') * 10 + (s_aGps_Rec_Buffer[i + 2] - '0') +
				            (((((s_aGps_Rec_Buffer[i + 3] - '0') * 10) + (s_aGps_Rec_Buffer[i + 4] - '0')) / 0.6) * 0.01)) * 3600;

				l_g = (((s_aGps_Rec_Buffer[i + 6] - '0') * 1000) + ((s_aGps_Rec_Buffer[i + 7] - '0') * 100) +
				       ((s_aGps_Rec_Buffer[i + 8] - '0') * 10) + (s_aGps_Rec_Buffer[i + 9] - '0')) * 0.006;
				g_fLatitude += l_g;
				#elif GPS_CALC_ATOF
				l_g = l_g;
				g_fLatitude = atof((const char *)&s_aGps_Rec_Buffer[i + 1]);
				#endif

				i += 9;
			}
			uf = 1;
		}
		else if(i1 == 4 && uf == 0)
		{
			if(s_aGps_Rec_Buffer[i + 1] == 0x2c)	Latitude_Dir = 0;
			else if(s_aGps_Rec_Buffer[i + 1] == 'N') Latitude_Dir = 1;
			else if(s_aGps_Rec_Buffer[i + 1] == 'S') Latitude_Dir = 0;
			i++;
			uf = 1;
		}
		else if(i1 == 5 && uf == 0)
		{
			if(s_aGps_Rec_Buffer[i + 1] == 0x2c)
				g_fLongitude = 0;
			else
			{
				#if	GPS_CALC_ORIGINAL
				g_fLongitude = ((s_aGps_Rec_Buffer[i + 1] - '0') * 100 + (s_aGps_Rec_Buffer[i + 2] - '0') * 10 +
				             (s_aGps_Rec_Buffer[i + 3] - '0') + (((((s_aGps_Rec_Buffer[i + 4] - '0') * 10) + (s_aGps_Rec_Buffer[i + 5] - '0')) / 0.6) * 0.01)) * 3600;

				l_g = (((s_aGps_Rec_Buffer[i + 7] - '0') * 1000) + ((s_aGps_Rec_Buffer[i + 8] - '0') * 100) +
				       ((s_aGps_Rec_Buffer[i + 9] - '0') * 10) + (s_aGps_Rec_Buffer[i + 10] - '0')) * 0.006;
				g_fLongitude += l_g;
				#elif GPS_CALC_ATOF
				l_g = l_g;
				g_fLongitude = atof((const char *)&s_aGps_Rec_Buffer[i + 1]);
				#endif
				i += 10;
			}
			uf = 1;
		}
		else if(i1 == 6 && uf == 0)
		{
			if(s_aGps_Rec_Buffer[i + 1] == 0x2c)	Longitude_Dir = 0;
			else if(s_aGps_Rec_Buffer[i + 1] == 'E') Longitude_Dir = 1;
			else if(s_aGps_Rec_Buffer[i + 1] == 'W') Longitude_Dir = 0;
			i++;
			uf = 1;
		}
		else if(i1 == 7 && uf == 0)
		{
			uf = 1;
		}
		else if(i1 == 8 && uf == 0)
		{
			uf = 1;
		}
		else if(i1 == 9 && uf == 0)
		{
			if(s_aGps_Rec_Buffer[i + 1] >= '0' && s_aGps_Rec_Buffer[i + 1] <= 0x39)
			{
				Day = (s_aGps_Rec_Buffer[i + 1] - '0') * 10 + (s_aGps_Rec_Buffer[i + 2] - '0');
				Month = (s_aGps_Rec_Buffer[i + 3] - '0') * 10 + (s_aGps_Rec_Buffer[i + 4] - '0');
				Year = (s_aGps_Rec_Buffer[i + 5] - '0') * 10 + (s_aGps_Rec_Buffer[i + 6] - '0');
				s_bDateTimeValid = 1;
			}
			else
				s_bDateTimeValid = 0;
			i += 6;
			uf = 1;
		}
	}

}

unsigned char GPS_Is_Location_Valid(void)
{
	return g_bGPS_Location_Valid;
}

unsigned char Get_GPS_Location_Value(float *pLongitude, float *pLatitude)
{
	unsigned char ret = 0;
	if(g_bGPS_Location_Valid)
	{
		ret = 1;
		*pLongitude = Longitude_Dir ? g_fLongitude : -g_fLongitude;
		*pLatitude = Latitude_Dir ? g_fLatitude : -g_fLatitude;
	}
	else
	{
		*pLongitude = 0;
		*pLatitude = 0;
	}
	return ret;
}

const char aFormatString[] = {0x25, 0x64, 0xC2, 0xB0, 0x25, 0x64, 0x27, 0x25, 0x64, 0x2E, 0x25, 0x64, 0x22, 0x2C, 0x25, 0x63, 0x00}; /*%d°%d'%d.%d",%c*/

unsigned char Get_GPS_Location_String(char *strLongitude, char *strLatitude)
{
	unsigned char ret = 0;

	if(g_bGPS_Location_Valid)
	{
	#if	GPS_CALC_ORIGINAL
		long a, b, c, d;
		a = g_fLongitude / 3600;
		b = g_fLongitude * 100;
		b = (b % 360000) / 6000;
		c = g_fLongitude * 100;
		c = ((c % 360000) % 6000) / 100;
		d = g_fLongitude * 100;
		d = ((d % 360000) % 6000) % 1000 % 100;
		sprintf(strLongitude, aFormatString, a, b, c, d, Longitude_Dir ? 'E' : 'W');
		a = g_fLatitude / 3600;
		b = g_fLatitude * 100;
		b = (b % 360000) / 6000;
		c = g_fLatitude * 100;
		c = ((c % 360000) % 6000) / 100;
		d = g_fLatitude * 100;
		d = ((d % 360000) % 6000) % 1000 % 100;
		sprintf(strLatitude, aFormatString, a, b, c, d, Latitude_Dir ? 'N' : 'S');
		ret = 1;
	#elif GPS_CALC_ATOF
		float fTemp;
		fTemp = (g_fLongitude - (int)g_fLongitude) * 60;
		sprintf(strLongitude, aFormatString,
		        (int)g_fLongitude / 100, (int)g_fLongitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100),
		        Longitude_Dir ? 'E' : 'W');
		fTemp = (g_fLatitude - (int)g_fLatitude) * 60;
		sprintf(strLatitude, aFormatString,
		        (int)g_fLatitude / 100, (int)g_fLatitude % 100, (int)fTemp, (int)((fTemp - (int)fTemp) * 100),
		        Latitude_Dir ? 'N' : 'S');
		ret = 1;
	#endif
	}
	return ret;
}

void Gps_Data_Parse(void)
{
	if(s_bNewGPRMCDataReceived)
	{
		s_bNewGPRMCDataReceived = false;
		GPRMC_Data_Parse();

		/*GPS 同步时间*/
		if(s_bGPS_Sync_SystemTime != 1)
		{
			if(g_bGPS_Location_Valid == 1 && s_bDateTimeValid)
			{
				unsigned long nUTCTime;
				sDateTime sDT, sDateTemp;

				sDT.wYear = Year;
				sDT.wMonth = Month;
				sDT.wDay = Day;
				sDT.wHour = Hour;
				sDT.wMinute = Min;
				sDT.wSecond = Sec;

				nUTCTime = Get_UTC_Value(&sDT);
				nUTCTime += (int)g_sSystemParam.nTimeZone * 3600;
				sDateTemp = Get_LocalTime_From_UTC(nUTCTime);

				if(g_bSpiHardwareBusBusy == false)
				{
					Rtc_Gpio_Init();
					Rtc_Set_Time(&sDateTemp);

					s_bGPS_Sync_SystemTime = 1;
				}
			}
		}
	}
}



