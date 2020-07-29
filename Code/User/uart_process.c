#include "platform.h"
#include <stdarg.h>
#include "uart_process.h"
#include "delay.h"
#include "uart.h"
#include "rtc.h"
#include "spi_flash.h"
#include "GUI.h"
#include "parameter.h"
#include "data.h"
#include "system_misc.h"
#include "CustomDisplayDriver.h"
#include "voice.h"
#include "app.h"
#include "gprs_user.h"
#include "dfu_config.h"

#define UART_BUFFER_SIZE		600
#define USB_CMD_PROCESS_BUFFER_SIZE	500
#define UART_CMD_TIMOUT		500	/*单位ms*/

typedef enum
{
	eUSBPhaseHead0,
	eUSBPhaseHead1,
	eUSBPhaseCmd,
	eUSBPhaseDataLength,
	eUSBPhaseData,
	eUSBPhaseCrc,
	eUSBPhaseEnd,
} eUSBRecievePhase;

typedef struct
{
	unsigned char nHead1;
	unsigned char nHead2;
	unsigned char nCmd;
	unsigned short nLength;
	unsigned char aData[1030];
} __attribute__ ((packed))sUSBTxPacket;


static volatile unsigned short s_nUartInCnt = 0, s_nUartOutCnt = 0;

static unsigned short s_nCmdProcessBufferCnt;
static unsigned char s_aCmdProcessBuffer[USB_CMD_PROCESS_BUFFER_SIZE];

unsigned short s_ValidCmdIndex = 0;
unsigned char g_aUartCmdRevBuffer[UART_BUFFER_SIZE];
sUSBTxPacket s_sUARTAckPacket;
eUSBRecievePhase s_eUSBRecievePhase = eUSBPhaseHead0;

static volatile bool s_bStartRevCmd = false;
static volatile unsigned long s_nStartRevCmdTime = 0;

/*此函数要被中断调用*/
void Receive_Data_From_UART1(unsigned char RevData)
{
	g_aUartCmdRevBuffer[s_nUartInCnt] = RevData;
	if(++s_nUartInCnt >= UART_BUFFER_SIZE)
	{
		s_nUartInCnt = 0;
	}
}

/*封装并发送数据*/
static void Uart_Transmit_Data(unsigned char *aArray, unsigned short nSize)
{
	unsigned short i;
	for(i = 0; i < nSize; i++)
	{
		while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
		USART_SendData(USART1, aArray[i]);
	}
}

void Uart_Packet_And_Send(unsigned char nCmd, unsigned char  *pData, unsigned short nLen)
{
	unsigned short i = 0,nCheckSum = 0;
	s_sUARTAckPacket.nHead1 = 0xBC;
	s_sUARTAckPacket.nHead2 = 0xFD;
	s_sUARTAckPacket.nCmd = nCmd;
	s_sUARTAckPacket.nLength = nLen;
	while(i< nLen)
	{
		nCheckSum += pData[i];
		s_sUARTAckPacket.aData[i] = pData[i];
		i++;
	}
	s_sUARTAckPacket.aData[nLen] = nCheckSum;
	s_sUARTAckPacket.aData[nLen + 1] = 0xAE;

	Uart_Transmit_Data((unsigned char *)&s_sUARTAckPacket, 7 + nLen);
}

static unsigned char Get_Uart_Data(void)
{
	unsigned char nTemp;
	nTemp = g_aUartCmdRevBuffer[s_nUartOutCnt];
	if(++s_nUartOutCnt >= UART_BUFFER_SIZE)
	{
		s_nUartOutCnt = 0;
	}
	return nTemp;
}

static void  Uart_Recieve_Error_Handle(int nLine)
{
	s_bStartRevCmd = false;
	s_eUSBRecievePhase = eUSBPhaseHead0;

	TRACE_PRINTF("Uart_Recieve_Error_Handle( Line = %d)\r\n",nLine);
}

/***********************************************************************************************************************/

void User_Debug_Printf(const char *fmt, ...)
{
	if(g_sSystemParam.bEnableTraceFunction)
	{
		int nLen;
		static int nLogIndex = 0;
		static char aDebugBuff[300];
		va_list ap;
		sprintf(aDebugBuff,"[%08d] ",nLogIndex++);
		va_start(ap, fmt);
		nLen = vsprintf(&aDebugBuff[11], fmt, ap);
		va_end(ap);
		Uart_Packet_And_Send(ACK_USB_LOG, (unsigned char *)aDebugBuff, nLen + 11);
//		Uart_Transmit_Data((unsigned char *)aDebugBuff,nLen + 11);
	}	
}

static void Uart_Cmd_Link(unsigned short nLen ,unsigned char *pData)
{
	if(nLen == 2)
	{
		unsigned short nTemp;
		nTemp = pData[0] + (((unsigned short)pData[1]) << 8);
	#if EAGLE_1
		if(nTemp == 0xA11A)
		{
			unsigned char aData[2];
			aData[0] = 0x1A;
			aData[1] = 0x1A;
			Uart_Packet_And_Send(ACK_LINK_TICK, aData, 2);
		}
	#elif EAGLE_8
		if(nTemp == 0xA81A)
		{
			unsigned char aData[2];
			aData[0] = 0x1A;
			aData[1] = 0x8A;
			Uart_Packet_And_Send(ACK_LINK_TICK, aData, 2);
		}
	#endif
	}
}

