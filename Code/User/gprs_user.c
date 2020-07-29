#include "platform.h"
#include "parameter.h"
#include "data.h"
#include "app.h"
#include "spi_flash.h"
#include "delay.h"
#include "system_misc.h"
#include "rtc.h"
#include "windows.h"
#include "gprs.h"
#include "gprs_user.h"
#include "gps.h"


#include <stdlib.h>  
#include <stdio.h>  
#include <string.h>  
#include "calibrate.h"


#if USE_GPRS

typedef struct
{	
	bool bGetValidIMEI;
	bool bAleadyLogin;
	bool bAleadyFinishUpload;

	/*与服务器通信自然序列号*/
	unsigned long nPacketIndex;

	bool bSwitchNewState;
	unsigned char nRetryCnt;
	eGprsUserState eUserState;
	unsigned long nStartWaitTime;

	/*不包括首尾,CMD,DATA LENGTH描述字节.*/
	int nCurrentSendPacketSize;	
	bool bIsReveiceAckFromServer;

	/*记录相关*/
	sRecord sRecord;
	unsigned short nRecordDataLength;
	/*照片相关*/
	unsigned char nImagePacketIndex;
	unsigned char nTotalImagePacketNumber;
	unsigned long nJpegImageSize;
	/*指纹相关*/
	unsigned char nFingerprintPacketIndex;
	unsigned short nFingerprintTemplateSize;
}__attribute__ ((packed))sGPRSUser;

static sGPRSUser g_sGPRSUser;

/*进入主循环前调用,开机调用一次即可.*/
void Upload_Record_Var_Init(void)
{
	memset(&g_sGPRSUser, 0, sizeof(g_sGPRSUser));
	g_sGPRSUser.bSwitchNewState = true;
	g_sGPRSUser.eUserState = eGprsUserGetIMEI;
	g_sGPRSUser.bGetValidIMEI = strlen(Get_Hardware_IMEI()) ? true : false;
}

signed char Get_GPRS_SingalQuality(void)
{
	return g_nGprsSingalQuality;
}

static bool _Is_GPRS_System_PoweOn(void)
{
	return g_bGprsPowerOnState;
}

static bool _Reset_Upload_State_And_Retry(const char *FileName, int nLineNumber)
{
	if(GPRS_Power_Control(eGPRSPowerRestart, FileName, nLineNumber))
	{
		GprsUser_Switch_State(eGprsUserGetIMEI);
		return true;
	}

	GprsUser_Switch_State(eGprsUserNothing);
	return false;
}

char * Get_Hardware_IMEI(void)
{
	return g_sSystemParam.aIMEI;
}

void Save_Hardware_IMEI(char *aString)
{
	g_sGPRSUser.bGetValidIMEI = true;
	if(memcmp(g_sSystemParam.aIMEI,aString, 15))
	{
		memset(g_sSystemParam.aIMEI, 0, sizeof(g_sSystemParam.aIMEI));
		memcpy(g_sSystemParam.aIMEI, aString, 15);
		Data_System_Parameter_Save(&g_sSystemParam);
		Data_System_Parameter_Load(&g_sSystemParam);
		/*当前条目重新取数据*/
		g_sGPRSUser.bSwitchNewState = true;
	}
}

char Get_Net_AccessMode(void)
{
	return g_sServerParam.nNetAccessMode;
}

unsigned short Get_Net_RemotePort(void)
{
	return g_sServerParam.nRemotePort;
}

char * Get_Net_RemoteIP(void)
{
	return g_sServerParam.aRemoteIP;
}

char * Get_Net_RemoteURL(void)
{
	return g_sServerParam.aRemoteURL;

}

void Gprs_Set_UserLogin_State(bool bNewState)
{	
	g_sGPRSUser.bAleadyLogin = bNewState;
}

bool Gprs_Get_UserLogin_State(void)
{	
	return g_sGPRSUser.bAleadyLogin;
}

void Gprs_Analyse_Server_Package_Data(char *pBuffer ,unsigned short nLen)
{
	//char aServerData[100];
	//TRACE_PRINTF("GPRS-->Receive Data from server!(Packet size = %d)\r\n",nLen);
	//TRACE_PRINTF("pBuffer = %s)\r\n",pBuffer);
	#if 0
	if(nLen < sizeof(aServerData))
	{
		memcpy(aServerData, pBuffer, nLen);
		Gprs_Analyse_Server_Package_Data_Sub(aServerData,nLen);
	}
	#else
	Gprs_Analyse_Server_Package_Data_Sub(pBuffer,nLen);
	#endif 
}

/*通知窗体记录已上传*/
void Inform_Window_Gprs_Upload_Successful(void)
{
	WM_HWIN hWin;
	hWin = GetWinHandleByWinType(eDisplayRecord);
	if(hWin)
	{
		WM_SendMessageNoPara(hWin, WM_NOTIFY_GPRS_UPLOAD_EVENT);
	}
}

void Update_Already_Upload_Record_Counter(void)
{
	g_nAlreadyUploadCounter++;
	if(g_nAlreadyUploadCounter > g_sRecordInforMap.nTotalRecordsCounter)
	{
		g_nAlreadyUploadCounter = g_sRecordInforMap.nTotalRecordsCounter;
	}
	Data_Already_Upload_Counter_Save(&g_nAlreadyUploadCounter);
	Data_Already_Upload_Counter_Load(&g_nAlreadyUploadCounter);
	TRACE_PRINTF("Finish upload record(g_nAlreadyUploadCounter = %d)!\r\n", g_nAlreadyUploadCounter);

	Inform_Window_Gprs_Upload_Successful();
}

void Notify_User_Invalid_Simcard(void)
{
	eUserWindow w;
	w = GetLastWinTypeFromList();
	if(w != eTestMain && w != ePrintRecord)
	{
		sDialogWindowProperty s;
		s.nWinType = eDialogOK;
		s.pFont = _GetFont(Font_Content);
		s.pContent = _InvalidSimCardTip;
		s.nContentColor = GUI_RED;
		s.nBackGroundColor = GUI_LIGHTGRAY;
		s.nAutoCloseTime = 5;
		s.pFunMiddleOK = NULL;
		CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
	}
}

void Notify_User_Deive_IMEI_NeedRegister(void)
{
	eUserWindow w;
	w = GetLastWinTypeFromList();
	if(w != eTestMain && w != ePrintRecord)
	{
		sDialogWindowProperty s;
		s.nWinType = eDialogOK;
		s.pFont = _GetFont(Font_Content);
		s.pContent = _DeviceIMEINotRegister;
		s.nContentColor = GUI_RED;
		s.nBackGroundColor = GUI_LIGHTGRAY;
		s.nAutoCloseTime = 10;
		s.pFunMiddleOK = NULL;
		CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
	}
}

static bool Is_ValidData_Need_Upload(void)
{
	return (g_nAlreadyUploadCounter < g_sRecordInforMap.nTotalRecordsCounter) ? true : false;
}

static bool Get_New_Upload_Record_Data(void)
{
	if(Is_ValidData_Need_Upload())
	{
		g_sGPRSUser.bAleadyFinishUpload = false;
		memset(&g_sGPRSUser.sRecord, 0, sizeof(g_sGPRSUser.sRecord));
		g_sGPRSUser.nJpegImageSize = 0;
		g_sGPRSUser.nFingerprintTemplateSize = 0;

		TRACE_PRINTF("Gprs-->Current prepare upload record index = %d !\r\n", g_nAlreadyUploadCounter + 1);
		
		Data_Record_Load_ByID(g_nAlreadyUploadCounter, &g_sGPRSUser.sRecord);

		return true;
	}

	return false;
}

#if SERVER_USE_KEYUN_DAI

