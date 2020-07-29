#ifndef _GPRS_USER_H_
#define _GPRS_USER_H_

#include "platform.h"

/*上传协议包版本*/
#define GPRS_DATA_PACKAGE_VERSION   			"A"
/*GPRS图片数据包每包携带的有效数据大小,本3G模块每次AT只允许1500K*/
#define GPRS_IMAGE_PACKAGE_VALID_DATA_SIZE	512
/*空闲自动关机超时*/
#define GPRS_AUTOSHUTDOWN_TIMEOUT			(3 * 60000)

typedef enum
{
	eGprsUserNothing,
	eGprsUserGetIMEI,
	eGprsUserGetRecordData,
	eGprsUserLoginServer,
	eGprsUserUploadRecordData,
	eGprsUserConfirmFinishTransmit,
}eGprsUserState;

/*用户区间调用的函数--开始*/
void Upload_Record_Var_Init(void);
void Exec_GPRS_User_Loop(void);
signed char Get_GPRS_SingalQuality(void);
/*用户区间调用的函数--结束*/


/*内部使用的函数--开始*/
void Notify_User_Invalid_Simcard(void);
void Notify_User_Deive_IMEI_NeedRegister(void);
char Get_Net_AccessMode(void);
char * Get_Net_RemoteIP(void);
char * Get_Net_RemoteURL(void);
unsigned short Get_Net_RemotePort(void);
bool Gprs_Get_UserLogin_State(void);
void Gprs_Set_UserLogin_State(bool bNewState);
char * Get_Hardware_IMEI(void);
void Save_Hardware_IMEI(char *aString);
void GprsUser_Switch_State(eGprsUserState eUserState);
void Gprs_Analyse_Server_Package_Data(char *pBuffer ,unsigned short nLen);
void Gprs_Analyse_Server_Package_Data_Sub(char *pBuffer ,unsigned short nLen);
void Update_Already_Upload_Record_Counter(void);
/*内部使用的函数--结束*/
unsigned short  Get_GPRS_To_Server_Record_Packet(char *pPackageBuffer);

#if SERVER_USE_XING_JIKONG_GU
unsigned short  Get_GPRS_To_Server_Result_Packet(char *pPackageBuffer);//added by lxl 20180814
void GPRS_HeartRate_Packet_Upload(void);
void Device_Close_Packet_Upload(void);
#endif

#endif

