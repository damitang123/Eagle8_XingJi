#include "platform.h"
#include "parameter.h"
#include "data.h"
#include "calibrate.h"
#include "spi_flash.h"
#include "delay.h"
#include "data.h"
#include "Windows.h"
#include "ads8320.h"
#include "gprs.h"
#include "app.h"
#include "spi_flash_efficient_rw.h"


sRecord		g_sRecord;
sDeviceUserInfor g_sDeviceUserInfo;	/*这个结构体的内容最终会赋值到记录里.*/
sPoliceUserInfor g_sCurrPoliceUserInfor;
unsigned long g_nNewRecordID;
sRecordInforMap g_sRecordInforMap;
unsigned long 	g_nAlreadyUploadCounter;
sUSBProgramFileHeader g_sUSBProgramFileHeader;
/*统计抽气时间.*/
unsigned long 		g_nStartInhaleTime;
float 			g_fTotalInhaleTimeCounter;

/*统计总测试次数*/
unsigned long 		g_nTotalTestCounter;

/*向PC后台传输数据*/
bool 			g_bUartStartTransRecords = false;
unsigned long 		g_nUartCurrentTransRecordIndex = 0;		/*从0计数*/

void  Data_Init(void)
{
	Spi_Flash_Init();
	
	/*参数导入*/
	Data_TouchCalibration_Load();
	Data_Test_Parameter_Load(&g_sTestParam);
	Data_System_Parameter_Load(&g_sSystemParam);
	Data_Printer_Parameter_Load(&g_sPrinterParam);
	#if SERVER_USE_XING_JIKONG_GU
	Data_HeartRate_Parameter_Load(&g_sHeartRateParam);
	#endif
	Data_AlcCalibration_Parameter_Load(&g_sAlcCalibrationParam);
	Data_Temperature_Calibration_Parameter_Load(&g_sTemperatureCalibrationParam);
	Data_DeviceID_Config_Load(&g_sDeviceIDConfig);
	Data_Server_Parameter_Load(&g_sServerParam);
	Data_Logo_Attribute_Load(&g_sLogoAttribute);
	Data_Device_UserInfor_Load(&g_sDeviceUserInfo);
	
	Data_Record_Load_Last(&g_sRecord);
	Data_Record_InforMap_Load(&g_sRecordInforMap);
	Data_Total_Test_Counter_Load(&g_nTotalTestCounter);
	Data_Total_InhaleTime_Counter_Load(&g_fTotalInhaleTimeCounter);
	Data_Already_Upload_Counter_Load(&g_nAlreadyUploadCounter);

	TRACE_PRINTF("*****************************************************\r\n");
	TRACE_PRINTF("sizeof(sRecord) = %d\r\n", sizeof(sRecord));
	TRACE_PRINTF("sizeof(sTestParam) = %d\r\n", sizeof(sTestParam));
	TRACE_PRINTF("sizeof(sSystemParam) = %d\r\n", sizeof(sSystemParam));
	TRACE_PRINTF("sizeof(sPrinterParam) = %d\r\n", sizeof(sPrinterParam));
	TRACE_PRINTF("sizeof(sAlcCalibrationParam) = %d\r\n", sizeof(sAlcCalibrationParam));
	TRACE_PRINTF("sizeof(sTemperatureCalibrationParam) = %d\r\n", sizeof(sTemperatureCalibrationParam));
	TRACE_PRINTF("sizeof(sDeviceIDConfig) = %d\r\n", sizeof(sDeviceIDConfig));
	TRACE_PRINTF("sizeof(sServerParam) = %d\r\n", sizeof(sServerParam));

	TRACE_PRINTF("g_nTotalTestCounter = %d\r\n", g_nTotalTestCounter);
	TRACE_PRINTF("g_sRecordInforMap.nTotalRecordsCounter = %d\r\n", g_sRecordInforMap.nTotalRecordsCounter);
	TRACE_PRINTF("g_nAlreadyUploadCounter = %d\r\n", g_nAlreadyUploadCounter);
	TRACE_PRINTF("g_fTotalInhaleTimeCounter = %.2f\r\n", g_fTotalInhaleTimeCounter);
	TRACE_PRINTF("g_sAlcCalibrationParam.nPeekCaliValue = %d\r\n", g_sAlcCalibrationParam.nPeekCaliValue);
	TRACE_PRINTF("g_sAlcCalibrationParam.nIntergralLowPoint = %d\r\n", g_sAlcCalibrationParam.nIntergralLowPoint);
	TRACE_PRINTF("g_sAlcCalibrationParam.nIntergralHighPoint = %d\r\n", g_sAlcCalibrationParam.nIntergralHighPoint);
	TRACE_PRINTF("g_sSystemParam.nAutoPowerOffTime = %ld\r\n", g_sSystemParam.nAutoPowerOffTime);
	TRACE_PRINTF("g_sSystemParam.nAutoSleepTime = %ld\r\n", g_sSystemParam.nAutoSleepTime);

	TRACE_PRINTF("*****************************************************\r\n");

	g_Language = (eSystemLanguage)g_sSystemParam.nSystemLanguage;
	if(g_Language > eLanguage_Spanish)
	{
		g_Language = eLanguage_Chinese;
	}
}