void Gprs_Analyse_Server_Package_Data_Sub(char *pBuffer ,unsigned short nLen)
{
	char *pProcPtr;
	char aTemp[20];
	unsigned char nCmd;
	unsigned short nDataLength;
	sRecord *pRecord = &g_sGPRSUser.sRecord;
	
	if(pBuffer[0] == '[' && pBuffer[nLen - 1] == ']')
	{
		nCmd = pBuffer[1];
		switch(nCmd)
		{
			/*登陆应答包*/
			case 0x22:
				pProcPtr = pBuffer + 2;
				memcpy(&nDataLength, pProcPtr, 2);
				pProcPtr += 2;
				
				if(nDataLength < nLen)
				{
					/*第一段是IMEI,取出判断*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(nDataLength == 1)
					{
						switch(*pProcPtr)
						{
							case 'S':
								Gprs_Set_UserLogin_State(true);
								TRACE_PRINTF("GPRS-->Login server successful!\r\n");
								break;
								
							case 'F':
								Gprs_Set_UserLogin_State(false);
								GPRS_Power_Control(eGPRSPowerDownAlways, __FILE__, __LINE__);
								Notify_User_Deive_IMEI_NeedRegister();
								TRACE_PRINTF("GPRS-->Login server failed(IMEI is not registered in target server)!\r\n");
								break;
							default:
								TRACE_PRINTF("GPRS-->Login server unknown error = %d!\r\n",*pProcPtr);
								GPRS_Power_Control(eGPRSPowerDownAlways, __FILE__, __LINE__);
								goto _Error_Exit;
						}
					}
					else
					{
						TRACE_PRINTF("GPRS-->Cmd(0x21) Login state DataLength (DESCPRITION ERROR) = %d\r\n",nDataLength);
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*第二段是服务器时间*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(nDataLength == 12)
					{
					#if 0
						sDateTime s;
						memset(aTemp, 0 ,sizeof(aTemp));
						memcpy(aTemp, pProcPtr + 0, 2);
						s.wYear = atoi(aTemp);
						memcpy(aTemp, pProcPtr + 2, 2);
						s.wMonth = atoi(aTemp);
						memcpy(aTemp, pProcPtr + 4, 2);
						s.wDay= atoi(aTemp);
						memcpy(aTemp, pProcPtr + 6, 2);
						s.wHour= atoi(aTemp);
						memcpy(aTemp, pProcPtr + 8, 2);
						s.wMinute= atoi(aTemp);
						memcpy(aTemp, pProcPtr + 10, 2);
						s.wSecond= atoi(aTemp);
						Rtc_Set_Time(&s);
					#endif
					}
					else
					{
						TRACE_PRINTF("GPRS-->Cmd(0x21) Server time DataLength (DESCPRITION ERROR) = %d\r\n",nDataLength);
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;
				}
				else
				{
					TRACE_PRINTF("GPRS-->Cmd(0x21) DataLength (DESCPRITION ERROR) = %d\r\n",nDataLength);
				}
				break;

			/*数据上传完成确认包.*/
			case 0x1F:
				pProcPtr = pBuffer + 2;
				memcpy(&nDataLength, pProcPtr, 2);
				pProcPtr += 2;
				
				if(nDataLength < nLen)
				{
					/*第一段是IMEI,取出判断*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(nDataLength == 15)
					{
						if(memcmp(g_sSystemParam.aIMEI,pProcPtr, 15))
						{
							TRACE_PRINTF("GPRS-->Cmd(0x1F) IMEI doesn't match!\r\n");
							goto _Error_Exit;
						}
					}
					else
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) IMEI DataLength doesn't match(Expect size = %d , real size = %d)!\r\n", 15, nDataLength);
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*第二段是时间*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					sprintf (aTemp, "%02d%02d%02d%02d%02d%02d",
							 pRecord->aDateTime[0], pRecord->aDateTime[1],
							 pRecord->aDateTime[2], pRecord->aDateTime[3],
							 pRecord->aDateTime[4], pRecord->aDateTime[5] );
					if(nDataLength == 12)
					{
						if(memcmp(aTemp,pProcPtr, 12))
						{
							TRACE_PRINTF("GPRS-->Cmd(0x1F) Record time doesn't match!\r\n");
							goto _Error_Exit;
						}
					}
					else
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) Record time length doesn't match(Expect size = %d , real size = %d)!\r\n", 12, nDataLength);
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*第三段是酒精测试报告大小*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(g_sGPRSUser.nRecordDataLength != atoi(pProcPtr))
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) Record basic size doesn't match(Expect size = %d , real size = %d)!\r\n", g_sGPRSUser.nRecordDataLength, atoi(pProcPtr));
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*第四段是图片大小*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(g_sGPRSUser.nJpegImageSize != atoi(pProcPtr))
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) Record nJpegImageSize doesn't match(Expect size = %d , real size = %d)!\r\n", g_sGPRSUser.nJpegImageSize, atoi(pProcPtr));
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*第五段是指纹特征值大小*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(g_sGPRSUser.nFingerprintTemplateSize != atoi(pProcPtr))
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) Record nFingerprintTemplateSize doesn't match(Expect size = %d , real size = %d)!\r\n", g_sGPRSUser.nFingerprintTemplateSize, atoi(pProcPtr));
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					
					/*第六段是该人员在数据库中出现的次数*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					TRACE_PRINTF("nHistoryDrinkTimes = %d\r\n",atoi(pProcPtr));
					pProcPtr += nDataLength;

					g_sGPRSUser.bAleadyFinishUpload = true;
					Update_Already_Upload_Record_Counter();
					
					TRACE_PRINTF("GPRS-->Cmd(0x1F) Receive expect Ack(upload successful).\r\n");
				}
				else
				{
					TRACE_PRINTF("GPRS-->Cmd(0x1F) DataLength (DESCPRITION ERROR) = %d\r\n",nDataLength);
				}
				break;

			/*数据接收应答包*/
			case 0x10:
				pProcPtr = pBuffer + 2;
				memcpy(&nDataLength, pProcPtr, 2);
				pProcPtr += 2;
				
				if(nDataLength < nLen)
				{
					/*第一段是IMEI,取出判断*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(nDataLength == 15)
					{
						if(memcmp(g_sSystemParam.aIMEI,pProcPtr, 15))
						{
							TRACE_PRINTF("GPRS-->Cmd(0x10) IMEI doesn't match!\r\n");
							goto _Error_Exit;
						}
					}
					else
					{	
						TRACE_PRINTF("GPRS-->Cmd(0x1F) IMEI DataLength doesn't match(Expect size = %d , real size = %d)!\r\n", 15, nDataLength);
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*第二段是收到的字节长度*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if((g_sGPRSUser.nCurrentSendPacketSize - 5) != atoi(pProcPtr))
					{
						TRACE_PRINTF("GPRS-->Cmd(0x10) PacketSize doesn't match(Expect size = %d , real size = %d)!\r\n", g_sGPRSUser.nCurrentSendPacketSize, atoi(pProcPtr));
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					g_sGPRSUser.bIsReveiceAckFromServer = true;
					TRACE_PRINTF("GPRS-->Cmd(0x10) Receive expect Ack.\r\n");
				}
				else
				{
					TRACE_PRINTF("GPRS-->Cmd(0x10) DataLength (DESCPRITION ERROR) = %d\r\n",nDataLength);
				}
				
				break;
		}
	}

	return;

_Error_Exit:
	{
		int i;
		/*打印收到的数据*/
		for(i = 0; i < nLen; i++)
		{
			delay_us(100);
			TRACE_PRINTF("%02X  %c\r\n", pBuffer[i], (pBuffer[i] >= 0x20 && pBuffer[i] < 0x7F) ? pBuffer[i] : 0x20);
		}
	}
}

static void _Add_Item_Data(char **pBuffer, char *pSrcStr)
{
	int nLen = strlen(pSrcStr);
	memcpy(*pBuffer, (unsigned char *)&nLen, 2);
	memcpy(*pBuffer + 2, pSrcStr, nLen);
	*pBuffer += (nLen + 2);
}

static unsigned short Get_GPRS_To_Server_Login_Packet (char *pPackageBuffer)
{
	unsigned short nLen;
	char *pStart = NULL;
	char *pStr = pPackageBuffer;

	/*起始符*/
	*pStr++ = '[';

	/*登陆命令码*/
	*pStr++ = 0x21;

	/*Data length 占位*/
	pStr += 2;

	/*记录下有效数据包的起始长度*/
	pStart = pStr;

	/*1.IMEI数据段*/
	_Add_Item_Data(&pStr, Get_Hardware_IMEI());

	/*2.登陆数据包版本.*/
	_Add_Item_Data(&pStr, GPRS_DATA_PACKAGE_VERSION);

	/*3.软件版本号*/
	_Add_Item_Data(&pStr, FW_VERSION);

	/*结束符*/
	*pStr++ = ']';

	/*计算数据长度,并写入.*/
	nLen = pStr - pStart - 1;
	memcpy(pPackageBuffer + 2, (unsigned char *)&nLen, 2);

	return (pStr - pPackageBuffer);
}

static unsigned short Get_GPRS_To_Server_Record_Packet(char *pPackageBuffer)
{
	unsigned short nLen;
	char *pStart = NULL;
	char *pStr = pPackageBuffer;
	char aTemp[100];
	sRecord *pRecord = &g_sGPRSUser.sRecord;
	float fAlcUnitConvertValue;

	/*起始符*/
	*pStr++ = '[';

	/*上传记录命令码*/
	*pStr++ = 0x1A;

	/*Data length 占位*/
	pStr += 2;

	/*记录下有效数据包的起始长度*/
	pStart = pStr;

	/*1.IMEI数据段*/
	_Add_Item_Data(&pStr, Get_Hardware_IMEI());

	/*2.记录时间日期*/
	sprintf (aTemp, "%02d%02d%02d%02d%02d%02d",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5] );
	_Add_Item_Data(&pStr, aTemp);

	/*3.登陆数据包版本.*/
	_Add_Item_Data(&pStr, GPRS_DATA_PACKAGE_VERSION);

	/*4.仪器编号*/
	sprintf(aTemp, "%s%06ld", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	_Add_Item_Data(&pStr, aTemp);

	/*5.数据上传接口.3G模块*/
	_Add_Item_Data(&pStr, "3");

	/*6.测试序号*/
	sprintf (aTemp, "%08d", pRecord->nRecordIndex);
	_Add_Item_Data(&pStr, aTemp);

	/*7.测试模式*/
	sprintf (aTemp, "%d", pRecord->nTestMode);
	_Add_Item_Data(&pStr, aTemp);

	/*GPRS定位信息段*/
	if(pRecord->fLongitude != 0 && pRecord->fLatitude != 0)
	{
		/*8.GPS是否有效*/
		_Add_Item_Data(&pStr, "A");

		/*9.维度值*/
		sprintf(aTemp, "%f", pRecord->fLatitude >= 0 ? pRecord->fLatitude : -(pRecord->fLatitude));
		_Add_Item_Data(&pStr, aTemp);

		/*10.维度标识,北N,南S*/
		sprintf(aTemp, "%s", pRecord->fLatitude > 0 ? "N" : "S");
		_Add_Item_Data(&pStr, aTemp);

		/*11.经度值*/
		sprintf(aTemp, "%f", pRecord->fLongitude >= 0 ? pRecord->fLongitude : -(pRecord->fLongitude));
		_Add_Item_Data(&pStr, aTemp);

		/*12.经度标识,东E,西W*/
		sprintf(aTemp, "%s", pRecord->fLongitude > 0 ? "E" : "W");
		_Add_Item_Data(&pStr, aTemp);
	}
	else
	{
		/*8.GPS是否有效*/
		_Add_Item_Data(&pStr, "V");

		/*9.维度值*/
		_Add_Item_Data(&pStr, "");

		/*10.维度标识,北N,南S*/
		_Add_Item_Data(&pStr, "");

		/*11.经度值*/
		_Add_Item_Data(&pStr, "");

		/*12.经度标识,东E,西W*/
		_Add_Item_Data(&pStr, "");
	}

	/*13.测试温度*/
	sprintf (aTemp, "%d", pRecord->nTemperature);
	_Add_Item_Data(&pStr, aTemp);

	/*14.转换系数*/
	sprintf (aTemp, "%d", pRecord->nKBrACBAC);
	_Add_Item_Data(&pStr, aTemp);

	/*15.酒精测试结果,带单位结果的字符串.*/
	fAlcUnitConvertValue = Get_Final_Convert_Display_Result(pRecord->fAlcValue, pRecord->nTestUnit, pRecord->nKBrACBAC);
#if USE_GB_T21254_2017
	Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, aTemp,eFormatToInt);
#else
	Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, aTemp,eFormatToFloat);
#endif
	strcat(aTemp, Get_Unit_String(pRecord->nTestUnit));
	_Add_Item_Data(&pStr, aTemp);

	/*16.酒精测试值,mg/100ml为单位的数值字符串*/
	sprintf (aTemp, "%f", pRecord->fAlcValue);
	_Add_Item_Data(&pStr, aTemp);

	/*17.驾驶证号*/
	_Add_Item_Data(&pStr, (char *)pRecord->aDriverLicense);

	/*18.测试人姓名*/
	_Add_Item_Data(&pStr, (char *)pRecord->aTestee);

	/*19.测试地点*/
	_Add_Item_Data(&pStr, (char *)pRecord->aLocation);

	/*20.车牌号*/
	_Add_Item_Data(&pStr, (char *)pRecord->aPlateNumber);

	/*21.执勤民警姓名*/
	_Add_Item_Data(&pStr, (char *)pRecord->aPoliceName);

	/*22.执勤民警警号*/
	_Add_Item_Data(&pStr, (char *)pRecord->aPoliceID);

	/*23.所属单位*/
	_Add_Item_Data(&pStr, (char *)pRecord->aDepartment);

	/*结束符*/
	*pStr++ = ']';

	/*计算数据长度,并写入.*/
	nLen = pStr - pStart - 1;
	memcpy(pPackageBuffer + 2, (unsigned char *)&nLen, 2);

	return (pStr - pPackageBuffer);
}

static unsigned short Get_GPRS_To_Server_Finish_Confirm_Packet (char *pPackageBuffer)
{
	unsigned short nLen;
	char *pStart = NULL;
	char *pStr = pPackageBuffer;
	char aTemp[100];
	sRecord *pRecord = &g_sGPRSUser.sRecord;
	
	/*起始符*/
	*pStr++ = '[';

	/*上传指纹信息命令码*/
	*pStr++ = 0x1E;

	/*Data length 占位*/
	pStr += 2;

	/*记录下有效数据包的起始长度*/
	pStart = pStr;

	/*1.IMEI数据段*/
	_Add_Item_Data(&pStr, Get_Hardware_IMEI());

	/*2.记录时间日期*/
	sprintf (aTemp, "%02d%02d%02d%02d%02d%02d",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5] );
	_Add_Item_Data(&pStr, aTemp);

	/*3.酒精测试数据包大小.*/
	sprintf(aTemp, "%d",g_sGPRSUser.nRecordDataLength);
	_Add_Item_Data(&pStr, aTemp);

	/*4.照片文件大小.*/
	sprintf(aTemp, "%d",g_sGPRSUser.nJpegImageSize);
	_Add_Item_Data(&pStr, aTemp);

	/*5.指纹特征文件大小.*/
	sprintf(aTemp, "%d",g_sGPRSUser.nFingerprintTemplateSize);
	_Add_Item_Data(&pStr, aTemp);
	
	/*结束符*/
	*pStr++ = ']';

	/*计算数据长度,并写入.*/
	nLen = pStr - pStart - 1;
	memcpy(pPackageBuffer + 2, (unsigned char *)&nLen, 2);

	return (pStr - pPackageBuffer);
}

void GprsUser_Switch_State(eGprsUserState eUserState)
{
	g_sGPRSUser.nRetryCnt = 3;
	g_sGPRSUser.eUserState = eUserState;
	g_sGPRSUser.bSwitchNewState = true;
	g_sGPRSUser.nStartWaitTime = Get_System_Time();
	g_sGPRSUser.bIsReveiceAckFromServer = false;
}

void GprsUser_SendData_Timeout_Handle(const char *FileName, int nLineNumber)
{
	if(g_sGPRSUser.nRetryCnt)
	{	
		g_sGPRSUser.nRetryCnt--;
		g_sGPRSUser.bSwitchNewState = true;
	}
	else
	{
		_Reset_Upload_State_And_Retry(FileName, nLineNumber);
	}
}

/*主循环中调用*/
void Exec_GPRS_User_Loop(void)
{
	GPRS_Exce();

	switch(g_sGPRSUser.eUserState)
	{
		case eGprsUserGetIMEI:
			if(g_sGPRSUser.bGetValidIMEI)
			{
				if(Is_ValidData_Need_Upload() == false && _Is_GPRS_System_PoweOn())
				{
					GPRS_Power_Control(eGPRSPowerDownNormal, __FILE__, __LINE__);
				}
				GprsUser_Switch_State(eGprsUserGetRecordData);
			}
			else
			{
				if(_Is_GPRS_System_PoweOn() == false)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
				}
			}
			break;

		case eGprsUserGetRecordData:
			if(Is_ValidData_Need_Upload())
			{
				if(_Is_GPRS_System_PoweOn() == false)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
				}
				if(Get_New_Upload_Record_Data())
				{
					GprsUser_Switch_State(eGprsUserLoginServer);
				}
			}
			else if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + GPRS_AUTOSHUTDOWN_TIMEOUT)
			{
				if(_Is_GPRS_System_PoweOn())
				{
					GPRS_Power_Control(eGPRSPowerDownNormal, __FILE__, __LINE__);
				}
			}
			break;

		case eGprsUserLoginServer:
			if(Gprs_Get_UserLogin_State())
			{
				GprsUser_Switch_State(eGprsUserUploadRecordData);
				break;
			}
			
			if(g_sGPRSUser.bSwitchNewState)
			{
				g_sGPRSUser.bSwitchNewState = false;
				g_sGPRSUser.nStartWaitTime = Get_System_Time();
				g_nGprs_Tx_Cnt = Get_GPRS_To_Server_Login_Packet(GPRS_Tx_Buffer);
				TRACE_PRINTF("GPRS-->Get_GPRS_To_Server_Login_Packet! g_nGprs_Tx_Cnt = %d\r\n",g_nGprs_Tx_Cnt);
			}

			if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + (Get_Gprs_Tcp_Connect_State() ? 30000 : 110000))
			{
				GprsUser_SendData_Timeout_Handle(__FILE__, __LINE__);
			}
			else if(g_nGprs_Tx_Cnt == 0)
			{
				if(Gprs_Get_UserLogin_State())
				{
					GprsUser_Switch_State(eGprsUserUploadRecordData);
				}
			}
			break;

		case eGprsUserUploadRecordData:
			if(g_sGPRSUser.bSwitchNewState)
			{
				g_sGPRSUser.bSwitchNewState = false;
				g_sGPRSUser.nStartWaitTime = Get_System_Time();
				g_nGprs_Tx_Cnt = Get_GPRS_To_Server_Record_Packet(GPRS_Tx_Buffer);
				g_sGPRSUser.nCurrentSendPacketSize = g_nGprs_Tx_Cnt;
				g_sGPRSUser.nRecordDataLength = g_nGprs_Tx_Cnt - 5;
				TRACE_PRINTF("GPRS-->Get_GPRS_To_Server_Record_Packet! g_nGprs_Tx_Cnt = %d\r\n",g_nGprs_Tx_Cnt);
			}

			if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + 30000)
			{
				GprsUser_SendData_Timeout_Handle(__FILE__, __LINE__);
			}
			else if(g_nGprs_Tx_Cnt == 0)
			{
				if(g_sGPRSUser.bIsReveiceAckFromServer)
				{
					GprsUser_Switch_State(eGprsUserConfirmFinishTransmit);
				}
			}
			break;
		
		case eGprsUserConfirmFinishTransmit:
			if(g_sGPRSUser.bSwitchNewState)
			{
				g_sGPRSUser.bSwitchNewState = false;
				g_sGPRSUser.nStartWaitTime = Get_System_Time();
				g_nGprs_Tx_Cnt = Get_GPRS_To_Server_Finish_Confirm_Packet(GPRS_Tx_Buffer);
				g_sGPRSUser.nCurrentSendPacketSize = g_nGprs_Tx_Cnt;
				TRACE_PRINTF("GPRS-->Get_GPRS_To_Server_Finish_Confirm_Packet! g_nGprs_Tx_Cnt = %d\r\n",g_nGprs_Tx_Cnt);
			}

			if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + 15000)
			{
				GprsUser_SendData_Timeout_Handle(__FILE__, __LINE__);
			}
			else if(g_nGprs_Tx_Cnt == 0)
			{
				if(g_sGPRSUser.bAleadyFinishUpload)
				{
					/*取下一条数据.*/
					GprsUser_Switch_State(eGprsUserGetRecordData);
					TRACE_PRINTF("Upload successful, get next need upload data!\r\n");
				}
			}
			break;
	}
}
#elif SERVER_USE_KEYUN_YE_NEW