static void Uart_Cmd_Set_Time(unsigned short nLen,unsigned char *pData)
{
	if(nLen == 6)
	{
		sDateTime sDateTimeTemp;
		memcpy(&sDateTimeTemp, pData, 6);
		if(g_bSpiHardwareBusBusy == false)
		{
			Rtc_Set_Time(&sDateTimeTemp);
		}
		Uart_Packet_And_Send(ACK_SET_SYSTEM_TIME, NULL, 0);
	}
}

static void Uart_Cmd_Set_Lcd_Brightness(unsigned short nLen,unsigned char *pData)
{
	if(nLen == 1)
	{
		if(*pData > 100)
		{
			*pData = 100;
		}
		Lcd_Back_Light_Pwm_Pulse_Config(Get_Actual_LcdBackLight_Percent(*pData));
		Uart_Packet_And_Send(ACK_SET_LCD_BRIGHTNESS, NULL, 0);
	}
}

static void Uart_Cmd_Set_Voice_Volume(unsigned short nLen,unsigned char *pData)
{
	if(nLen == 1)
	{
		if(*pData > 100)
		{
			*pData = 100;
		}
		Voc_Cmd(Get_Actual_Volume_Config(*pData));
		Uart_Packet_And_Send(ACK_SET_VOICE_VOLUME, NULL, 0);
	}
}

static void Uart_Cmd_Get_Total_Inhale_Time(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_GET_TOTAL_INHALE_TIIME_COUNTER, (unsigned char *)&g_fTotalInhaleTimeCounter, sizeof(g_fTotalInhaleTimeCounter));
}
static void Uart_Cmd_Clear_Total_Inhale_Time(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_TOTAL_INHALE_TIME * FLASH_SECTOR_SIZE);
	Data_Total_InhaleTime_Counter_Load(&g_fTotalInhaleTimeCounter);
	Uart_Packet_And_Send(ACK_CLEAR_TOTAL_INHALE_TIIME_COUNTER, NULL, 0);
}

static void Uart_Cmd_Get_Total_Test_Counter(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_GET_TOTAL_TEST_COUNTER, (unsigned char *)&g_nTotalTestCounter, sizeof(g_nTotalTestCounter));
}
static void Uart_Cmd_Clear_Total_Test_Counter(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_TOTAL_TEST_COUNTER * FLASH_SECTOR_SIZE);
	Data_Total_Test_Counter_Load(&g_nTotalTestCounter);
	Uart_Packet_And_Send(ACK_CLEAR_TOTAL_TEST_COUNTER, NULL, 0);
}


/*测试参数****************************************************************************************/
static void Uart_Cmd_Get_Test_Parameter(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_GET_TEST_PARAM, (unsigned char *)&g_sTestParam, sizeof(sTestParam));
}
static void Uart_Cmd_Set_Test_Parameter(unsigned short nLen,unsigned char *pData)
{
	if(nLen == sizeof(sTestParam))
	{
		Data_Test_Parameter_Save((sTestParam*)pData);
		Data_Test_Parameter_Load(&g_sTestParam);
		Uart_Packet_And_Send(ACK_SET_TEST_PARAM, (unsigned char *)&g_sTestParam, sizeof(sTestParam));
	}
}
static void Uart_Cmd_Recover_Default_Test_Parameter(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_TEST_PARAM * FLASH_SECTOR_SIZE);
	Data_Test_Parameter_Load(&g_sTestParam);
	Uart_Packet_And_Send(ACK_RECOVER_DEFAULT_TEST_PARAM, NULL, 0);
}
/******************************************************************************************************/


/*系统参数****************************************************************************************/
static void Uart_Cmd_Get_System_Parameter(unsigned short nLen,unsigned char *pData)
{
	sSystemParam sSystemParamTemp;
	sSystemParamTemp = g_sSystemParam;
	memset(sSystemParamTemp.aAdminPassword, 0, sizeof(sSystemParamTemp.aAdminPassword));
	Uart_Packet_And_Send(ACK_GET_SYSTEM_PARAM, (unsigned char *)&sSystemParamTemp, sizeof(sSystemParam));
}
static void Uart_Cmd_Set_System_Parameter(unsigned short nLen,unsigned char *pData)
{
	unsigned char nFlowVolumeAdjustCoefficient = 0;
	if(nLen == sizeof(sSystemParam))
	{
		sSystemParam sSystemParamTemp;
		sSystemParamTemp = *(sSystemParam*)pData;
		if(strlen(sSystemParamTemp.aAdminPassword) == 0)
		{
			memcpy(sSystemParamTemp.aAdminPassword, g_sSystemParam.aAdminPassword, sizeof(sSystemParamTemp.aAdminPassword));
		}
		sSystemParamTemp.nFlowVolumeAdjustCoefficient = nFlowVolumeAdjustCoefficient;
		Data_System_Parameter_Save(&sSystemParamTemp);
		Data_System_Parameter_Load(&g_sSystemParam);
		Uart_Packet_And_Send(ACK_SET_SYSTEM_PARAM, (unsigned char *)&g_sSystemParam, sizeof(sSystemParam));
	}
}
static void Uart_Cmd_Recover_Default_System_Parameter(unsigned short nLen,unsigned char *pData)
{
	sSystemParam s;
	s = g_sSystemParam;
	Flash_Sector_Erase(FLASH_SECTOR_SYSTEM_PARAM * FLASH_SECTOR_SIZE);
	Data_System_Parameter_Load(&g_sSystemParam);
	memcpy(g_sSystemParam.aIMEI, s.aIMEI, sizeof(s.aIMEI));
	Data_System_Parameter_Save(&g_sSystemParam);
	Uart_Packet_And_Send(ACK_RECOVER_DEFAULT_SYSTEM_PARAM, NULL, 0);
}
/******************************************************************************************************/