bool Data_Test_Parameter_Load(sTestParam *pTestParam)
{
	sTestParam s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sTestParam), FLASH_SECTOR_TEST_PARAM) == false)
	{
		pTestParam->nVersion = 0xF0;
		pTestParam->nTestUnit = 0;
		pTestParam->nBrACBACK = 2200;
		pTestParam->nInhaleTime = 3000;
		pTestParam->fAlcDrinkThreshold = 20.0;
		pTestParam->fAlcDrunkThreshold = 80.0;
		pTestParam->nResultMode = 0;
		pTestParam->nAdjustCoefficientPeek = 100;
		pTestParam->nBlowTime = 3000;
		pTestParam->nBlowPress = 1;
		pTestParam->nAdjustCoefficientIntegral = 100;
		pTestParam->nAlcAlarmMode = 3;
		pTestParam->nBlowFlowVolume = 12;
		pTestParam->nSampleMethod = 0;
		memset(pTestParam->aReserved, 0, sizeof(pTestParam->aReserved));
		return false;
	}

	*pTestParam = s;

	if(pTestParam->nBlowFlowVolume < 12)
	{
		pTestParam->nBlowFlowVolume = 12;
	}

	return true;
}

bool Data_Test_Parameter_Save(sTestParam *pTestParam)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pTestParam,
	                           sizeof(sTestParam), FLASH_SECTOR_TEST_PARAM);

	return true;
}

bool Data_System_Parameter_Load(sSystemParam *pSystemParam)
{
	sSystemParam s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sSystemParam), FLASH_SECTOR_SYSTEM_PARAM) == false)
	{
		pSystemParam->nVersion = 0xF0;
		pSystemParam->nSystemLanguage = eLanguage_Chinese;
		strcpy(pSystemParam->aAdminPassword, "26715360");
		pSystemParam->nVoiceSwitch = 1;
		pSystemParam->nVoiceVolume = 100;
		pSystemParam->nAutoSleepTime = 3 * 60000;
		pSystemParam->nAutoPowerOffTime = 5 * 60000;
		pSystemParam->nSystemDisplayTheme = 0;
		pSystemParam->nAutoBackLightTuneSwitch = 1;
		pSystemParam->nBackLightPercent = 50;
		memset(pSystemParam->aIMEI, 0, sizeof(pSystemParam->aIMEI));
		pSystemParam->nIMEIValidFlag = 0;
		pSystemParam->nAlcAlarmDurationTime = 0;
		pSystemParam->nTimeZone = 8;
		pSystemParam->bEnableTraceFunction = false;
		pSystemParam->nFlowVolumeAdjustCoefficient = 0;
		memset(pSystemParam->aReserved, 0, sizeof(pSystemParam->aReserved));
		return false;
	}

	*pSystemParam = s;

	return true;
}

bool Data_System_Parameter_Save(sSystemParam *pSystemParam)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pSystemParam,
	                           sizeof(sSystemParam), FLASH_SECTOR_SYSTEM_PARAM);

	return true;
}


bool Data_Printer_Parameter_Load(sPrinterParam *pPrinterParam)
{
	sPrinterParam s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sPrinterParam), FLASH_SECTOR_PRINTER_PARAM) == false)
	{
		pPrinterParam->nVersion = 0xF0;
		pPrinterParam->nPrinterType = 1;
		pPrinterParam->nPrintCopys = 1;
		memset(pPrinterParam->aReserved, 0, sizeof(pPrinterParam->aReserved));
		return false;
	}

	*pPrinterParam = s;

	return true;
}