void Gprs_Analyse_Server_Package_Data_Sub(char *pBuffer ,unsigned short nLen)
{
	unsigned char nCmd;
	if(pBuffer[0] == '[' && pBuffer[nLen - 1] == ']')
	{
		nCmd = pBuffer[1];
		switch(nCmd)
		{
			/*登陆应答包*/
			case 0x22:
				Gprs_Set_UserLogin_State(true);
				TRACE_PRINTF("GPRS-->Login server successful!\r\n");
				break;

			/*基础数据上传完成确认包.*/
			case 0x1B:
				g_sGPRSUser.bIsReveiceAckFromServer = true;
				break;

			/*照片上传回应包.有3个子命令*/
			case 0x4C:
				break;
			
			/*指纹数据上传完成确认包.*/
			case 0x4E:
				g_sGPRSUser.bIsReveiceAckFromServer = true;
				break;
					
			default:
				goto _Error_Exit;
		}
	}
	else
	{
			goto _Error_Exit;
	}

_Error_Exit:
	{
		int i;
		/*打印收到的数据*/
		for(i = 0; i < nLen; i++)
		{
			delay_us(100);
			TRACE_PRINTF("%02X  %c\r\n", pBuffer[i], (pBuffer[i] >= 0x20 && pBuffer[i] < 0x7F) ? pBuffer[i] : 0x20);
		}
	}
}