/*标定参数****************************************************************************************/
static void Uart_Cmd_Get_Alc_Calibration(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_GET_ALC_CALIBRATION_PARAM, (unsigned char *)&g_sAlcCalibrationParam, sizeof(sAlcCalibrationParam));
}
static void Uart_Cmd_Set_Alc_Calibration(unsigned short nLen,unsigned char *pData)
{	sAlcCalibrationParam *p;
	if(nLen == sizeof(sAlcCalibrationParam))
	{
		p = (sAlcCalibrationParam*)pData;
		p->nIntergralLowPoint = (unsigned long)((float)g_sAlcCalibrationParam.nIntergralLowPoint * p->fLRealTestAlcoholValue / p->fLStandardAlcoholValue);
		p->nIntergralHighPoint = (unsigned long)((float)g_sAlcCalibrationParam.nIntergralHighPoint * p->fHRealTestAlcoholValue / p->fHStandardAlcoholValue);
		p->fLRealTestAlcoholValue = p->fLStandardAlcoholValue;
		p->fHRealTestAlcoholValue = p->fHStandardAlcoholValue;
		p->nUserIntergralLowPoint = p->nIntergralLowPoint;
		p->nUserIntergralHighPoint = p->nIntergralHighPoint;
		p->fLStandardAlcoholValue = 20;
		p->fHStandardAlcoholValue = 80;
		p->fLRealTestAlcoholValue = 20;
		p->fHRealTestAlcoholValue = 80;
		p->fPeekStandardAlcoholValue = 40;
		p->fPeekRealTestAlcoholValue = 40;
		Data_AlcCalibration_Parameter_Save(p);
		Data_AlcCalibration_Parameter_Load(&g_sAlcCalibrationParam);
		/*系数还原*/
		g_sTestParam.nAdjustCoefficientIntegral = 100;
		Data_Test_Parameter_Save(&g_sTestParam);
		Data_Test_Parameter_Load(&g_sTestParam);
		Uart_Packet_And_Send(ACK_SET_ALC_CALIBRATION_PARAM, (unsigned char *)&g_sAlcCalibrationParam, sizeof(sAlcCalibrationParam));
	}
}
static void Uart_Cmd_Recover_Default_Alc_Calibration(unsigned short nLen,unsigned char *pData)
{
	unsigned long nZeroIntergralValue;
	nZeroIntergralValue = g_sAlcCalibrationParam.nZeroIntergralValue;
	/*系数还原*/
	g_sTestParam.nAdjustCoefficientIntegral = 100;
	Data_Test_Parameter_Save(&g_sTestParam);
	Data_Test_Parameter_Load(&g_sTestParam);
	Flash_Sector_Erase(FLASH_SECTOR_ALC_CALIBRATION * FLASH_SECTOR_SIZE);
	Data_AlcCalibration_Parameter_Load(&g_sAlcCalibrationParam);
	g_sAlcCalibrationParam.nZeroIntergralValue = nZeroIntergralValue;
	Data_AlcCalibration_Parameter_Save(&g_sAlcCalibrationParam);
	Uart_Packet_And_Send(ACK_RECOVER_DEFAULT_ALC_CALIBRATION_PARAM, NULL, 0);
}
/******************************************************************************************************/


/*温补参数****************************************************************************************/
static void Uart_Cmd_Get_Temperature_Calibration(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_GET_TEMP_CALIBRATION_PARAM, (unsigned char *)&g_sTemperatureCalibrationParam, sizeof(sTemperatureCalibrationParam));
}
static void Uart_Cmd_Set_Temperature_Calibration(unsigned short nLen,unsigned char *pData)
{	sTemperatureCalibrationParam *p;
	if(nLen == sizeof(sTemperatureCalibrationParam))
	{
		p = (sTemperatureCalibrationParam *)pData;
		Data_Temperature_Calibration_Parameter_Save(p);
		Data_Temperature_Calibration_Parameter_Load(&g_sTemperatureCalibrationParam);
		Uart_Packet_And_Send(ACK_SET_TEMP_CALIBRATION_PARAM, (unsigned char *)&g_sTemperatureCalibrationParam, sizeof(sTemperatureCalibrationParam));
	}
}
static void Uart_Cmd_Recover_Default_Temperature_Calibration(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_TEMPERATURE_CAL * FLASH_SECTOR_SIZE);
	Data_Temperature_Calibration_Parameter_Load(&g_sTemperatureCalibrationParam);
	Uart_Packet_And_Send(ACK_RECOVER_DEFAULT_TEMP_CALIBRATION_PARAM, NULL, 0);
}
/******************************************************************************************************/