bool Data_Printer_Parameter_Save(sPrinterParam *pPrinterParam)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pPrinterParam,
	                           sizeof(sPrinterParam), FLASH_SECTOR_PRINTER_PARAM);

	return true;
}

#if SERVER_USE_XING_JIKONG_GU

bool Data_HeartRate_Parameter_Load(sHeartRateParam *pHeartRateParam)
{
	sHeartRateParam s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sHeartRateParam), FLASH_SECTOR_HEARTRATE_PARAM) == false)
	{
		pHeartRateParam->byVersion = 0xF0;
		pHeartRateParam->byHeartRate = 10; //10s
		pHeartRateParam->byValidDataFlag=0;
		memset(pHeartRateParam->a_byReserved, 0, sizeof(pHeartRateParam->a_byReserved));
		return false;
	}

	*pHeartRateParam = s;

	return true;
}

bool Data_HeartRate_Parameter_Save(sHeartRateParam *pHeartRateParam)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pHeartRateParam,
	                           sizeof(sHeartRateParam), FLASH_SECTOR_HEARTRATE_PARAM);

	return true;
}
#endif

bool Data_AlcCalibration_Parameter_Load(sAlcCalibrationParam *pAlcCalibrationParam)
{
	sAlcCalibrationParam s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sAlcCalibrationParam), FLASH_SECTOR_ALC_CALIBRATION) == false)
	{
		pAlcCalibrationParam->nVersion = 0xF0;
		memcpy(pAlcCalibrationParam->aDateTimePeek, (void *)&g_sDateTime, sizeof(pAlcCalibrationParam->aDateTimePeek));
		pAlcCalibrationParam->nPeekCaliValue = 3160;
		memcpy(pAlcCalibrationParam->aDateTimeIntergral, (void *)&g_sDateTime, sizeof(pAlcCalibrationParam->aDateTimeIntergral));
		pAlcCalibrationParam->nIntergralLowPoint = 11000000 / 4;
		pAlcCalibrationParam->nIntergralHighPoint = 11000000;
		pAlcCalibrationParam->nPeekCaliValue = 3160;
		pAlcCalibrationParam->nUserIntergralLowPoint = pAlcCalibrationParam->nIntergralLowPoint;
		pAlcCalibrationParam->nUserIntergralHighPoint = pAlcCalibrationParam->nIntergralHighPoint;
		pAlcCalibrationParam->fPeekStandardAlcoholValue = 40;
		pAlcCalibrationParam->fLStandardAlcoholValue = 20;
		pAlcCalibrationParam->fHStandardAlcoholValue = 80;
		pAlcCalibrationParam->fLRealTestAlcoholValue = 20;
		pAlcCalibrationParam->fHRealTestAlcoholValue = 80;
		pAlcCalibrationParam->nZeroIntergralValue = 100000;
		memset(pAlcCalibrationParam->aReserved, 0, sizeof(pAlcCalibrationParam->aReserved));
		return false;
	}

	*pAlcCalibrationParam = s;

	Calibrate_Parameter_Check(pAlcCalibrationParam);

	return true;
}

bool Data_AlcCalibration_Parameter_Save(sAlcCalibrationParam *pAlcCalibrationParam)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pAlcCalibrationParam,
	                           sizeof(sAlcCalibrationParam), FLASH_SECTOR_ALC_CALIBRATION);

	return true;
}

BOOL Data_Temperature_Calibration_Parameter_Load(sTemperatureCalibrationParam *pTempCalData)
{
	unsigned long nAddr;

	nAddr = FLASH_SECTOR_TEMPERATURE_CAL * FLASH_PAGE_SIZE;

	if(Flash_Read_Byte(nAddr + 0x5ff) == 0xA5)
	{
		Flash_Read_Array(nAddr, (unsigned char *)pTempCalData, sizeof(sTemperatureCalibrationParam));
		return TRUE;
	}
	else
	{
		/*赋默认值*/
		Data_Init_Default_TemperatureCalibrate();
		return FALSE;
	}
}