static unsigned short Get_GPRS_To_Server_Login_Packet (char *pPackageBuffer)
{
	unsigned short nLen;
	char *pStart = NULL;
	char *pStr = pPackageBuffer;

	/*起始符*/
	*pStr++ = '[';

	/*登陆命令码*/
	*pStr++ = 0x21;

	/*Packet包序列号*/
	sprintf(pStr, "%05d", g_sGPRSUser.nPacketIndex++);
	pStr += 5;

	/*Data length 占位*/
	pStr += 2;

	/*记录下有效数据包的起始长度*/
	pStart = pStr;

	/*数据段括号*/
	*pStr++ = '(';

	/*1.IMEI数据段*/
	memcpy(pStr, Get_Hardware_IMEI(), 15);
	pStr += 15;
	*pStr++ = ',';

	/*2.版本号*/
	sprintf ( pStr, "V1.0" );
	pStr += 4;
	*pStr++ = ',';

	/*3.MCC,目前固定为中国移动MCC码*/
	sprintf(pStr,"%03d",86);
	pStr += 3;

	/*数据段括号*/
	*pStr++ = ')';
	
	/*结束符*/
	*pStr++ = ']';

	/*计算数据长度,并写入.*/
	nLen = pStr - pStart - 1;
	memcpy(pStart - 2, (unsigned char *)&nLen, 2);

	return (pStr - pPackageBuffer);
}

static unsigned short Get_GPRS_To_Server_Record_Packet(char *pPackageBuffer)
{
	unsigned short nLen;
	char *pStart = NULL;
	char *pStr = pPackageBuffer;
	sRecord *pRecord = &g_sGPRSUser.sRecord;

	/*起始符*/
	*pStr++ = '[';

	/*上传记录命令码*/
	*pStr++ = 0x1A;

	/*Packet包序列号*/
	sprintf(pStr, "%05d", g_sGPRSUser.nPacketIndex++);
	pStr += 5;

	/*Data length 占位*/
	pStr += 2;

	/*记录下有效数据包的起始长度*/
	pStart = pStr;

	/*数据段括号*/
	*pStr++ = '(';

	/*1.仪器编号*/
	sprintf(pStr, "%s%06d", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr += 8;
	*pStr++ = 0x00;

	/*2.记录时间日期*/
	sprintf (pStr, "%02d%02d%02d%02d%02d%02d",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5]);
	pStr += 12;
	*pStr++ = 0x00;

	/*3.测试序号*/
	sprintf (pStr, "%08d", pRecord->nRecordIndex);
	pStr += 8;
	*pStr++ = 0x00;

	/*4.测试模式*/
	sprintf (pStr, "%d", pRecord->nTestMode);
	pStr += 1;
	*pStr++ = 0x00;

	/*GPRS定位信息段*/
	if(pRecord->fLongitude != 0 && pRecord->fLatitude != 0)
	{
		/*5.GPS是否有效*/
		*pStr++ = 'A';
		*pStr++ = 0x00;

		/*6*/
		sprintf(pStr, "%f", pRecord->fLatitude >= 0 ? pRecord->fLatitude : -(pRecord->fLatitude));
		pStr += strlen(pStr);
		*pStr++ = 0x00;

		/*7*/
		*pStr++ = pRecord->fLatitude > 0 ? 'N' : 'S';
		*pStr++ = 0x00;

		/*8*/
		sprintf(pStr, "%f", pRecord->fLongitude >= 0 ? pRecord->fLongitude : -(pRecord->fLongitude));
		pStr += strlen(pStr);
		*pStr++ = 0x00;

		/*9*/
		*pStr++ = pRecord->fLongitude > 0 ? 'E' : 'W';
		*pStr++ = 0x00;
	}
	else
	{
		/*5.GPS是否有效*/
		*pStr++ = 'V';
		*pStr++ = 0x00;
		/*6*/
		*pStr++ = 0x00;
		/*7*/
		*pStr++ = 0x00;
		/*8*/
		*pStr++ = 0x00;
		/*9*/
		*pStr++ = 0x00;
	}

	//字段10: 测试温度 ----- 不定长，（N_STRING）,如27.5 表示27.5摄氏度
	sprintf ( pStr, "%d", pRecord->nTemperature );
	pStr += strlen ( pStr );
	*pStr++ = 0x00; //字段分隔符

	//字段11：酒精测试结果 ----- 不定长，（N_STRING），g.g格式单位mg/100ml，如：34.3表示34.3mg/100ml
#if USE_GB_T21254_2017
	Get_Format_Alc_Convert_String(pRecord->fAlcValue, 0, pStr,eFormatToInt);
#else
	Get_Format_Alc_Convert_String(pRecord->fAlcValue, 0, pStr,eFormatToFloat);
#endif
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	//字段12：驾驶证号
	strncpy(pStr, (char *)pRecord->aDriverLicense, sizeof(pRecord->aDriverLicense));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	//字段13: 测试人姓名
	strncpy(pStr, (char *)pRecord->aTestee, sizeof(pRecord->aTestee));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	//字段14：测试地点
	strncpy(pStr, (char *)pRecord->aLocation, sizeof(pRecord->aLocation));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	//字段15：车牌号
	strncpy(pStr, (char *)pRecord->aPlateNumber, sizeof(pRecord->aPlateNumber));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	//字段16：执勤民警姓名
	strncpy(pStr, (char *)pRecord->aPoliceName, sizeof(pRecord->aPoliceName));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	//字段17：执勤民警警号
	strncpy(pStr, (char *)pRecord->aPoliceID, sizeof(pRecord->aPoliceID));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	//字段18：所属单位
	strncpy(pStr, (char *)pRecord->aDepartment, sizeof(pRecord->aDepartment));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //字段分隔符

	/*数据段括号*/
	*pStr++ = ')';

	/*结束符*/
	*pStr++ = ']';

	/*计算数据长度,并写入.*/
	nLen = pStr - pStart - 1;
	memcpy(pStart - 2, (unsigned char *)&nLen, 2);

	return (pStr - pPackageBuffer);
}


