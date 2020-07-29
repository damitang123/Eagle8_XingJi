#ifndef _DATA_H_
#define _DATA_H_

#include "parameter.h"


typedef struct
{
	unsigned long 		nRecordIndex;
	signed char 		nTemperature;
	float 			fAlcValue;
	unsigned long 		nIntergralValue;
	unsigned char 	aDateTime[6];
	unsigned char 	nTestMode;		/*0,主动1,被动 2,拒测 3,筛查*/
	unsigned char 	nTestUnit;
	unsigned short 	nKBrACBAC;
	unsigned char 	nAdjustCoefficient;
	unsigned char 	nTestResult;
	float				fLongitude;
	float    			fLatitude;
	unsigned char 	aTestee[23];
	unsigned char 	aLocation[46];
	unsigned char 	aPlateNumber[26];
	unsigned char 	aDriverLicense[20];
	unsigned char 	aPoliceID[15];
	unsigned char 	aDepartment[46];
	unsigned char 	aPoliceName[23];
	char 			aReserved[24];
} __attribute__ ((packed)) sRecord;

typedef struct
{
	unsigned char 	nVersion;
	unsigned char 	nIndex;				/*序列号,由上位机负责,从1计数*/
	unsigned long 		nModifyUTC;			/*设备修改的UTC时间.*/
	char 			aPoliceID[12];		/*UTF8编码*/
	char 			aPoliceName[23];		/*UTF8编码*/
	char 			aDepartment[46];	/*UTF8编码*/
	char 			aPassword[7];		/*用户密码,仅能输入数字*/
	unsigned char 	aReserved[33];		/**/
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed))sPoliceUserInfor;

typedef struct
{
	unsigned char 	aLocation[46];
	unsigned char 	aPoliceID[15];
	unsigned char 	aPoliceName[23];
	unsigned char 	aDepartment[46];
	unsigned char		nValidDataFlag;
} __attribute__ ((packed)) sDeviceUserInfor;