BOOL Data_Temperature_Calibration_Parameter_Save(sTemperatureCalibrationParam *pTempCalData)
{
	unsigned long nAddr;

	nAddr = FLASH_SECTOR_TEMPERATURE_CAL * FLASH_PAGE_SIZE;

	Flash_Sector_Erase(nAddr);

	if(Flash_Write_Array(nAddr, (unsigned char *)pTempCalData, sizeof(sTemperatureCalibrationParam)))
	{
		Flash_Write_Byte(nAddr + 0x5ff, 0xA5);
		return TRUE;
	}
	else
		return FALSE;
}

bool Data_DeviceID_Config_Load(sDeviceIDConfig *pDeviceIDConfig)
{
	sDeviceIDConfig s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sDeviceIDConfig), FLASH_SECTOR_DEVICE_ID) == false)
	{
		pDeviceIDConfig->nVersion = 0xF0;
	#if EAGLE_1
		strcpy(pDeviceIDConfig->aNamePrefix, "E1");
		strcpy(pDeviceIDConfig->aDeviceTypeString,"Eagle-1");
	#elif EAGLE_8
		#if defined(CUST_CHINA_EAGLE8_TO_EAGLE1)
			strcpy(pDeviceIDConfig->aNamePrefix, "E1");
			strcpy(pDeviceIDConfig->aDeviceTypeString,"Eagle-1");
		#else
			strcpy(pDeviceIDConfig->aNamePrefix, "E8");
			strcpy(pDeviceIDConfig->aDeviceTypeString,"Eagle-8");
		#endif
	#endif
		pDeviceIDConfig->nDeviceID = 1;
		memset(pDeviceIDConfig->aReserved, 0, sizeof(pDeviceIDConfig->aReserved));
		return false;
	}

	*pDeviceIDConfig = s;

	return true;
}

bool Data_DeviceID_Config_Save(sDeviceIDConfig *pDeviceIDConfig)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pDeviceIDConfig,
	                           sizeof(sDeviceIDConfig), FLASH_SECTOR_DEVICE_ID);

	return true;
}

bool Data_Server_Parameter_Load(sServerParam *pServerParam)
{
	sServerParam s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sServerParam), FLASH_SECTOR_SERVER_PARAM) == false)
	{
	#if SERVER_USE_KEYUN_DAI
		pServerParam->nVersion = 0xF0;
		strcpy(pServerParam->aRemoteIP, "47.104.88.1");
		strcpy(pServerParam->aRemoteURL, "");
		pServerParam->nRemotePort = 8282;
		pServerParam->nNetAccessMode = 0;
	#elif SERVER_USE_XING_JIKONG_GU
		pServerParam->nVersion = 0xF1;
		strcpy(pServerParam->aRemoteIP, "221.12.76.40"); //"121.42.140.91" modified on 20181106
		strcpy(pServerParam->aRemoteURL, "www.bracaudit-gps.com");
		pServerParam->nRemotePort = 8036;
		pServerParam->nNetAccessMode = 0;
		strcpy(pServerParam->aAPNString, "CMIOTTSJTJCZD.HE");
	#else
		pServerParam->nVersion = 0xF0;
		strcpy(pServerParam->aRemoteIP, "121.42.140.91");
		strcpy(pServerParam->aRemoteURL, "www.bracaudit-gps.com");
		pServerParam->nRemotePort = 8036;
		pServerParam->nNetAccessMode = 0;
	#endif
		memset(pServerParam->aReserved, 0, sizeof(pServerParam->aReserved));
		return false;
	}

	*pServerParam = s;

	return true;
}

bool Data_Server_Parameter_Save(sServerParam *pServerParam)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pServerParam,
	                           sizeof(sServerParam), FLASH_SECTOR_SERVER_PARAM);

	return true;
}