void GprsUser_Switch_State(eGprsUserState eUserState)
{
	g_sGPRSUser.nRetryCnt = 3;
	g_sGPRSUser.eUserState = eUserState;
	g_sGPRSUser.bSwitchNewState = true;
	g_sGPRSUser.nStartWaitTime = Get_System_Time();
	g_sGPRSUser.bIsReveiceAckFromServer = false;
}

void GprsUser_SendData_Timeout_Handle(const char *FileName, int nLineNumber)
{
	if(g_sGPRSUser.nRetryCnt)
	{	
		g_sGPRSUser.nRetryCnt--;
		g_sGPRSUser.bSwitchNewState = true;
	}
	else
	{
		_Reset_Upload_State_And_Retry(FileName, nLineNumber);
	}
}

/*主循环中调用*/
void Exec_GPRS_User_Loop(void)
{
	GPRS_Exce();

	switch(g_sGPRSUser.eUserState)
	{
		case eGprsUserGetIMEI:
			if(g_sGPRSUser.bGetValidIMEI)
			{
				if(Is_ValidData_Need_Upload() == false && _Is_GPRS_System_PoweOn())
				{
					GPRS_Power_Control(eGPRSPowerDownNormal, __FILE__, __LINE__);
				}
				GprsUser_Switch_State(eGprsUserGetRecordData);
			}
			else
			{
				if(_Is_GPRS_System_PoweOn() == false)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
				}
			}
			break;

		case eGprsUserGetRecordData:
			if(Is_ValidData_Need_Upload())
			{
				if(_Is_GPRS_System_PoweOn() == false)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
				}
				if(Get_New_Upload_Record_Data())
				{
					GprsUser_Switch_State(eGprsUserLoginServer);
				}
			}
			else if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + GPRS_AUTOSHUTDOWN_TIMEOUT)
			{
				if(_Is_GPRS_System_PoweOn())
				{
					GPRS_Power_Control(eGPRSPowerDownNormal, __FILE__, __LINE__);
				}
			}
			break;

		case eGprsUserLoginServer:
			if(Gprs_Get_UserLogin_State())
			{
				GprsUser_Switch_State(eGprsUserUploadRecordData);
				break;
			}
			
			if(g_sGPRSUser.bSwitchNewState)
			{
				g_sGPRSUser.bSwitchNewState = false;
				
				g_sGPRSUser.nStartWaitTime = Get_System_Time();
				g_nGprs_Tx_Cnt = Get_GPRS_To_Server_Login_Packet(GPRS_Tx_Buffer);
				TRACE_PRINTF("GPRS-->Get_GPRS_To_Server_Login_Packet! g_nGprs_Tx_Cnt = %d\r\n",g_nGprs_Tx_Cnt);
			}

			if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + (Get_Gprs_Tcp_Connect_State() ? 30000 : 110000))
			{
				GprsUser_SendData_Timeout_Handle(__FILE__, __LINE__);
			}
			else if(g_nGprs_Tx_Cnt == 0)
			{
				if(Gprs_Get_UserLogin_State())
				{
					GprsUser_Switch_State(eGprsUserUploadRecordData);
				}
			}
			break;

		case eGprsUserUploadRecordData:
			if(g_sGPRSUser.bSwitchNewState)
			{
				g_sGPRSUser.bSwitchNewState = false;
				g_sGPRSUser.nStartWaitTime = Get_System_Time();
				g_nGprs_Tx_Cnt = Get_GPRS_To_Server_Record_Packet(GPRS_Tx_Buffer);
				g_sGPRSUser.nCurrentSendPacketSize = g_nGprs_Tx_Cnt;
				TRACE_PRINTF("GPRS-->Get_GPRS_To_Server_Record_Packet! g_nGprs_Tx_Cnt = %d\r\n",g_nGprs_Tx_Cnt);
			}

			if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + 30000)
			{
				GprsUser_SendData_Timeout_Handle(__FILE__, __LINE__);
			}
			else if(g_nGprs_Tx_Cnt == 0)
			{
				if(g_sGPRSUser.bIsReveiceAckFromServer)
				{
					Update_Already_Upload_Record_Counter();
					GprsUser_Switch_State(eGprsUserGetRecordData);
				}
			}
			break;
	}
}

#elif SERVER_USE_XING_JIKONG_GU

#define BUFFER_SIZE     700   //added by lxl 20180816

typedef enum
{
	OK,
       FAIL
}eGprsSendState;