/*ID参数****************************************************************************************/
static void Uart_Cmd_Get_DeviceID(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_GET_DEVICE_ID_PARAM, (unsigned char *)&g_sDeviceIDConfig, sizeof(sDeviceIDConfig));
}
static void Uart_Cmd_Set_DeviceID(unsigned short nLen,unsigned char *pData)
{
	if(nLen == sizeof(sDeviceIDConfig))
	{
		Data_DeviceID_Config_Save((sDeviceIDConfig*)pData);
		Data_DeviceID_Config_Load(&g_sDeviceIDConfig);
		Uart_Packet_And_Send(ACK_SET_DEVICE_ID_PARAM, (unsigned char *)&g_sDeviceIDConfig, sizeof(sDeviceIDConfig));
	}
}
static void Uart_Cmd_Recover_Default_DeviceID(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_DEVICE_ID * FLASH_SECTOR_SIZE);
	Data_DeviceID_Config_Load(&g_sDeviceIDConfig);
	Uart_Packet_And_Send(ACK_RECOVER_DEFAULT_DEVICE_ID_PARAM, NULL, 0);
}
/******************************************************************************************************/


/*服务器参数****************************************************************************************/
static void Uart_Cmd_Get_Server_Parameter(unsigned short nLen,unsigned char *pData)
{	
	g_sServerParam.nVersion = 0xF1;//added by lxl 20181011
	Uart_Packet_And_Send(ACK_GET_SERVER_PARAM, (unsigned char *)&g_sServerParam, sizeof(sServerParam));
}
static void Uart_Cmd_Set_Server_Parameter(unsigned short nLen,unsigned char *pData)
{
	//if(nLen == sizeof(sServerParam)) //deleted  20181106 不然无法进行服务器参数的修改 需要找郝鹏确认 
	{
		Data_Server_Parameter_Save((sServerParam*)pData);
		Data_Server_Parameter_Load(&g_sServerParam);
		g_sServerParam.nVersion = 0xF1;//added by lxl 20181011
		Uart_Packet_And_Send(ACK_SET_SERVER_PARAM, (unsigned char *)&g_sServerParam, sizeof(sServerParam));
	}
}
static void Uart_Cmd_Recover_Server_Parameter(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_SERVER_PARAM * FLASH_SECTOR_SIZE);
	Data_Server_Parameter_Load(&g_sServerParam);
	Uart_Packet_And_Send(ACK_RECOVER_DEFAULT_SERVER_PARAM, NULL, 0);
}
/******************************************************************************************************/


/*设备使用者相关参数****************************************************************************************/
static void Uart_Cmd_Get_User_Device_Infor(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_GET_USER_DEVICE_INFOR, (unsigned char *)&g_sDeviceUserInfo, sizeof(sDeviceUserInfor));
}
static void Uart_Cmd_Set_User_Device_Infor(unsigned short nLen,unsigned char *pData)
{
	if(nLen == sizeof(sDeviceUserInfor))
	{
		Data_Device_UserInfor_Save((sDeviceUserInfor*)pData);
		Data_Device_UserInfor_Load(&g_sDeviceUserInfo);
		Uart_Packet_And_Send(ACK_SET_USER_DEVICE_INFOR, (unsigned char *)&g_sDeviceUserInfo, sizeof(sDeviceUserInfor));
	}
}
static void Uart_Cmd_Recover_User_Device_Infor(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_DEVICE_USER_INFOR * FLASH_SECTOR_SIZE);
	Data_Device_UserInfor_Load(&g_sDeviceUserInfo);
	Uart_Packet_And_Send(ACK_RECOVER_USER_DEVICE_INFOR, NULL, 0);
}
/******************************************************************************************************/

static void Uart_Delete_UserLogin_Information(unsigned short nLen,unsigned char *pData)
{
	Flash_Sector_Erase(FLASH_SECTOR_POLICE_USERS_INFOR * FLASH_SECTOR_SIZE);
	Flash_Sector_Erase(FLASH_SECTOR_POLICE_LAST_USER * FLASH_SECTOR_SIZE);
	Uart_Packet_And_Send(ACK_DEL_USER_LOGIN_INFORMATION, NULL, 0);
}