BOOL Data_Logo_Attribute_Save(sLogoAttribute *pLogoAttribute)
{
	unsigned long nAddr;

	nAddr = FLASH_LOGO_BMP_ATTRIBUTE * FLASH_PAGE_SIZE;
	Flash_Sector_Erase(nAddr);
	
	pLogoAttribute->nValidDataFlag = 0xA5;
	if(Flash_Write_Array(nAddr, (unsigned char *)pLogoAttribute, sizeof(sLogoAttribute)))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


BOOL Data_Logo_Attribute_Load(sLogoAttribute *pLogoAttribute)
{
	unsigned long nAddr;

	nAddr = FLASH_LOGO_BMP_ATTRIBUTE * FLASH_PAGE_SIZE;

	Flash_Read_Array(nAddr, (unsigned char *)pLogoAttribute, sizeof(sLogoAttribute));

	if(pLogoAttribute->nValidDataFlag != 0xA5)
	{
		memset(pLogoAttribute, 0, sizeof(sLogoAttribute));
		return FALSE;
	}

	return TRUE;
}


BOOL Data_TouchCalibration_Load(void)
{
	int aData[4];
	unsigned long nAddr;

	nAddr = FLASH_TOUCH_CALIBRATION * FLASH_PAGE_SIZE;

	if(Flash_Read_Byte(nAddr + 0x3ff) == 0xAA)
	{
		Flash_Read_Array(nAddr, (unsigned char *)aData, sizeof(aData));
		GUI_TOUCH_SetCalData(0,&aData[0],&aData[1]);
		GUI_TOUCH_SetCalData(1,&aData[2],&aData[3]);
		return TRUE;
	}
	else
	{
		pApp->bTouchCalibrateAtStartUp = true;
		return FALSE;
	}
}

BOOL Data_TouchCalibration_Save(void)
{
	int aData[4];
	unsigned long nAddr;

	nAddr = FLASH_TOUCH_CALIBRATION * FLASH_PAGE_SIZE;

	Flash_Sector_Erase(nAddr);

	GUI_TOUCH_GetCalData(0,&aData[0],&aData[1]);
	GUI_TOUCH_GetCalData(1,&aData[2],&aData[3]);

	if(Flash_Write_Array(nAddr, (unsigned char *)aData, sizeof(aData)))
	{
		Flash_Write_Byte(nAddr + 0x3ff, 0xAA);
		return TRUE;
	}
	else
		return FALSE;
}


bool Data_Record_InforMap_Save(sRecordInforMap *pRecordInforMap)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pRecordInforMap,
	                           sizeof(sRecordInforMap), FLASH_SECTOR_RECORD_INFO_MAP);

	return true;
}

bool Data_Record_InforMap_Load(sRecordInforMap *pRecordInforMap)
{
	sRecordInforMap s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(s), FLASH_SECTOR_RECORD_INFO_MAP) == false)
	{
		pRecordInforMap->nTotalRecordsCounter = 0;
		pRecordInforMap->nSDCardFreeStartSector = 0;
		return false;
	}

	*pRecordInforMap = s;

	return true;
}

bool Data_Device_UserInfor_Save(sDeviceUserInfor *pDeviceUserInfor)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pDeviceUserInfor,
	                           sizeof(sDeviceUserInfor), FLASH_SECTOR_DEVICE_USER_INFOR);

	return true;
}

bool Data_Device_UserInfor_Load(sDeviceUserInfor *pDeviceUserInfor)
{
	sDeviceUserInfor s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(s), FLASH_SECTOR_DEVICE_USER_INFOR) == false)
	{
		memset(pDeviceUserInfor, 0, sizeof(sDeviceUserInfor));
		return false;
	}

	*pDeviceUserInfor = s;

	return true;
}


bool Data_Total_InhaleTime_Counter_Load(float *fInhaleTimeCounter)
{
	sTotalInhaleTime s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sTotalInhaleTime), FLASH_SECTOR_TOTAL_INHALE_TIME) == false)
	{
		*fInhaleTimeCounter = 0;
		return false;
	}

	*fInhaleTimeCounter = s.fTotalInhaleTimeCounter;

	return true;
}

bool Data_Total_InhaleTime_Counter_Save(float *fInhaleTimeCounter)
{
	sTotalInhaleTime s;
	s.fTotalInhaleTimeCounter = *fInhaleTimeCounter;
	Spi_FLash_Save_User_Struct((unsigned char *)&s,
	                           sizeof(sTotalInhaleTime), FLASH_SECTOR_TOTAL_INHALE_TIME);

	return true;
}