char*  Analyse_Server_Package_GrandSub(char *pBuffer)
{
	char *SearchWord=pBuffer;
	char *Response_Position=NULL;
	
	SearchWord=strstr(pBuffer,"\r\n");//HTTP/1.1 200 
	SearchWord+=2;			
	SearchWord=strstr(SearchWord,"\r\n");//Content-Type: application/json;charset=UTF-8
	SearchWord+=2;
	SearchWord=strstr(SearchWord,"\r\n");//Transfer-Encoding: chunked
	SearchWord+=2;
	SearchWord=strstr(SearchWord,"\r\n");//Date: Thu, 16 Aug 2018 02:20:05 GMT
	SearchWord+=2;
	SearchWord=strstr(SearchWord,"\r\n");//空行
	SearchWord+=2;//第6行 数据包长度
	SearchWord=strstr(SearchWord,"\r\n");//应答包数据长度
	SearchWord+=2;//应答具体的数据包

	Response_Position=SearchWord;

	return Response_Position;//返回结果数据包位置 20180816

}
void Gprs_Analyse_Server_Package_Data_Sub(char *pBuffer ,unsigned short nLen)
{
	char *Data_Position=NULL;
	char *REMARK=NULL;
	char *TYPE=NULL;
	eGprsSendState Sendstate;
	
	if(!strstr(pBuffer,"HTTP"))//如果数据包中不包含HTTP，这位无效数据包,则不执行下面的操作 
	{
		TRACE_PRINTF("Empty Packet!\r\n");
		return;
	}
	
	Data_Position=Analyse_Server_Package_GrandSub(pBuffer);
	REMARK=strstr(Data_Position,"REMARK");//找到应答数据包
	if(strstr(Data_Position,"OK"))
	{
		Sendstate=OK;
		TYPE=strstr(Data_Position,"TYPE");
	}
	else
	{
		REMARK=strstr(Data_Position,"REMARK");//找到错误详述位置
		Sendstate=FAIL;
	}
	
	switch(Sendstate)
	{
		case OK:		
		        TRACE_PRINTF("TYPE:%d\r\n",*(TYPE+6)-'0');

			if((*(TYPE+6)-'0')==2) //酒精值包
				g_sXingJiKongGU.bResult_Packet_Flag=false;//g_Result_Packet_Flag=false;//结果数据包上传完毕

				
			if((*(TYPE+6)-'0')==1)
			{
				g_sGPRSUser.bIsReveiceAckFromServer=true;//测试记录的整包上传完毕,开机记录已上传的记录数 added by lxl 20180815
				g_sXingJiKongGU.bReveiceAckFromServer=false;//g_ReveiceAckFromServer=false; //测试记录的整包上传完毕
			}

			if((*(TYPE+6)-'0')==3) //心跳应答包
				g_sXingJiKongGU.bHeartRate=false;
			
		break;

		case FAIL:
			TRACE_PRINTF("REMARK:%s\r\n",REMARK);
		break;
	}
	
}
//只发送测试结果 20180814
char KeyPassword[12]="\"123456\"";
char username[10]="\"Test\"";
char VERSION[10]="\"A\"";
static unsigned short Result_DataPacket(char *pPackageBuffer)
{
	char *pStr = pPackageBuffer;
	char aResult[20];
	float fTemp;
	 float fLongitude=0,fLatitude=0;
	
	*pStr++='{';
	sprintf(pStr,"\"alcohol\":%c",'{');
	pStr+=strlen(pStr);

	/*记录下有效数据包的起始长度*/
	//pStart = pStr;
	 /*1.用户名*/
	sprintf(pStr, "\"USER\":%s,", username);
	pStr+=strlen(pStr);

	/*2.登录密码*/
	sprintf(pStr, "\"KEY\":%s,", KeyPassword);
	pStr+=strlen(pStr);

	/*3.JSON的版本*/
	sprintf (pStr, "\"VERSION\":%s,", VERSION);
	pStr+=strlen(pStr);

	/*4.设备编号字符串*/
	sprintf(pStr, "\"DEVICE\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);


	/*5.记录的编号*/ 
	sprintf (pStr, "\"RECORDID\":%08d,", g_sRecord.nRecordIndex);
	pStr+=strlen(pStr);

   	
	/*6.日期*/
	sprintf (pStr, "\"DATE\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 g_sRecord.aDateTime[0], g_sRecord.aDateTime[1],
			 g_sRecord.aDateTime[2], g_sRecord.aDateTime[3],
			 g_sRecord.aDateTime[4], g_sRecord.aDateTime[5]);
	pStr+=strlen(pStr);

     
	 /*7.测试模式*/
	sprintf (pStr, "\"MODE\":\"%d\",",g_sRecord.nTestMode);
	pStr+=strlen(pStr);

	 /*8.测试温度*/
	sprintf (pStr, "\"TEMP\":%d,", g_sRecord.nTemperature);
	pStr+=strlen(pStr);

	 /*9.酒精浓度字符串*/
	fTemp=Get_Final_Convert_Display_Result(g_sRecord.fAlcValue,g_sRecord.nTestUnit, g_sRecord.nKBrACBAC);
	Get_Format_Alc_Convert_String(fTemp, g_sRecord.nTestUnit,aResult,eFormatToInt); //eFormatToFloat modified on 20181106
	sprintf (pStr, "\"ALC\":%s,", aResult);
	pStr+=strlen(pStr);

	/*10.测试单位*/
	sprintf(pStr, "\"UNIT\":\"%s\",",Get_Unit_String(g_sRecord.nTestUnit));
	pStr+=strlen(pStr);

	/*11.呼气浓度与血液浓度转换系数*/
	sprintf(pStr, "\"K\":%d,",g_sRecord.nKBrACBAC);
	pStr+=strlen(pStr);

	/*12.酒精浓度字符串描述*/
	sprintf(pStr, "\"ALCSTRING\":\"%s%s\",",aResult,Get_Unit_String(g_sRecord.nTestUnit));
	pStr+=strlen(pStr);

	
	if(g_sRecord.fLongitude != 0 && g_sRecord.fLatitude != 0)
	{
	/*13.经度数据*/
	fLongitude=g_sRecord.fLongitude >= 0 ? g_sRecord.fLongitude : -(g_sRecord.fLongitude);
	sprintf(pStr, "\"LONGITUDE\":%f,",fLongitude);
	pStr+=strlen(pStr);
	
	/*14.纬度数据*/
	fLatitude=g_sRecord.fLatitude >= 0 ? g_sRecord.fLatitude : -(g_sRecord.fLatitude);
	sprintf(pStr, "\"Latitude\":%f,",fLatitude);
	pStr+=strlen(pStr);
	}
	else
	{
		/*13.经度数据*/
		sprintf(pStr, "\"LONGITUDE\":%s,","null");
		pStr+=strlen(pStr);
	
		/*14.纬度数据*/
		sprintf(pStr, "\"Latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*15.测试地点*/
	sprintf(pStr, "\"LOCATION\":\"%s\",",(char *)g_sRecord.aLocation);
	pStr+=strlen(pStr);
 
	/*16.测试人姓名*/
	sprintf(pStr, "\"TESTEE\":\"%s\",",(char *)g_sRecord.aTestee);
	pStr+=strlen(pStr);

	/*17.驾驶证号*/
	sprintf(pStr, "\"DRIVERID\":\"%s\",",(char *)g_sRecord.aDriverLicense);
	pStr+=strlen(pStr);

	/*18.车牌号*/
	sprintf(pStr, "\"PLATENUMBER\":\"%s\",",(char *)g_sRecord.aPlateNumber);
	pStr+=strlen(pStr);

	/*19.执勤民警编号*/
	sprintf(pStr, "\"POLICEID\":\"%s\",",(char *)g_sRecord.aPoliceID);
	pStr+=strlen(pStr);

	/*20.执勤民警姓名*/
	sprintf(pStr, "\"POLICENAME\":\"%s\",",(char *)g_sRecord.aPoliceName);
	pStr+=strlen(pStr);

	/*21.执法人员所属部门*/
	sprintf(pStr, "\"DEPARTMENT\":\"%s\"",(char *)g_sRecord.aDepartment);
	pStr+=strlen(pStr);
	
	//结束符
	*pStr++='}';
	
	*pStr++='}';

	return (pStr-pPackageBuffer);
}

static unsigned short Get_Request_DataPacket(char *pPackageBuffer)
{
	char *pStr = pPackageBuffer;
	sRecord *pRecord = &g_sGPRSUser.sRecord;//g_sGPRSUser.sRecord;

        float fTemp;
        char aResult[20];
        float fLongitude=0,fLatitude=0;
	/*起始符*/
	*pStr++ = '{'; //代表传入的是一个数组

	 sprintf(pStr,"\"alcohol\":%c",'{');
	pStr+=strlen(pStr);
	
	/*记录下有效数据包的起始长度*/
	//pStart = pStr;
	 /*1.用户名*/
	sprintf(pStr, "\"USER\":%s,", username);
	pStr+=strlen(pStr);

	/*2.登录密码*/
	sprintf(pStr, "\"KEY\":%s,", KeyPassword);
	pStr+=strlen(pStr);

	/*3.JSON的版本*/
	sprintf (pStr, "\"VERSION\":%s,", VERSION);
	pStr+=strlen(pStr);

	/*4.设备编号字符串*/
	sprintf(pStr, "\"DEVICE\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);


	/*5.记录的编号*/ 
	sprintf (pStr, "\"RECORDID\":%08d,", pRecord->nRecordIndex);
	pStr+=strlen(pStr);

   	
	/*6.日期*/
	sprintf (pStr, "\"DATE\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5]);
	pStr+=strlen(pStr);

     
	 /*7.测试模式*/
	sprintf (pStr, "\"MODE\":\"%d\",",pRecord->nTestMode);
	pStr+=strlen(pStr);

	 /*8.测试温度*/
	sprintf (pStr, "\"TEMP\":%d,", pRecord->nTemperature);
	pStr+=strlen(pStr);

	 /*9.酒精浓度字符串*/
	fTemp=Get_Final_Convert_Display_Result(pRecord->fAlcValue,pRecord->nTestUnit, pRecord->nKBrACBAC);
	Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit,aResult,eFormatToInt);// eFormatToFloat modified on 20181106
	sprintf (pStr, "\"ALC\":%s,", aResult);
	pStr+=strlen(pStr);

	/*10.测试单位*/
	sprintf(pStr, "\"UNIT\":\"%s\",",Get_Unit_String(pRecord->nTestUnit));
	pStr+=strlen(pStr);

	/*11.呼气浓度与血液浓度转换系数*/
	sprintf(pStr, "\"K\":%d,",pRecord->nKBrACBAC);
	pStr+=strlen(pStr);

	/*12.酒精浓度字符串描述*/
	sprintf(pStr, "\"ALCSTRING\":\"%s%s\",",aResult,Get_Unit_String(pRecord->nTestUnit));
	pStr+=strlen(pStr);

	
	if(pRecord->fLongitude != 0 && pRecord->fLatitude != 0)
	{
	/*13.经度数据*/
	fLongitude=pRecord->fLongitude >= 0 ? pRecord->fLongitude : -(pRecord->fLongitude);
	sprintf(pStr, "\"LONGITUDE\":%f,",fLongitude);
	pStr+=strlen(pStr);
	
	/*14.纬度数据*/
	fLatitude=pRecord->fLatitude >= 0 ? pRecord->fLatitude : -(pRecord->fLatitude);
	sprintf(pStr, "\"Latitude\":%f,",fLatitude);
	pStr+=strlen(pStr);
	}
	else
	{
		/*13.经度数据*/
		sprintf(pStr, "\"LONGITUDE\":%s,","null");
		pStr+=strlen(pStr);
	
		/*14.纬度数据*/
		sprintf(pStr, "\"Latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*15.测试地点*/
	sprintf(pStr, "\"LOCATION\":\"%s\",",(char *)pRecord->aLocation);
	pStr+=strlen(pStr);
 
	/*16.测试人姓名*/
	sprintf(pStr, "\"TESTEE\":\"%s\",",(char *)pRecord->aTestee);
	pStr+=strlen(pStr);

	/*17.驾驶证号*/
	sprintf(pStr, "\"DRIVERID\":\"%s\",",(char *)pRecord->aDriverLicense);
	pStr+=strlen(pStr);

	/*18.车牌号*/
	sprintf(pStr, "\"PLATENUMBER\":\"%s\",",(char *)pRecord->aPlateNumber);
	pStr+=strlen(pStr);

	/*19.执勤民警编号*/
	sprintf(pStr, "\"POLICEID\":\"%s\",",(char *)pRecord->aPoliceID);
	pStr+=strlen(pStr);

	/*20.执勤民警姓名*/
	sprintf(pStr, "\"POLICENAME\":\"%s\",",(char *)pRecord->aPoliceName);
	pStr+=strlen(pStr);

	/*21.执法人员所属部门*/
	sprintf(pStr, "\"DEPARTMENT\":\"%s\"",(char *)pRecord->aDepartment);
	pStr+=strlen(pStr);

	/*结束符*/
	*pStr++= '}';
	
	*pStr++='}';
	*pStr= 0x00;//added by lxl 20180813

	
	return (pStr - pPackageBuffer);//返回数据的总长度 

}

char GET_URL[80]="GET /keyun/alcohol/deviceService/test  HTTP/1.1\n";//added by lxl 20180816

unsigned short  Get_GPRS_To_Server_Result_Packet(char *pPackageBuffer)//added by lxl 20180814
{
   	char buffer[BUFFER_SIZE],aTemp[10];
   	char IP_Buffer[100];
   	char *pStart=NULL;
   	short len=0;
   	
   	memset(buffer,0,sizeof(buffer));
    	pStart=pPackageBuffer;
     	len= Result_DataPacket(buffer);//added by lxl 20180814
     	sprintf(aTemp, "%d", len);

	strcat(pStart,GET_URL); //keyun/alcohol/deviceService/test
        //strcat(pStart, "Host: 113.92.92.97:8036\n");
        memset(IP_Buffer,0,sizeof(IP_Buffer));
   	sprintf(IP_Buffer,"Host: %s:%d\n",g_sServerParam.aRemoteIP,g_sServerParam.nRemotePort);
        strcat(pStart, IP_Buffer);
        
        strcat(pStart, "Content-Type: aplication/json \n");
        strcat(pStart, "Content-Length: ");
        strcat(pStart, aTemp);
        strcat(pStart, "\n\n");
        //str2的值为post的数据
        strcat(pStart, buffer);
        strcat(pStart, "\r\n\r\n");
   
   return strlen(pStart);//added by lxl 
}

static unsigned short  Get_GPRS_To_Server_Record_Packet(char *pPackageBuffer)
{
   	char buffer[BUFFER_SIZE];
   	char IP_Buffer[100];
   	char *pStart=NULL,aTemp[10];
   	short len=0;
    	pStart=pPackageBuffer;
    	
	memset(buffer,0,sizeof(buffer));
  	
        len=Get_Request_DataPacket(buffer);
    	sprintf(aTemp, "%d", len);
   	strcat(pStart,GET_URL);// Home/Openapi/get_alcohol
   	
   	memset(IP_Buffer,0,sizeof(IP_Buffer));
   	sprintf(IP_Buffer,"Host: %s:%d\n",g_sServerParam.aRemoteIP,g_sServerParam.nRemotePort);
        //strcat(pStart, "Host: 113.92.92.97:8036\n");
        strcat(pStart, IP_Buffer);
   
        strcat(pStart, "Content-Type: aplication/json \n");
        strcat(pStart, "Content-Length: ");
        strcat(pStart, aTemp);
        strcat(pStart, "\n\n");
        //str2的值为post的数据
        strcat(pStart, buffer);
        strcat(pStart, "\r\n\r\n");

   return strlen(pStart);//(pStart-pStart_TEMP);//added by lxl 
}
static short Get_HeartRate_Data(char *pPackageBuffer)
{
	char *pStr = pPackageBuffer;
	float fLongitude=0,fLatitude=0;
        float fCurrentLongitude,fCurrentLatitude;
        char s_aAdmin[] = {0xE7,0xAE,0xA1,0xE7,0x90,0x86,0xE5,0x91,0x98,0x00};   /*管理员*/
        char s_department[5]={0x00};
	 
	fCurrentLongitude=g_bGPS_Location_Valid == 1  ? g_fLongitude : 0;
	fCurrentLatitude=g_bGPS_Location_Valid == 1  ? g_fLatitude: 0;
	
	*pStr++='{';
	sprintf(pStr,"\"heartrate\":%c",'{');
	pStr+=strlen(pStr);

	/*记录下有效数据包的起始长度*/
	//pStart = pStr;
	 /*1.用户名*/
	sprintf(pStr, "\"user\":%s,", username);
	pStr+=strlen(pStr);

	/*2.登录密码*/
	sprintf(pStr, "\"key\":%s,", KeyPassword);
	pStr+=strlen(pStr);

       	/*3.上线时间*/ 
	sprintf (pStr, "\"onlinedate\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 g_sDateTime.wYear, g_sDateTime.wMonth,
			 g_sDateTime.wDay, g_sDateTime.wHour,
			 g_sDateTime.wMinute, g_sDateTime.wSecond);
	pStr+=strlen(pStr);

	
	if(fCurrentLongitude!= 0 && fCurrentLatitude != 0)
	{
	/*4.经度数据*/
	fLongitude=fCurrentLongitude>= 0 ? fCurrentLongitude : -(fCurrentLongitude);
	sprintf(pStr, "\"longitude\":%f,",fLongitude);
	pStr+=strlen(pStr);
	
	/*5.纬度数据*/
	fLatitude=fCurrentLatitude >= 0 ? fCurrentLatitude : -(fCurrentLatitude);
	sprintf(pStr, "\"latitude\":%f,",fLatitude);
	pStr+=strlen(pStr);
	}
	else
	{
		/*4.经度数据*/
		sprintf(pStr, "\"longitude\":%s,","null");
		pStr+=strlen(pStr);
	
		/*5.纬度数据*/
		sprintf(pStr, "\"latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*6.执勤民警姓名*/
	if(pApp->bLoginAsAdministrator)
		sprintf(pStr, "\"policename\":\"%s\",",(char *)s_aAdmin); //管理员登录
	else
		sprintf(pStr, "\"policename\":\"%s\",",(char *)(g_sCurrPoliceUserInfor.aPoliceName));
	pStr+=strlen(pStr);
	

	/*7.执法人员所属部门*/
	if(pApp->bLoginAsAdministrator)
		sprintf(pStr, "\"department\":\"%s\",",s_department); //管理员登录
	else
		sprintf(pStr, "\"department\":\"%s\",",(char *)(g_sCurrPoliceUserInfor.aDepartment)); 
	pStr+=strlen(pStr);


	/*8.在线状态*/
	if(g_sXingJiKongGU.bDeviceState)
		sprintf(pStr, "\"state\":%d,",1); //管理员登录
	else
		sprintf(pStr, "\"state\":%d,",0); 
	pStr+=strlen(pStr);
	
	/*9.设备编号字符串*/
	sprintf(pStr, "\"device\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);

	/*10.心跳时间(S)*/
	sprintf(pStr, "\"heartbeat\":%d,", g_sHeartRateParam.byHeartRate);
	pStr+=strlen(pStr);
	
	//结束符
	*pStr++='}';
	
	*pStr++='}';

	return (pStr-pPackageBuffer);
}
static short Get_HeartRate_First_Data(char *pPackageBuffer)
{
	char *pStr = pPackageBuffer;
	float fLongitude=0,fLatitude=0;
        float fCurrentLongitude,fCurrentLatitude;
	 
	fCurrentLongitude=g_bGPS_Location_Valid == 1  ? g_fLongitude : 0;
	fCurrentLatitude=g_bGPS_Location_Valid == 1  ? g_fLatitude: 0;
	
	*pStr++='{';
	sprintf(pStr,"\"heartrate\":%c",'{');
	pStr+=strlen(pStr);

	/*记录下有效数据包的起始长度*/
	//pStart = pStr;
	 /*1.用户名*/
	sprintf(pStr, "\"user\":%s,", username);
	pStr+=strlen(pStr);

	/*2.登录密码*/
	sprintf(pStr, "\"key\":%s,", KeyPassword);
	pStr+=strlen(pStr);

       	/*3.上线时间*/ 
	sprintf (pStr, "\"onlinedate\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 g_sDateTime.wYear, g_sDateTime.wMonth,
			 g_sDateTime.wDay, g_sDateTime.wHour,
			 g_sDateTime.wMinute, g_sDateTime.wSecond);
	pStr+=strlen(pStr);

	
	if(fCurrentLongitude!= 0 && fCurrentLatitude != 0)
	{
		/*4.经度数据*/
		fLongitude=fCurrentLongitude>= 0 ? fCurrentLongitude : -(fCurrentLongitude);
		sprintf(pStr, "\"longitude\":%f,",fLongitude);
		pStr+=strlen(pStr);
	
		/*5.纬度数据*/
		fLatitude=fCurrentLatitude >= 0 ? fCurrentLatitude : -(fCurrentLatitude);
		sprintf(pStr, "\"latitude\":%f,",fLatitude);
		pStr+=strlen(pStr);
	}
	else
	{
		/*4.经度数据*/
		sprintf(pStr, "\"longitude\":%s,","null");
		pStr+=strlen(pStr);
	
		/*5.纬度数据*/
		sprintf(pStr, "\"latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*6.在线状态*/
	if(g_sXingJiKongGU.bDeviceState)
		sprintf(pStr, "\"state\":%d,",1); //管理员登录
	else
		sprintf(pStr, "\"state\":%d,",0); 
	pStr+=strlen(pStr);
	
	/*6.设备编号字符串*/
	sprintf(pStr, "\"device\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);

	/*8.心跳时间(S)*/
	sprintf(pStr, "\"heartbeat\":%d,", g_sHeartRateParam.byHeartRate);
	pStr+=strlen(pStr);
	
	//结束符
	*pStr++='}';
	
	*pStr++='}';

	return (pStr-pPackageBuffer);
}


char GET_HeartURL[80]="GET /keyun/alcohol/deviceService/state  HTTP/1.1\n";//added by lxl 20180816
static unsigned short GPRS_HeartRate_Packet(char *pPackageBuffer)
{
   	char buffer[BUFFER_SIZE];
   	char IP_Buffer[100];
   	char *pStart=NULL,aTemp[10];
   	short len=0;
    	pStart=pPackageBuffer;
    	
	memset(buffer,0,sizeof(buffer));

  	if(g_sXingJiKongGU.bDeviceStart)
  	{
  		g_sXingJiKongGU.bDeviceStart=false;
        	len=Get_HeartRate_First_Data(buffer);
        }
        else
        {
        	len=Get_HeartRate_Data(buffer);
        }
    	sprintf(aTemp, "%d", len);
   	strcat(pStart,GET_HeartURL);// Home/Openapi/get_alcohol
   	
   	memset(IP_Buffer,0,sizeof(IP_Buffer));
   	sprintf(IP_Buffer,"Host: %s:%d\n",g_sServerParam.aRemoteIP,g_sServerParam.nRemotePort);
        //strcat(pStart, "Host: 113.92.92.97:8036\n");
        strcat(pStart, IP_Buffer);
   
        strcat(pStart, "Content-Type: aplication/json \n");
        strcat(pStart, "Content-Length: ");
        strcat(pStart, aTemp);
        strcat(pStart, "\n\n");
        //str2的值为post的数据
        strcat(pStart, buffer);
        strcat(pStart, "\r\n\r\n");

   return strlen(pStart);//(pStart-pStart_TEMP);//added by lxl 
}
/*设备离线*/
void Device_Close_Packet_Upload(void)
{
	if(g_sXingJiKongGU.bSingal_Strength)
     	{ 	
		memset(GPRS_Tx_Buffer,0,sizeof(GPRS_Tx_Buffer));
		g_nGprs_Tx_Cnt=GPRS_HeartRate_Packet(GPRS_Tx_Buffer);
		g_sXingJiKongGU.bHeartRate=true;

		TRACE_PRINTF("heartrate:%s\r\n",GPRS_Tx_Buffer);
    		TRACE_PRINTF("length:%d\r\n",g_nGprs_Tx_Cnt);
    }
}
void GPRS_HeartRate_Packet_Upload(void)
{    
	static unsigned long ul_Current_Time=0;

     	if((g_sXingJiKongGU.bResult_Packet_Flag==false)&&(g_sXingJiKongGU.bReveiceAckFromServer==false)&&g_sXingJiKongGU.bSingal_Strength) //
     	{ 	
     		if((Get_System_Time()>g_sHeartRateParam.byHeartRate*1000+ul_Current_Time)&&(g_sXingJiKongGU.bDeviceStart==false))
     		{
     			ul_Current_Time=Get_System_Time();
			memset(GPRS_Tx_Buffer,0,sizeof(GPRS_Tx_Buffer));
			g_nGprs_Tx_Cnt=GPRS_HeartRate_Packet(GPRS_Tx_Buffer);
			g_sXingJiKongGU.bHeartRate=true;

			TRACE_PRINTF("heartrate:%s\r\n",GPRS_Tx_Buffer);
    			TRACE_PRINTF("length:%d\r\n",g_nGprs_Tx_Cnt);
    		}
    		
    		if(g_sXingJiKongGU.bDeviceStart==true)
    		{
    			ul_Current_Time=Get_System_Time();
    			
    			memset(GPRS_Tx_Buffer,0,sizeof(GPRS_Tx_Buffer));
			g_nGprs_Tx_Cnt=GPRS_HeartRate_Packet(GPRS_Tx_Buffer);
			g_sXingJiKongGU.bHeartRate=true;

			TRACE_PRINTF("heartrate:%s\r\n",GPRS_Tx_Buffer);
    			TRACE_PRINTF("length:%d\r\n",g_nGprs_Tx_Cnt);
    		}
    	}
}


void GprsUser_Switch_State(eGprsUserState eUserState)
{
	g_sGPRSUser.nRetryCnt = 3;
	g_sGPRSUser.eUserState = eUserState;
	g_sGPRSUser.bSwitchNewState = true;
	g_sGPRSUser.nStartWaitTime = Get_System_Time();
	g_sGPRSUser.bIsReveiceAckFromServer = false;
}

void GprsUser_SendData_Timeout_Handle(const char *FileName, int nLineNumber)
{
	if(g_sGPRSUser.nRetryCnt)
	{	
		g_sGPRSUser.nRetryCnt--;
		g_sGPRSUser.bSwitchNewState = true;
	}
	else
	{
		_Reset_Upload_State_And_Retry(FileName, nLineNumber);
	}
}

/*主循环中调用*/
void Exec_GPRS_User_Loop(void)
{
	GPRS_Exce();//真正的登录和打包数据的函数 20180809

	switch(g_sGPRSUser.eUserState)
	{
		case eGprsUserGetIMEI:
			if(g_sGPRSUser.bGetValidIMEI)//没有数据也要继续
			{
				if(0) //Is_ValidData_Need_Upload() == false &&_Is_GPRS_System_PoweOn()
				{
					GPRS_Power_Control(eGPRSPowerDownNormal, __FILE__, __LINE__);
				}
				
				GprsUser_Switch_State(eGprsUserGetRecordData);
			}
			else
			{
				if(_Is_GPRS_System_PoweOn() == false)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
				}
			}
			break;

		case eGprsUserGetRecordData:
			//检查是否有数据需要上传,且没有结果和心跳包上传数据包上传 modified by lxl 20180930
			if(Is_ValidData_Need_Upload()&&(g_sXingJiKongGU.bResult_Packet_Flag==false))//
			{
				if(_Is_GPRS_System_PoweOn() == false)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
				}
				if(Get_New_Upload_Record_Data()) //从此函数中获取数据记录
				{
					GprsUser_Switch_State(eGprsUserUploadRecordData);// eGprsUserLoginServer Modified by lxl 20180809
					g_sXingJiKongGU.bReveiceAckFromServer=true;//g_ReveiceAckFromServer=true;//只要一读到数据就置1, 直到该记录包上传完毕Modified by lxl 20180817
				}
			}

			break;

		case eGprsUserUploadRecordData: //上传数据 现在修改为http的协议格式进行上传数据，只需要在这里修改 20180808
			if(g_sGPRSUser.bSwitchNewState)
			{
				g_sGPRSUser.bSwitchNewState = false;
				g_sGPRSUser.nStartWaitTime = Get_System_Time();
				memset(GPRS_Tx_Buffer,0,sizeof(GPRS_Tx_Buffer));//added by lxl 20180816
				g_nGprs_Tx_Cnt = Get_GPRS_To_Server_Record_Packet(GPRS_Tx_Buffer);//获取数据记录 20180704 lxl
				g_sGPRSUser.nCurrentSendPacketSize = g_nGprs_Tx_Cnt;
				//g_ReveiceAckFromServer=true;//发送数据记录包标志位 added by lxl 20180816
				
				TRACE_PRINTF("GPRS_Tx_Buffer:%s\r\n",GPRS_Tx_Buffer);
				TRACE_PRINTF("GPRS-->Get_GPRS_To_Server_Record_Packet! g_nGprs_Tx_Cnt = %d\r\n",g_nGprs_Tx_Cnt);
			}
                      
			if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + 30000)
			{
				GprsUser_SendData_Timeout_Handle(__FILE__, __LINE__);
			}
			else if(g_nGprs_Tx_Cnt == 0)//每上传一次数据累计一次上传记录次数
			{
				if(g_sGPRSUser.bIsReveiceAckFromServer)
				{
					Update_Already_Upload_Record_Counter();//上传记录完成，开始记录已上传记录数
					GprsUser_Switch_State(eGprsUserGetRecordData);
				}
			}

			break;
	}
}

#endif

#else

void Upload_Record_Var_Init(void) {}
void Exec_GPRS_User_Loop(void) {}
signed char Get_GPRS_SingalQuality(void) { return -1;}
void Update_Already_Upload_Record_Counter(void){}


#endif