/*记录****************************************************************************************/
static void Uart_Cmd_Get_Record_Counter(unsigned short nLen,unsigned char *pData)
{	
	Uart_Packet_And_Send(ACK_GET_RECORDS_COUNTER,(unsigned char *)&g_sRecordInforMap.nTotalRecordsCounter, sizeof(g_sRecordInforMap.nTotalRecordsCounter));
}
static void Uart_Cmd_Record_Trans_Begin(unsigned short nLen,unsigned char *pData)
{	
	unsigned long nRecordIndex = 0;
	if(nLen <= 4 )
	{
		memcpy((unsigned char *)&nRecordIndex,pData,nLen);
		if(nRecordIndex)
		{
			nRecordIndex -= 1;
			Uart_Packet_And_Send(ACK_EXPORT_RECORDS_BEGIN,NULL, 0);
			g_bUartStartTransRecords = true;
			g_nUartCurrentTransRecordIndex = nRecordIndex;
		}
		else
		{
			Uart_Packet_And_Send(ACK_EXPORT_RECORDS_BEGIN,NULL, 0);
			g_bUartStartTransRecords = true;
			g_nUartCurrentTransRecordIndex = 0;
		}
	}
}
static void Uart_Cmd_Record_Trans_Finish(void)
{	
	Uart_Packet_And_Send(ACK_EXPORT_RECORDS_FINISH,NULL, 0);
	g_bUartStartTransRecords = false;
	g_nUartCurrentTransRecordIndex = 0;
}
static void Uart_Cmd_Break_Record_Trans(void)
{	
	Uart_Packet_And_Send(ACK_BREAK_RECORDS_EXPORT_RECORDS,NULL, 0);
	g_bUartStartTransRecords = false;
	g_nUartCurrentTransRecordIndex = 0;
}
static void Uart_Cmd_Clear_All_Records(unsigned short nLen,unsigned char *pData)
{	
	Data_Records_Clear();
	g_bUartStartTransRecords = false;
	g_nUartCurrentTransRecordIndex = 0;
	Uart_Packet_And_Send(ACK_CLEAR_ALL_RECORDS,NULL, 0);
}
static void Uart_Cmd_Get_Record_By_ID(unsigned short nLen,unsigned char *pData)
{	
	unsigned long nRecordIndex = 0;
	if(nLen <= 4 )
	{
		memcpy((unsigned char *)&nRecordIndex,pData,nLen);
		if(nRecordIndex)
		{
			nRecordIndex -= 1;
			Data_Record_Load_ByID(nRecordIndex, (sRecord *)s_sUARTAckPacket.aData);
			Uart_Packet_And_Send(ACK_EXPORT_RECORD_BY_ID,s_sUARTAckPacket.aData, sizeof(sRecord));
		}
	}
}
static void Uart_Cmd_Write_Record_By_ID(unsigned short nLen,unsigned char *pData)
{	
	unsigned long nRecordIndex = 0;
	if(nLen == sizeof(sRecord))
	{
		nRecordIndex = ((sRecord *)pData)->nRecordIndex;
		/*必须是连续写入才可以*/
		if(nRecordIndex == g_nNewRecordID)
		{
			Data_Record_Save_Sub((sRecord *)pData);
			Data_Record_Load_Last(&g_sRecord);
			Uart_Packet_And_Send(ACK_IMPORT_RECORDS_BY_ID,s_sUARTAckPacket.aData, sizeof(sRecord));
		}
	}
}

static void Uart_Cmd_Record_Trans_Sub(void)
{
	if(g_nUartCurrentTransRecordIndex < g_sRecordInforMap.nTotalRecordsCounter)
	{
		Data_Record_Load_ByID(g_nUartCurrentTransRecordIndex, (sRecord *)s_sUARTAckPacket.aData);
		Uart_Packet_And_Send(ACK_EXPORT_RECORDS_TRANSMITTING,s_sUARTAckPacket.aData, sizeof(sRecord));
		g_nUartCurrentTransRecordIndex ++;
	}
	else
	{	
		Uart_Cmd_Record_Trans_Finish();
	}
}


/******************************************************************************************************/

/*通过USB上传记录到服务器****************************************************************************************/
static void Uart_Get_Device_Records_Infor(unsigned short nLen,unsigned char *pData)
{
	sUsbUartUploadRecords s;
	s.nTotalTestCounter = g_sRecordInforMap.nTotalRecordsCounter;
	s.nAlreadyUploadCounter = g_nAlreadyUploadCounter;
	memcpy(s.aIMEI, g_sSystemParam.aIMEI, sizeof(s.aIMEI));
	memcpy(s.aNamePrefix, g_sDeviceIDConfig.aNamePrefix, sizeof(s.aNamePrefix));
	s.nDeviceID = g_sDeviceIDConfig.nDeviceID;
	memcpy(&s.aPacketVersion, GPRS_DATA_PACKAGE_VERSION, 1);
	sprintf(s.aFirmwareVersion, "%s", FW_VERSION);
	memcpy(s.aRemoteIP, g_sServerParam.aRemoteIP, sizeof(s.aRemoteIP));
	memcpy(s.aRemoteURL, g_sServerParam.aRemoteURL, sizeof(s.aRemoteURL));
	s.nRemotePort = g_sServerParam.nRemotePort;
	s.nNetAccessMode = g_sServerParam.nNetAccessMode;
	Uart_Packet_And_Send(ACK_GET_DEVICE_RECORDS_INFOR,(unsigned char *)&s, sizeof(sUsbUartUploadRecords));
}

static void Uart_Notify_Record_Succeed_Upload(unsigned short nLen,unsigned char *pData)
{
	if(nLen == 4)
	{
		unsigned long nValue;
		memcpy((void *)&nValue, pData, 4);
		if(nValue > g_nAlreadyUploadCounter && ((nValue - g_nAlreadyUploadCounter) == 1))
		{
			Update_Already_Upload_Record_Counter();
		}
		Uart_Packet_And_Send(ACK_NOTIFY_RCORD_SUCCEED_UPLOAD,NULL, 0);
	}
}

/******************************************************************************************************/

/******************************************************************************************************/
static void Uart_Trace_Log_Enable(unsigned short nLen,unsigned char *pData)
{
	if(nLen == 1)
	{
		g_sSystemParam.bEnableTraceFunction = *pData ?  true : false;
		Data_System_Parameter_Save(&g_sSystemParam);
		Data_System_Parameter_Load(&g_sSystemParam);
		Uart_Packet_And_Send(ACK_TRACE_LOG_ENABLE,NULL, 0);
	}
}

static void Uart_Write_Calibration_Parameter(unsigned short nLen,unsigned char *pData)
{
	sAlcCalibrationParam *p;
	if(nLen == sizeof(sAlcCalibrationParam))
	{
		p = (sAlcCalibrationParam*)pData;
		Data_AlcCalibration_Parameter_Save(p);
		Data_AlcCalibration_Parameter_Load(&g_sAlcCalibrationParam);
		Uart_Packet_And_Send(ACK_WRITE_CALIBRATION_PARAMETER, (unsigned char *)&g_sAlcCalibrationParam, sizeof(sAlcCalibrationParam));
	}
}