bool Data_Total_Test_Counter_Load(unsigned long *nTotalTestCounter)
{
	sTotalTestCounter s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sTotalTestCounter), FLASH_SECTOR_TOTAL_TEST_COUNTER) == false)
	{
		*nTotalTestCounter = 0;
		return false;
	}

	*nTotalTestCounter = s.nTotalTestCounter;

	return true;
}

bool Data_Total_Test_Counter_Save(unsigned long *nTotalTestCounter)
{
	sTotalTestCounter s;
	s.nTotalTestCounter = *nTotalTestCounter;
	Spi_FLash_Save_User_Struct((unsigned char *)&s,
	                           sizeof(sTotalTestCounter), FLASH_SECTOR_TOTAL_TEST_COUNTER);

	return true;
}

bool Data_Already_Upload_Counter_Load(unsigned long *nAlreadyUpLoadIndex)
{
	sAlreadyUploadRecord s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sAlreadyUploadRecord), FLASH_SECTOR_ALREADY_UPLOAD_INDEX) == false)
	{
		*nAlreadyUpLoadIndex = 0;
		return false;
	}

	*nAlreadyUpLoadIndex = s.nAlreadyUploadCounter;

	return true;
}

bool Data_Already_Upload_Counter_Save(unsigned long *nAlreadyUpLoadIndex)
{
	sAlreadyUploadRecord s;
	s.nAlreadyUploadCounter = *nAlreadyUpLoadIndex;
	Spi_FLash_Save_User_Struct((unsigned char *)&s,
	                           sizeof(sAlreadyUploadRecord), FLASH_SECTOR_ALREADY_UPLOAD_INDEX);

	return true;
}

/*不需要上电时读入,加载的时候自动读入到列表供用户选择*/
bool Data_Police_User_Infor_Load_ByID(unsigned short nID, sPoliceUserInfor *pPoliceUserInfor)
{
	unsigned long nAddr;
	nAddr = FLASH_SECTOR_POLICE_USERS_INFOR * FLASH_SECTOR_SIZE + (unsigned long)nID * sizeof(sPoliceUserInfor);
	Flash_Read_Array(nAddr, (unsigned char *)pPoliceUserInfor, sizeof(sPoliceUserInfor));
	return pPoliceUserInfor->nValidDataFlag == 0xA5 ? true : false;
}

bool Data_Police_User_Infor_Insert_Save(sPoliceUserInfor *pPoliceUserInfor)
{
	int i;
	sPoliceUserInfor s;
	unsigned long nAddr;

	pPoliceUserInfor->nValidDataFlag = 0xA5;
	
	/*1.查找空闲位置.*/
	for(i = 0 ; i < FLASH_SECTOR_SIZE / sizeof(sPoliceUserInfor); i++)
	{
		if(Data_Police_User_Infor_Load_ByID(i,&s) == false)
		{
			break;
		}
	}

	if(i == 0)
	{
		Flash_Sector_Erase(FLASH_SECTOR_POLICE_USERS_INFOR * FLASH_SECTOR_SIZE);
	}
	
	/*2.写入新数据*/
	nAddr = FLASH_SECTOR_POLICE_USERS_INFOR * FLASH_SECTOR_SIZE + i * sizeof(sPoliceUserInfor);
	if(i * sizeof(sPoliceUserInfor) < FLASH_SECTOR_SIZE)
	{
		Flash_Write_Array(nAddr , (unsigned char *)pPoliceUserInfor, sizeof(sPoliceUserInfor));
	}

	return true;
}

/*存储的当前用户的信息..自动填入警察身份信息(编号,ID,部门等)*/
bool Data_Police_Last_User_Infor_Load(sPoliceUserInfor *pPoliceUserInfor)
{
	sPoliceUserInfor s;
	if(Spi_FLash_Load_User_Struct((unsigned char *)&s,
	                              sizeof(sPoliceUserInfor), FLASH_SECTOR_POLICE_LAST_USER) == false)
	{
		memset(pPoliceUserInfor,0,sizeof(sPoliceUserInfor));
		return false;
	}

	*pPoliceUserInfor = s;

	return true;
}

bool Data_Police_Last_User_Infor_Save(sPoliceUserInfor *pPoliceUserInfor)
{
	Spi_FLash_Save_User_Struct((unsigned char *)pPoliceUserInfor,
	                           sizeof(sPoliceUserInfor), FLASH_SECTOR_POLICE_LAST_USER);

	return true;
}