typedef struct
{
	unsigned long 		nTotalRecordsCounter;
	unsigned long		nSDCardFreeStartSector;
	unsigned char 	aReserved[7];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sRecordInforMap;

typedef struct
{
	float 			fTotalInhaleTimeCounter;
	unsigned char 	aReserved[3];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sTotalInhaleTime;

typedef struct
{
	unsigned long 		nTotalTestCounter;
	unsigned char 	aReserved[3];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sTotalTestCounter;

typedef struct
{
	unsigned long 		nAlreadyUploadCounter;
	unsigned char 	aReserved[3];
	unsigned char 	nValidDataFlag;
}__attribute__ ((packed)) sAlreadyUploadRecord;


typedef struct
{
	unsigned char 	nFileType;	/*0是JPEG图片,1是数据,2是BMP图片.*/
	unsigned long		nFileSize;
	unsigned short 	nNumberOfPackages;
	unsigned short 	nPackageSize;
	unsigned long 		nFlashStartAddress;
	unsigned short	nStartX0;
	unsigned short	nStartY0;
	unsigned short	nFileWidth;
	unsigned short	nFileHeight;
} __attribute__ ((packed)) sUSBProgramFileHeader;


typedef struct
{
	unsigned long		nTotalTestCounter;
	unsigned long		nAlreadyUploadCounter;
	char				aIMEI[16];
	char				aNamePrefix[5];
	unsigned long		nDeviceID;
	char 			aPacketVersion;
	char				aFirmwareVersion[30];
	char				aRemoteIP[16];
	char 			aRemoteURL[40];
	unsigned short 	nRemotePort;
	char				nNetAccessMode;		/*0为IP,1为DNS*/
}__attribute__ ((packed))sUsbUartUploadRecords;


#define FLASH_PAGE_SIZE						0x1000

/*FLASH 分布图*/
#define FLASH_SECTOR_TEST_PARAM				513
#define FLASH_SECTOR_SYSTEM_PARAM			514
#define FLASH_SECTOR_PRINTER_PARAM			515
#define FLASH_SECTOR_ALC_CALIBRATION			516
#define FLASH_SECTOR_TEMPERATURE_CAL			517
#define FLASH_SECTOR_DEVICE_ID				518
#define FLASH_SECTOR_SERVER_PARAM			519
#define FLASH_SECTOR_HEARTRATE_PARAM		520 //added by lxl 20180929 
#define FLASH_TOUCH_CALIBRATION				521


#define FLASH_SECTOR_RECORD_INFO_MAP			525
#define FLASH_SECTOR_TOTAL_INHALE_TIME		526
#define FLASH_SECTOR_TOTAL_TEST_COUNTER		527
#define FLASH_SECTOR_ALREADY_UPLOAD_INDEX	528
#define FLASH_SECTOR_DEVICE_USER_INFOR		529	//设备使用者信息
#define FLASH_SECTOR_POLICE_USERS_INFOR		530	//已注册用户的信息列表信息
#define FLASH_SECTOR_POLICE_LAST_USER		531	//当前使用的用户信息.


#define FLASH_SECTOR_FP_ENROLL_USER_INFO		544 	//保留两个扇区的位置.


#define FLASH_SECTOR_TEMP_SWAP				546

#define FLASH_LOGO_BMP_ATTRIBUTE				547
#define FLASH_LOGO_BMP_START_SECTOR			548	//至少留32个扇区位置.

#define FLASH_SECTOR_START_RECORD			580


/*片内LOGO起始地址,大小小于64K*/
#define INSIDE_FLASH_LOGO_START_ADDR			0x08070000


extern sRecord 		g_sRecord;
extern sDeviceUserInfor g_sDeviceUserInfo;
extern sPoliceUserInfor g_sCurrPoliceUserInfor;
extern sRecordInforMap g_sRecordInforMap;
extern unsigned long 	g_nNewRecordID;
extern unsigned long 	g_nAlreadyUploadCounter;
extern sUSBProgramFileHeader g_sUSBProgramFileHeader;

extern unsigned long 	g_nStartInhaleTime;
extern float 			g_fTotalInhaleTimeCounter;

extern unsigned long 	g_nTotalTestCounter;

extern bool 			g_bUartStartTransRecords;
extern unsigned long 	g_nUartCurrentTransRecordIndex;

void Data_Init(void);
bool Data_Test_Parameter_Load(sTestParam *pTestParam);
bool Data_Test_Parameter_Save(sTestParam *pTestParam);
bool Data_System_Parameter_Load(sSystemParam *pSystemParam);
bool Data_System_Parameter_Save(sSystemParam *pSystemParam);
bool Data_Printer_Parameter_Load(sPrinterParam *pPrinterParam);
bool Data_Printer_Parameter_Save(sPrinterParam *pPrinterParam);
bool Data_AlcCalibration_Parameter_Load(sAlcCalibrationParam *pAlcCalibrationParam);
bool Data_AlcCalibration_Parameter_Save(sAlcCalibrationParam *pAlcCalibrationParam);
bool Data_Temperature_Calibration_Parameter_Load(sTemperatureCalibrationParam *pTempCalData);
bool Data_Temperature_Calibration_Parameter_Save(sTemperatureCalibrationParam *pTempCalData);
bool Data_DeviceID_Config_Load(sDeviceIDConfig *pDeviceIDConfig);
bool Data_DeviceID_Config_Save(sDeviceIDConfig *pDeviceIDConfig);
bool Data_Server_Parameter_Load(sServerParam *pServerParam);
bool Data_Server_Parameter_Save(sServerParam *pServerParam);
BOOL Data_Logo_Attribute_Load(sLogoAttribute *pLogoAttribute);
BOOL Data_Logo_Attribute_Save(sLogoAttribute *pLogoAttribute);
BOOL Data_TouchCalibration_Load(void);
BOOL Data_TouchCalibration_Save(void);


bool Data_Device_UserInfor_Save(sDeviceUserInfor *pDeviceUserInfor);
bool Data_Device_UserInfor_Load(sDeviceUserInfor *pDeviceUserInfor);
bool Data_Record_InforMap_Load(sRecordInforMap *pRecordInforMap);
bool Data_Record_InforMap_Save(sRecordInforMap *pRecordInforMap);
bool Data_Total_InhaleTime_Counter_Load(float *fInhaleTimeCounter);
bool Data_Total_InhaleTime_Counter_Save(float *fInhaleTimeCounter);
bool Data_Total_Test_Counter_Load(unsigned long *nTotalTestCounter);
bool Data_Total_Test_Counter_Save(unsigned long *nTotalTestCounter);
bool Data_Already_Upload_Counter_Load(unsigned long *nAlreadyUpLoadIndex);
bool Data_Already_Upload_Counter_Save(unsigned long *nAlreadyUpLoadIndex);

bool Data_Police_User_Infor_Insert_Save(sPoliceUserInfor *pPoliceUserInfor);
bool Data_Police_User_Infor_Load_ByID(unsigned short nID, sPoliceUserInfor *pPoliceUserInfor);

bool Data_Police_Last_User_Infor_Load(sPoliceUserInfor *pPoliceUserInfor);
bool Data_Police_Last_User_Infor_Save(sPoliceUserInfor *pPoliceUserInfor);

bool Data_Record_Load_ByID(unsigned long nRecordID, sRecord *pRecord);
bool Data_Record_Load_Last(sRecord *pRecord);
bool Data_Record_Save_Sub(sRecord *pRecord);
void Data_Record_Full_Save(void);
void Data_Records_Clear(void);

#if SERVER_USE_XING_JIKONG_GU
bool Data_HeartRate_Parameter_Load(sHeartRateParam *pHeartRateParam);//added 
bool Data_HeartRate_Parameter_Save(sHeartRateParam *pHeartRateParam);

#endif

#endif