static void Uart_Restart_System(unsigned short nLen,unsigned char *pData)
{
	Uart_Packet_And_Send(ACK_RESTART_SYSTEM,NULL, 0);
	if(nLen == 1 && *pData == 0xFA)
	{
		Enter_Upgrade_Mode();
	}
	else
	{
		delay_ms(100);
		NVIC_SystemReset();
	}
}

static void Uart_Delete_Logo_Parameter(unsigned short nLen,unsigned char *pData)
{
	unsigned long nAddr;
	nAddr = FLASH_LOGO_BMP_ATTRIBUTE * FLASH_PAGE_SIZE;
	Flash_Sector_Erase(nAddr);
	memset(&g_sLogoAttribute,0,sizeof(g_sLogoAttribute));
	Uart_Packet_And_Send(ACK_DELETE_LOGO_PARAMETER,NULL, 0);
}
/******************************************************************************************************/

/*USB烧录文件,开机LOGO等**********************************************************************/
static unsigned long s_nProgramPackageIndex; 
static void Uart_Cmd_Get_ProgramFileHeader(unsigned short nLen,unsigned char *pData)
{
	if(nLen == sizeof(sUSBProgramFileHeader)) //deleted on 20181107
	{
		g_sUSBProgramFileHeader = *(sUSBProgramFileHeader *)pData;
		s_nProgramPackageIndex = 0; 
		Uart_Packet_And_Send(ACK_FILE_DESCRIPTION, NULL, 0);
		delay_ms(1); //added by lxl 20181108 不添加的话，应答数据包有问题，不知为何
		TRACE_PRINTF("Uart_Cmd_Get_ProgramFileHeader!\r\n");
		if(g_sUSBProgramFileHeader.nFileType == 2)
		{
			g_sLogoAttribute.nFlashStartAddress = INSIDE_FLASH_LOGO_START_ADDR;
			g_sUSBProgramFileHeader.nFlashStartAddress =  INSIDE_FLASH_LOGO_START_ADDR;
		}
	}
}

static void Uart_Cmd_ProgramFile_Sub(unsigned short nLen,unsigned char *pData)
{
	unsigned short nIndex;
	memcpy(&nIndex,pData,2);
	if(nIndex == s_nProgramPackageIndex + 1)
	{
		if(g_sUSBProgramFileHeader.nFileType == 2)
		{
			int nAddrOffset;
			
			__disable_irq();
			
			FLASH_Unlock();
			FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);

			if((g_sUSBProgramFileHeader.nFlashStartAddress % 2048) == 0)
			{
				FLASH_ErasePage(g_sUSBProgramFileHeader.nFlashStartAddress);
			}
			
			for(nAddrOffset = 0; nAddrOffset < nLen -2; nAddrOffset += 4)
			{
				unsigned long nValue;
				memcpy((void *)&nValue, pData + 2 + nAddrOffset, 4);
				FLASH_ProgramWord(g_sUSBProgramFileHeader.nFlashStartAddress + nAddrOffset, nValue);
			}
			FLASH_Lock();
			
			__enable_irq();
		}
		else
		{	
			Spi_Flash_Init();
			if((g_sUSBProgramFileHeader.nFlashStartAddress % FLASH_PAGE_SIZE) == 0)
			{
				Flash_Sector_Erase(g_sUSBProgramFileHeader.nFlashStartAddress);
			}
			Flash_Write_Array(g_sUSBProgramFileHeader.nFlashStartAddress, pData + 2, nLen -2);
			Spi_Flash_DeInit();
		}

		TRACE_PRINTF("Flash address = 0x%08X, PackageIndex = %d, nLen = %d!\r\n",g_sUSBProgramFileHeader.nFlashStartAddress, nIndex , nLen -2);

		s_nProgramPackageIndex++;
		g_sUSBProgramFileHeader.nFlashStartAddress += nLen -2;

		Uart_Packet_And_Send(ACK_FILE_GRUOPS, NULL, 0); 

		if(nIndex == g_sUSBProgramFileHeader.nNumberOfPackages)
		{
			if(g_sUSBProgramFileHeader.nFileType == 0 || g_sUSBProgramFileHeader.nFileType == 2)
			{
				g_sLogoAttribute.nFileType = g_sUSBProgramFileHeader.nFileType;
				g_sLogoAttribute.nStartX0 = g_sUSBProgramFileHeader.nStartX0;
				g_sLogoAttribute.nStartY0 = g_sUSBProgramFileHeader.nStartY0;
				g_sLogoAttribute.nFileWidth = g_sUSBProgramFileHeader.nFileWidth;
				g_sLogoAttribute.nFileHeight = g_sUSBProgramFileHeader.nFileHeight;
				g_sLogoAttribute.nFileSize = g_sUSBProgramFileHeader.nFileSize;
				Spi_Flash_Init();
				Data_Logo_Attribute_Save(&g_sLogoAttribute);
				Data_Logo_Attribute_Load(&g_sLogoAttribute);
				Spi_Flash_DeInit();
			}
		}
	}
}
/******************************************************************************************************/