BOOL Data_Record_Load_ByID(unsigned long nRecordID, sRecord *pRecord)
{
	unsigned long nAddr;
	nAddr = FLASH_SECTOR_START_RECORD * FLASH_PAGE_SIZE + nRecordID * sizeof(sRecord);
	Flash_Read_Array(nAddr, (unsigned char *)pRecord, sizeof(sRecord));
	return TRUE;
}

BOOL Data_Record_Load_Last(sRecord *pRecord)
{
	if(Data_Record_InforMap_Load(&g_sRecordInforMap) == false)
	{
		TRACE_PRINTF("Erase g_sRecordInforMap sector!\r\n");
		Flash_Sector_Erase(FLASH_SECTOR_RECORD_INFO_MAP * FLASH_SECTOR_SIZE);
		Data_Record_InforMap_Save(&g_sRecordInforMap);
	}
	
	if(g_sRecordInforMap.nTotalRecordsCounter != 0)
	{
		Data_Record_Load_ByID(g_sRecordInforMap.nTotalRecordsCounter - 1 , pRecord);
	}
	else
	{
		memset(pRecord, 0, sizeof(sRecord));
	}

	g_nNewRecordID = g_sRecordInforMap.nTotalRecordsCounter + 1;

	return true;
}

BOOL Data_Record_Save_Sub(sRecord *pRecord)
{
	unsigned long nAddr;

	nAddr = FLASH_SECTOR_START_RECORD * FLASH_SECTOR_SIZE + (g_nNewRecordID - 1) * sizeof(sRecord);

	if(nAddr > Flash_Get_Max_Sector() * FLASH_SECTOR_SIZE)
	{
		TRACE_PRINTF("Data Record Failed!Aready reach max sector!\r\n");
		return false;
	}

	if(nAddr % FLASH_SECTOR_SIZE == 0)
	{
		TRACE_PRINTF("Need erase current sector(save sub)!\r\n");
		Flash_Sector_Erase(nAddr);
	}

	g_sRecordInforMap.nTotalRecordsCounter++;
	Data_Record_InforMap_Save(&g_sRecordInforMap);

	TRACE_PRINTF("Data_Record_Save_Sub(g_sRecordInforMap.nTotalRecordsCounter = %ld)\r\n", g_sRecordInforMap.nTotalRecordsCounter);

	if(Flash_Write_Array(nAddr, (unsigned char *)pRecord, sizeof(sRecord)))
	{
		/*记录下历史的警员信息*/
		sDeviceUserInfor s;
		memcpy(s.aLocation, pRecord->aLocation, sizeof(s.aLocation));
		memcpy(s.aPoliceID, pRecord->aPoliceID, sizeof(s.aPoliceID));
		memcpy(s.aPoliceName, pRecord->aPoliceName, sizeof(s.aPoliceName));
		memcpy(s.aDepartment, pRecord->aDepartment, sizeof(s.aDepartment));
		if(memcmp((void *)&s, (void *)&g_sDeviceUserInfo, sizeof(sDeviceUserInfor) - 1))
		{
			Data_Device_UserInfor_Save((sDeviceUserInfor*)&s);
			Data_Device_UserInfor_Load(&g_sDeviceUserInfo);
		}
		return true;
	}

	return false;
}

void Data_Record_Full_Save(void)
{
	/*存入记录*/
	if(pApp->bNewDataNeedSave)  //检查是否有数据需要保存
	{
		pApp->bNewDataNeedSave = false;
		Spi_Flash_Init();
		Data_Record_Save_Sub(&g_sRecord);
		Data_Record_Load_Last(&g_sRecord);
		Spi_Flash_DeInit();
	}
}

void Data_Records_Clear(void)
{
	/*1.删除记录总计数*/
	Flash_Sector_Erase(FLASH_SECTOR_RECORD_INFO_MAP * FLASH_PAGE_SIZE);

	/*2.删除已上传记数*/
	Flash_Sector_Erase(FLASH_SECTOR_ALREADY_UPLOAD_INDEX * FLASH_PAGE_SIZE);
	
	Data_Record_Load_Last(&g_sRecord);
	Data_Already_Upload_Counter_Load(&g_nAlreadyUploadCounter);
}