static void Uart_Cmd_Parser_Sub(void)
{
	unsigned char nCmd;
	unsigned short nCmdLen;
	unsigned char *p = &s_aCmdProcessBuffer[2];
	
	nCmd = *p;
	nCmdLen = p[1] + (((unsigned short)p[2]) << 8);
	
	switch(nCmd)
	{
	case 0:
		break;
		
	case CMD_LINK_TICK:
		Uart_Cmd_Link(nCmdLen,&p[3]);
		break;
		
	case CMD_GET_TEST_PARAM:
		Uart_Cmd_Get_Test_Parameter(nCmdLen,&p[3]);
		break;
	case CMD_SET_TEST_PARAM:
		Uart_Cmd_Set_Test_Parameter(nCmdLen,&p[3]);
		break;
	case CMD_RECOVER_DEFAULT_TEST_PARAM:
		Uart_Cmd_Recover_Default_Test_Parameter(nCmdLen,&p[3]);
		break;

	case CMD_GET_SYSTEM_PARAM:
		Uart_Cmd_Get_System_Parameter(nCmdLen,&p[3]);
		break;
	case CMD_SET_SYSTEM_PARAM:
		Uart_Cmd_Set_System_Parameter(nCmdLen,&p[3]);
		break;
	case CMD_RECOVER_DEFAULT_SYSTEM_PARAM:
		Uart_Cmd_Recover_Default_System_Parameter(nCmdLen,&p[3]);
		break;

	case CMD_GET_ALC_CALIBRATION_PARAM:
		Uart_Cmd_Get_Alc_Calibration(nCmdLen,&p[3]);
		break;
	case CMD_SET_ALC_CALIBRATION_PARAM:
		Uart_Cmd_Set_Alc_Calibration(nCmdLen,&p[3]);
		break;
	case CMD_RECOVER_DEFAULT_ALC_CALIBRATION_PARAM:
		Uart_Cmd_Recover_Default_Alc_Calibration(nCmdLen,&p[3]);
		break;
		
	case CMD_GET_RECORDS_COUNTER:
		Uart_Cmd_Get_Record_Counter(nCmdLen,&p[3]);
		break;
	case CMD_EXPORT_RECORDS_REQUEST:
		Uart_Cmd_Record_Trans_Begin(nCmdLen,&p[3]);
		break;
	case CMD_BREAK_RECORDS_EXPORT_RECORDS:
		Uart_Cmd_Break_Record_Trans();
		break;
	case CMD_CLEAR_ALL_RECORDS:
		Uart_Cmd_Clear_All_Records(nCmdLen,&p[3]);
		break;
	case CMD_EXPORT_RECORD_BY_ID:
		Uart_Cmd_Get_Record_By_ID(nCmdLen,&p[3]);
		break;
	case CMD_IMPORT_RECORDS_BY_ID:
		Uart_Cmd_Write_Record_By_ID(nCmdLen,&p[3]);
		break;

	case CMD_GET_TEMP_CALIBRATION_PARAM:
		Uart_Cmd_Get_Temperature_Calibration(nCmdLen,&p[3]);
		break;
	case CMD_SET_TEMP_CALIBRATION_PARAM:
		Uart_Cmd_Set_Temperature_Calibration(nCmdLen,&p[3]);
		break;
	case CMD_RECOVER_DEFAULT_TEMP_CALIBRATION_PARAM:
		Uart_Cmd_Recover_Default_Temperature_Calibration(nCmdLen,&p[3]);
		break;

	case CMD_GET_DEVICE_ID_PARAM:
		Uart_Cmd_Get_DeviceID(nCmdLen,&p[3]);
		break;
	case CMD_SET_DEVICE_ID_PARAM:
		Uart_Cmd_Set_DeviceID(nCmdLen,&p[3]);
		break;
	case CMD_RECOVER_DEFAULT_DEVICE_ID_PARAM:
		Uart_Cmd_Recover_Default_DeviceID(nCmdLen,&p[3]);
		break;

	case CMD_GET_SERVER_PARAM:
		Uart_Cmd_Get_Server_Parameter(nCmdLen,&p[3]);
		break;
	case CMD_SET_SERVER_PARAM:
		Uart_Cmd_Set_Server_Parameter(nCmdLen,&p[3]);
		break;
	case CMD_RECOVER_DEFAULT_SERVER_PARAM:
		Uart_Cmd_Recover_Server_Parameter(nCmdLen,&p[3]);
		break;

	case CMD_SET_SYSTEM_TIME:
		Uart_Cmd_Set_Time(nCmdLen,&p[3]);
		break;
	case CMD_SET_LCD_BRIGHTNESS:
		Uart_Cmd_Set_Lcd_Brightness(nCmdLen,&p[3]);
		break;
	case CMD_SET_VOICE_VOLUME:
		Uart_Cmd_Set_Voice_Volume(nCmdLen,&p[3]);
		break;

	case CMD_GET_TOTAL_INHALE_TIIME_COUNTER:
		Uart_Cmd_Get_Total_Inhale_Time(nCmdLen,&p[3]);
		break;
	case CMD_CLEAR_TOTAL_INHALE_TIIME_COUNTER:
		Uart_Cmd_Clear_Total_Inhale_Time(nCmdLen,&p[3]);
		break;
		
	case CMD_GET_TOTAL_TEST_COUNTER:
		Uart_Cmd_Get_Total_Test_Counter(nCmdLen,&p[3]);
		break;
	case CMD_CLEAR_TOTAL_TEST_COUNTER:
		Uart_Cmd_Clear_Total_Test_Counter(nCmdLen,&p[3]);
		break;

	case CMD_GET_USER_DEVICE_INFOR:
		Uart_Cmd_Get_User_Device_Infor(nCmdLen,&p[3]);
		break;
	case CMD_SET_USER_DEVICE_INFOR:
		Uart_Cmd_Set_User_Device_Infor(nCmdLen,&p[3]);
		break;
	case CMD_RECOVER_USER_DEVICE_INFOR:
		Uart_Cmd_Recover_User_Device_Infor(nCmdLen,&p[3]);
		break;

	case CMD_GET_DEVICE_RECORDS_INFOR:
		Uart_Get_Device_Records_Infor(nCmdLen,&p[3]);
		break;
	case CMD_NOTIFY_RCORD_SUCCEED_UPLOAD:
		Uart_Notify_Record_Succeed_Upload(nCmdLen,&p[3]);
		break;

	case CMD_DEL_USER_LOGIN_INFORMATION:
		Uart_Delete_UserLogin_Information(nCmdLen,&p[3]);
		break;

	case CMD_TRACE_LOG_ENABLE:
		Uart_Trace_Log_Enable(nCmdLen,&p[3]);
		break;

	case CMD_WRITE_CALIBRATION_PARAMETER:
		Uart_Write_Calibration_Parameter(nCmdLen,&p[3]);
		break;
	
	case CMD_RESTART_SYSTEM:
		Uart_Restart_System(nCmdLen,&p[3]);
		break;
	
	case CMD_DELETE_LOGO_PARAMETER:
		Uart_Delete_Logo_Parameter(nCmdLen,&p[3]);
		break;
		
	case CMD_FILE_DESCRIPTION:
		Uart_Cmd_Get_ProgramFileHeader(nCmdLen,&p[3]);
		break;

	case CMD_FILE_GRUOPS:
		Uart_Cmd_ProgramFile_Sub(nCmdLen,&p[3]);
		break;
	
	default:
		Uart_Cmd_Link(nCmdLen,&p[3]);
		break;
	}
}


void Uart_Usb_Cmd_Parser(void)
{
	static unsigned char nCheckSum;
	static unsigned short nDataLength;
	static unsigned char nDataLengthDescptionBytes;

	while(s_nUartOutCnt != s_nUartInCnt)
	{
		switch(s_eUSBRecievePhase)
		{
		case eUSBPhaseHead0:
			if(Get_Uart_Data() == 0xBC)
			{
				s_bStartRevCmd = true;
				s_nCmdProcessBufferCnt = 0;
				s_nStartRevCmdTime = Get_System_Time();
				s_eUSBRecievePhase = eUSBPhaseHead1;
				s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = 0xBC;
			}
			break;

		case eUSBPhaseHead1:
			if(Get_Uart_Data() == 0xFD)
			{
				s_eUSBRecievePhase = eUSBPhaseCmd;
				s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = 0xFD;
			}
			break;

		case eUSBPhaseCmd:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			nDataLength = 0;
			nDataLengthDescptionBytes = 0;
			nCheckSum = 0;
			s_eUSBRecievePhase = eUSBPhaseDataLength;
			break;

		case eUSBPhaseDataLength:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			if(nDataLengthDescptionBytes)
			{
				nDataLength = s_aCmdProcessBuffer[3] + (((unsigned short)s_aCmdProcessBuffer[4]) << 8);
				if(nDataLength >= UART_BUFFER_SIZE)
				{
					Uart_Recieve_Error_Handle(__LINE__);
				}
				else if(nDataLength)
				{
					s_eUSBRecievePhase = eUSBPhaseData;
				}
				else
				{
					s_eUSBRecievePhase = eUSBPhaseCrc;
				}
			}
			else
			{
				nDataLengthDescptionBytes++;
			}
			break;

		case eUSBPhaseData:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			nCheckSum += s_aCmdProcessBuffer[s_nCmdProcessBufferCnt - 1];
			if(s_nCmdProcessBufferCnt >= USB_CMD_PROCESS_BUFFER_SIZE)
			{
				Uart_Recieve_Error_Handle(__LINE__);
			}
			if(--nDataLength == 0)
			{
				s_eUSBRecievePhase = eUSBPhaseCrc;
			}
			break;

		case eUSBPhaseCrc:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			if(s_nCmdProcessBufferCnt >= USB_CMD_PROCESS_BUFFER_SIZE || nCheckSum != s_aCmdProcessBuffer[s_nCmdProcessBufferCnt - 1])
			{
				Uart_Recieve_Error_Handle(__LINE__);
				break;
			}
			s_eUSBRecievePhase = eUSBPhaseEnd;
			break;

		case eUSBPhaseEnd:
			if(Get_Uart_Data() == 0xAE)
			{
				Uart_Cmd_Parser_Sub();
				pApp->nStartIdleTime = Get_System_Time();
				s_bStartRevCmd = false;
				s_eUSBRecievePhase = eUSBPhaseHead0;
			}
			break;
		}
	}

	if(s_bStartRevCmd && Get_System_Time()  > s_nStartRevCmdTime +  UART_CMD_TIMOUT)
	{
		Uart_Recieve_Error_Handle(__LINE__);
	}

	if(g_bUartStartTransRecords)
	{
		Uart_Cmd_Record_Trans_Sub();
	}
}
