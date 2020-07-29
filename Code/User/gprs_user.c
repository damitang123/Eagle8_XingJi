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

	/*�������ͨ����Ȼ���к�*/
	unsigned long nPacketIndex;

	bool bSwitchNewState;
	unsigned char nRetryCnt;
	eGprsUserState eUserState;
	unsigned long nStartWaitTime;

	/*��������β,CMD,DATA LENGTH�����ֽ�.*/
	int nCurrentSendPacketSize;	
	bool bIsReveiceAckFromServer;

	/*��¼���*/
	sRecord sRecord;
	unsigned short nRecordDataLength;
	/*��Ƭ���*/
	unsigned char nImagePacketIndex;
	unsigned char nTotalImagePacketNumber;
	unsigned long nJpegImageSize;
	/*ָ�����*/
	unsigned char nFingerprintPacketIndex;
	unsigned short nFingerprintTemplateSize;
}__attribute__ ((packed))sGPRSUser;

static sGPRSUser g_sGPRSUser;

/*������ѭ��ǰ����,��������һ�μ���.*/
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
		/*��ǰ��Ŀ����ȡ����*/
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

/*֪ͨ�����¼���ϴ�*/
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
			/*��½Ӧ���*/
			case 0x22:
				pProcPtr = pBuffer + 2;
				memcpy(&nDataLength, pProcPtr, 2);
				pProcPtr += 2;
				
				if(nDataLength < nLen)
				{
					/*��һ����IMEI,ȡ���ж�*/
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

					/*�ڶ����Ƿ�����ʱ��*/
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

			/*�����ϴ����ȷ�ϰ�.*/
			case 0x1F:
				pProcPtr = pBuffer + 2;
				memcpy(&nDataLength, pProcPtr, 2);
				pProcPtr += 2;
				
				if(nDataLength < nLen)
				{
					/*��һ����IMEI,ȡ���ж�*/
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

					/*�ڶ�����ʱ��*/
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

					/*�������Ǿƾ����Ա����С*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(g_sGPRSUser.nRecordDataLength != atoi(pProcPtr))
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) Record basic size doesn't match(Expect size = %d , real size = %d)!\r\n", g_sGPRSUser.nRecordDataLength, atoi(pProcPtr));
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*���Ķ���ͼƬ��С*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(g_sGPRSUser.nJpegImageSize != atoi(pProcPtr))
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) Record nJpegImageSize doesn't match(Expect size = %d , real size = %d)!\r\n", g_sGPRSUser.nJpegImageSize, atoi(pProcPtr));
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					/*�������ָ������ֵ��С*/
					memcpy(&nDataLength, pProcPtr, 2);
					pProcPtr += 2;
					if(g_sGPRSUser.nFingerprintTemplateSize != atoi(pProcPtr))
					{
						TRACE_PRINTF("GPRS-->Cmd(0x1F) Record nFingerprintTemplateSize doesn't match(Expect size = %d , real size = %d)!\r\n", g_sGPRSUser.nFingerprintTemplateSize, atoi(pProcPtr));
						goto _Error_Exit;
					}
					pProcPtr += nDataLength;

					
					/*�������Ǹ���Ա�����ݿ��г��ֵĴ���*/
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

			/*���ݽ���Ӧ���*/
			case 0x10:
				pProcPtr = pBuffer + 2;
				memcpy(&nDataLength, pProcPtr, 2);
				pProcPtr += 2;
				
				if(nDataLength < nLen)
				{
					/*��һ����IMEI,ȡ���ж�*/
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

					/*�ڶ������յ����ֽڳ���*/
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
		/*��ӡ�յ�������*/
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

	/*��ʼ��*/
	*pStr++ = '[';

	/*��½������*/
	*pStr++ = 0x21;

	/*Data length ռλ*/
	pStr += 2;

	/*��¼����Ч���ݰ�����ʼ����*/
	pStart = pStr;

	/*1.IMEI���ݶ�*/
	_Add_Item_Data(&pStr, Get_Hardware_IMEI());

	/*2.��½���ݰ��汾.*/
	_Add_Item_Data(&pStr, GPRS_DATA_PACKAGE_VERSION);

	/*3.����汾��*/
	_Add_Item_Data(&pStr, FW_VERSION);

	/*������*/
	*pStr++ = ']';

	/*�������ݳ���,��д��.*/
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

	/*��ʼ��*/
	*pStr++ = '[';

	/*�ϴ���¼������*/
	*pStr++ = 0x1A;

	/*Data length ռλ*/
	pStr += 2;

	/*��¼����Ч���ݰ�����ʼ����*/
	pStart = pStr;

	/*1.IMEI���ݶ�*/
	_Add_Item_Data(&pStr, Get_Hardware_IMEI());

	/*2.��¼ʱ������*/
	sprintf (aTemp, "%02d%02d%02d%02d%02d%02d",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5] );
	_Add_Item_Data(&pStr, aTemp);

	/*3.��½���ݰ��汾.*/
	_Add_Item_Data(&pStr, GPRS_DATA_PACKAGE_VERSION);

	/*4.�������*/
	sprintf(aTemp, "%s%06ld", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	_Add_Item_Data(&pStr, aTemp);

	/*5.�����ϴ��ӿ�.3Gģ��*/
	_Add_Item_Data(&pStr, "3");

	/*6.�������*/
	sprintf (aTemp, "%08d", pRecord->nRecordIndex);
	_Add_Item_Data(&pStr, aTemp);

	/*7.����ģʽ*/
	sprintf (aTemp, "%d", pRecord->nTestMode);
	_Add_Item_Data(&pStr, aTemp);

	/*GPRS��λ��Ϣ��*/
	if(pRecord->fLongitude != 0 && pRecord->fLatitude != 0)
	{
		/*8.GPS�Ƿ���Ч*/
		_Add_Item_Data(&pStr, "A");

		/*9.ά��ֵ*/
		sprintf(aTemp, "%f", pRecord->fLatitude >= 0 ? pRecord->fLatitude : -(pRecord->fLatitude));
		_Add_Item_Data(&pStr, aTemp);

		/*10.ά�ȱ�ʶ,��N,��S*/
		sprintf(aTemp, "%s", pRecord->fLatitude > 0 ? "N" : "S");
		_Add_Item_Data(&pStr, aTemp);

		/*11.����ֵ*/
		sprintf(aTemp, "%f", pRecord->fLongitude >= 0 ? pRecord->fLongitude : -(pRecord->fLongitude));
		_Add_Item_Data(&pStr, aTemp);

		/*12.���ȱ�ʶ,��E,��W*/
		sprintf(aTemp, "%s", pRecord->fLongitude > 0 ? "E" : "W");
		_Add_Item_Data(&pStr, aTemp);
	}
	else
	{
		/*8.GPS�Ƿ���Ч*/
		_Add_Item_Data(&pStr, "V");

		/*9.ά��ֵ*/
		_Add_Item_Data(&pStr, "");

		/*10.ά�ȱ�ʶ,��N,��S*/
		_Add_Item_Data(&pStr, "");

		/*11.����ֵ*/
		_Add_Item_Data(&pStr, "");

		/*12.���ȱ�ʶ,��E,��W*/
		_Add_Item_Data(&pStr, "");
	}

	/*13.�����¶�*/
	sprintf (aTemp, "%d", pRecord->nTemperature);
	_Add_Item_Data(&pStr, aTemp);

	/*14.ת��ϵ��*/
	sprintf (aTemp, "%d", pRecord->nKBrACBAC);
	_Add_Item_Data(&pStr, aTemp);

	/*15.�ƾ����Խ��,����λ������ַ���.*/
	fAlcUnitConvertValue = Get_Final_Convert_Display_Result(pRecord->fAlcValue, pRecord->nTestUnit, pRecord->nKBrACBAC);
#if USE_GB_T21254_2017
	Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, aTemp,eFormatToInt);
#else
	Get_Format_Alc_Convert_String(fAlcUnitConvertValue, pRecord->nTestUnit, aTemp,eFormatToFloat);
#endif
	strcat(aTemp, Get_Unit_String(pRecord->nTestUnit));
	_Add_Item_Data(&pStr, aTemp);

	/*16.�ƾ�����ֵ,mg/100mlΪ��λ����ֵ�ַ���*/
	sprintf (aTemp, "%f", pRecord->fAlcValue);
	_Add_Item_Data(&pStr, aTemp);

	/*17.��ʻ֤��*/
	_Add_Item_Data(&pStr, (char *)pRecord->aDriverLicense);

	/*18.����������*/
	_Add_Item_Data(&pStr, (char *)pRecord->aTestee);

	/*19.���Եص�*/
	_Add_Item_Data(&pStr, (char *)pRecord->aLocation);

	/*20.���ƺ�*/
	_Add_Item_Data(&pStr, (char *)pRecord->aPlateNumber);

	/*21.ִ��������*/
	_Add_Item_Data(&pStr, (char *)pRecord->aPoliceName);

	/*22.ִ���񾯾���*/
	_Add_Item_Data(&pStr, (char *)pRecord->aPoliceID);

	/*23.������λ*/
	_Add_Item_Data(&pStr, (char *)pRecord->aDepartment);

	/*������*/
	*pStr++ = ']';

	/*�������ݳ���,��д��.*/
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
	
	/*��ʼ��*/
	*pStr++ = '[';

	/*�ϴ�ָ����Ϣ������*/
	*pStr++ = 0x1E;

	/*Data length ռλ*/
	pStr += 2;

	/*��¼����Ч���ݰ�����ʼ����*/
	pStart = pStr;

	/*1.IMEI���ݶ�*/
	_Add_Item_Data(&pStr, Get_Hardware_IMEI());

	/*2.��¼ʱ������*/
	sprintf (aTemp, "%02d%02d%02d%02d%02d%02d",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5] );
	_Add_Item_Data(&pStr, aTemp);

	/*3.�ƾ��������ݰ���С.*/
	sprintf(aTemp, "%d",g_sGPRSUser.nRecordDataLength);
	_Add_Item_Data(&pStr, aTemp);

	/*4.��Ƭ�ļ���С.*/
	sprintf(aTemp, "%d",g_sGPRSUser.nJpegImageSize);
	_Add_Item_Data(&pStr, aTemp);

	/*5.ָ�������ļ���С.*/
	sprintf(aTemp, "%d",g_sGPRSUser.nFingerprintTemplateSize);
	_Add_Item_Data(&pStr, aTemp);
	
	/*������*/
	*pStr++ = ']';

	/*�������ݳ���,��д��.*/
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

/*��ѭ���е���*/
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
					/*ȡ��һ������.*/
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
			/*��½Ӧ���*/
			case 0x22:
				Gprs_Set_UserLogin_State(true);
				TRACE_PRINTF("GPRS-->Login server successful!\r\n");
				break;

			/*���������ϴ����ȷ�ϰ�.*/
			case 0x1B:
				g_sGPRSUser.bIsReveiceAckFromServer = true;
				break;

			/*��Ƭ�ϴ���Ӧ��.��3��������*/
			case 0x4C:
				break;
			
			/*ָ�������ϴ����ȷ�ϰ�.*/
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
		/*��ӡ�յ�������*/
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

	/*��ʼ��*/
	*pStr++ = '[';

	/*��½������*/
	*pStr++ = 0x21;

	/*Packet�����к�*/
	sprintf(pStr, "%05d", g_sGPRSUser.nPacketIndex++);
	pStr += 5;

	/*Data length ռλ*/
	pStr += 2;

	/*��¼����Ч���ݰ�����ʼ����*/
	pStart = pStr;

	/*���ݶ�����*/
	*pStr++ = '(';

	/*1.IMEI���ݶ�*/
	memcpy(pStr, Get_Hardware_IMEI(), 15);
	pStr += 15;
	*pStr++ = ',';

	/*2.�汾��*/
	sprintf ( pStr, "V1.0" );
	pStr += 4;
	*pStr++ = ',';

	/*3.MCC,Ŀǰ�̶�Ϊ�й��ƶ�MCC��*/
	sprintf(pStr,"%03d",86);
	pStr += 3;

	/*���ݶ�����*/
	*pStr++ = ')';
	
	/*������*/
	*pStr++ = ']';

	/*�������ݳ���,��д��.*/
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

	/*��ʼ��*/
	*pStr++ = '[';

	/*�ϴ���¼������*/
	*pStr++ = 0x1A;

	/*Packet�����к�*/
	sprintf(pStr, "%05d", g_sGPRSUser.nPacketIndex++);
	pStr += 5;

	/*Data length ռλ*/
	pStr += 2;

	/*��¼����Ч���ݰ�����ʼ����*/
	pStart = pStr;

	/*���ݶ�����*/
	*pStr++ = '(';

	/*1.�������*/
	sprintf(pStr, "%s%06d", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr += 8;
	*pStr++ = 0x00;

	/*2.��¼ʱ������*/
	sprintf (pStr, "%02d%02d%02d%02d%02d%02d",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5]);
	pStr += 12;
	*pStr++ = 0x00;

	/*3.�������*/
	sprintf (pStr, "%08d", pRecord->nRecordIndex);
	pStr += 8;
	*pStr++ = 0x00;

	/*4.����ģʽ*/
	sprintf (pStr, "%d", pRecord->nTestMode);
	pStr += 1;
	*pStr++ = 0x00;

	/*GPRS��λ��Ϣ��*/
	if(pRecord->fLongitude != 0 && pRecord->fLatitude != 0)
	{
		/*5.GPS�Ƿ���Ч*/
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
		/*5.GPS�Ƿ���Ч*/
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

	//�ֶ�10: �����¶� ----- ����������N_STRING��,��27.5 ��ʾ27.5���϶�
	sprintf ( pStr, "%d", pRecord->nTemperature );
	pStr += strlen ( pStr );
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�11���ƾ����Խ�� ----- ����������N_STRING����g.g��ʽ��λmg/100ml���磺34.3��ʾ34.3mg/100ml
#if USE_GB_T21254_2017
	Get_Format_Alc_Convert_String(pRecord->fAlcValue, 0, pStr,eFormatToInt);
#else
	Get_Format_Alc_Convert_String(pRecord->fAlcValue, 0, pStr,eFormatToFloat);
#endif
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�12����ʻ֤��
	strncpy(pStr, (char *)pRecord->aDriverLicense, sizeof(pRecord->aDriverLicense));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�13: ����������
	strncpy(pStr, (char *)pRecord->aTestee, sizeof(pRecord->aTestee));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�14�����Եص�
	strncpy(pStr, (char *)pRecord->aLocation, sizeof(pRecord->aLocation));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�15�����ƺ�
	strncpy(pStr, (char *)pRecord->aPlateNumber, sizeof(pRecord->aPlateNumber));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�16��ִ��������
	strncpy(pStr, (char *)pRecord->aPoliceName, sizeof(pRecord->aPoliceName));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�17��ִ���񾯾���
	strncpy(pStr, (char *)pRecord->aPoliceID, sizeof(pRecord->aPoliceID));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	//�ֶ�18��������λ
	strncpy(pStr, (char *)pRecord->aDepartment, sizeof(pRecord->aDepartment));
	pStr += strlen (pStr);
	*pStr++ = 0x00; //�ֶηָ���

	/*���ݶ�����*/
	*pStr++ = ')';

	/*������*/
	*pStr++ = ']';

	/*�������ݳ���,��д��.*/
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

/*��ѭ���е���*/
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
	SearchWord=strstr(SearchWord,"\r\n");//����
	SearchWord+=2;//��6�� ���ݰ�����
	SearchWord=strstr(SearchWord,"\r\n");//Ӧ������ݳ���
	SearchWord+=2;//Ӧ���������ݰ�

	Response_Position=SearchWord;

	return Response_Position;//���ؽ�����ݰ�λ�� 20180816

}
void Gprs_Analyse_Server_Package_Data_Sub(char *pBuffer ,unsigned short nLen)
{
	char *Data_Position=NULL;
	char *REMARK=NULL;
	char *TYPE=NULL;
	eGprsSendState Sendstate;
	
	if(!strstr(pBuffer,"HTTP"))//������ݰ��в�����HTTP����λ��Ч���ݰ�,��ִ������Ĳ��� 
	{
		TRACE_PRINTF("Empty Packet!\r\n");
		return;
	}
	
	Data_Position=Analyse_Server_Package_GrandSub(pBuffer);
	REMARK=strstr(Data_Position,"REMARK");//�ҵ�Ӧ�����ݰ�
	if(strstr(Data_Position,"OK"))
	{
		Sendstate=OK;
		TYPE=strstr(Data_Position,"TYPE");
	}
	else
	{
		REMARK=strstr(Data_Position,"REMARK");//�ҵ���������λ��
		Sendstate=FAIL;
	}
	
	switch(Sendstate)
	{
		case OK:		
		        TRACE_PRINTF("TYPE:%d\r\n",*(TYPE+6)-'0');

			if((*(TYPE+6)-'0')==2) //�ƾ�ֵ��
				g_sXingJiKongGU.bResult_Packet_Flag=false;//g_Result_Packet_Flag=false;//������ݰ��ϴ����

				
			if((*(TYPE+6)-'0')==1)
			{
				g_sGPRSUser.bIsReveiceAckFromServer=true;//���Լ�¼�������ϴ����,������¼���ϴ��ļ�¼�� added by lxl 20180815
				g_sXingJiKongGU.bReveiceAckFromServer=false;//g_ReveiceAckFromServer=false; //���Լ�¼�������ϴ����
			}

			if((*(TYPE+6)-'0')==3) //����Ӧ���
				g_sXingJiKongGU.bHeartRate=false;
			
		break;

		case FAIL:
			TRACE_PRINTF("REMARK:%s\r\n",REMARK);
		break;
	}
	
}
//ֻ���Ͳ��Խ�� 20180814
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

	/*��¼����Ч���ݰ�����ʼ����*/
	//pStart = pStr;
	 /*1.�û���*/
	sprintf(pStr, "\"USER\":%s,", username);
	pStr+=strlen(pStr);

	/*2.��¼����*/
	sprintf(pStr, "\"KEY\":%s,", KeyPassword);
	pStr+=strlen(pStr);

	/*3.JSON�İ汾*/
	sprintf (pStr, "\"VERSION\":%s,", VERSION);
	pStr+=strlen(pStr);

	/*4.�豸����ַ���*/
	sprintf(pStr, "\"DEVICE\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);


	/*5.��¼�ı��*/ 
	sprintf (pStr, "\"RECORDID\":%08d,", g_sRecord.nRecordIndex);
	pStr+=strlen(pStr);

   	
	/*6.����*/
	sprintf (pStr, "\"DATE\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 g_sRecord.aDateTime[0], g_sRecord.aDateTime[1],
			 g_sRecord.aDateTime[2], g_sRecord.aDateTime[3],
			 g_sRecord.aDateTime[4], g_sRecord.aDateTime[5]);
	pStr+=strlen(pStr);

     
	 /*7.����ģʽ*/
	sprintf (pStr, "\"MODE\":\"%d\",",g_sRecord.nTestMode);
	pStr+=strlen(pStr);

	 /*8.�����¶�*/
	sprintf (pStr, "\"TEMP\":%d,", g_sRecord.nTemperature);
	pStr+=strlen(pStr);

	 /*9.�ƾ�Ũ���ַ���*/
	fTemp=Get_Final_Convert_Display_Result(g_sRecord.fAlcValue,g_sRecord.nTestUnit, g_sRecord.nKBrACBAC);
	Get_Format_Alc_Convert_String(fTemp, g_sRecord.nTestUnit,aResult,eFormatToInt); //eFormatToFloat modified on 20181106
	sprintf (pStr, "\"ALC\":%s,", aResult);
	pStr+=strlen(pStr);

	/*10.���Ե�λ*/
	sprintf(pStr, "\"UNIT\":\"%s\",",Get_Unit_String(g_sRecord.nTestUnit));
	pStr+=strlen(pStr);

	/*11.����Ũ����ѪҺŨ��ת��ϵ��*/
	sprintf(pStr, "\"K\":%d,",g_sRecord.nKBrACBAC);
	pStr+=strlen(pStr);

	/*12.�ƾ�Ũ���ַ�������*/
	sprintf(pStr, "\"ALCSTRING\":\"%s%s\",",aResult,Get_Unit_String(g_sRecord.nTestUnit));
	pStr+=strlen(pStr);

	
	if(g_sRecord.fLongitude != 0 && g_sRecord.fLatitude != 0)
	{
	/*13.��������*/
	fLongitude=g_sRecord.fLongitude >= 0 ? g_sRecord.fLongitude : -(g_sRecord.fLongitude);
	sprintf(pStr, "\"LONGITUDE\":%f,",fLongitude);
	pStr+=strlen(pStr);
	
	/*14.γ������*/
	fLatitude=g_sRecord.fLatitude >= 0 ? g_sRecord.fLatitude : -(g_sRecord.fLatitude);
	sprintf(pStr, "\"Latitude\":%f,",fLatitude);
	pStr+=strlen(pStr);
	}
	else
	{
		/*13.��������*/
		sprintf(pStr, "\"LONGITUDE\":%s,","null");
		pStr+=strlen(pStr);
	
		/*14.γ������*/
		sprintf(pStr, "\"Latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*15.���Եص�*/
	sprintf(pStr, "\"LOCATION\":\"%s\",",(char *)g_sRecord.aLocation);
	pStr+=strlen(pStr);
 
	/*16.����������*/
	sprintf(pStr, "\"TESTEE\":\"%s\",",(char *)g_sRecord.aTestee);
	pStr+=strlen(pStr);

	/*17.��ʻ֤��*/
	sprintf(pStr, "\"DRIVERID\":\"%s\",",(char *)g_sRecord.aDriverLicense);
	pStr+=strlen(pStr);

	/*18.���ƺ�*/
	sprintf(pStr, "\"PLATENUMBER\":\"%s\",",(char *)g_sRecord.aPlateNumber);
	pStr+=strlen(pStr);

	/*19.ִ���񾯱��*/
	sprintf(pStr, "\"POLICEID\":\"%s\",",(char *)g_sRecord.aPoliceID);
	pStr+=strlen(pStr);

	/*20.ִ��������*/
	sprintf(pStr, "\"POLICENAME\":\"%s\",",(char *)g_sRecord.aPoliceName);
	pStr+=strlen(pStr);

	/*21.ִ����Ա��������*/
	sprintf(pStr, "\"DEPARTMENT\":\"%s\"",(char *)g_sRecord.aDepartment);
	pStr+=strlen(pStr);
	
	//������
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
	/*��ʼ��*/
	*pStr++ = '{'; //���������һ������

	 sprintf(pStr,"\"alcohol\":%c",'{');
	pStr+=strlen(pStr);
	
	/*��¼����Ч���ݰ�����ʼ����*/
	//pStart = pStr;
	 /*1.�û���*/
	sprintf(pStr, "\"USER\":%s,", username);
	pStr+=strlen(pStr);

	/*2.��¼����*/
	sprintf(pStr, "\"KEY\":%s,", KeyPassword);
	pStr+=strlen(pStr);

	/*3.JSON�İ汾*/
	sprintf (pStr, "\"VERSION\":%s,", VERSION);
	pStr+=strlen(pStr);

	/*4.�豸����ַ���*/
	sprintf(pStr, "\"DEVICE\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);


	/*5.��¼�ı��*/ 
	sprintf (pStr, "\"RECORDID\":%08d,", pRecord->nRecordIndex);
	pStr+=strlen(pStr);

   	
	/*6.����*/
	sprintf (pStr, "\"DATE\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 pRecord->aDateTime[0], pRecord->aDateTime[1],
			 pRecord->aDateTime[2], pRecord->aDateTime[3],
			 pRecord->aDateTime[4], pRecord->aDateTime[5]);
	pStr+=strlen(pStr);

     
	 /*7.����ģʽ*/
	sprintf (pStr, "\"MODE\":\"%d\",",pRecord->nTestMode);
	pStr+=strlen(pStr);

	 /*8.�����¶�*/
	sprintf (pStr, "\"TEMP\":%d,", pRecord->nTemperature);
	pStr+=strlen(pStr);

	 /*9.�ƾ�Ũ���ַ���*/
	fTemp=Get_Final_Convert_Display_Result(pRecord->fAlcValue,pRecord->nTestUnit, pRecord->nKBrACBAC);
	Get_Format_Alc_Convert_String(fTemp, pRecord->nTestUnit,aResult,eFormatToInt);// eFormatToFloat modified on 20181106
	sprintf (pStr, "\"ALC\":%s,", aResult);
	pStr+=strlen(pStr);

	/*10.���Ե�λ*/
	sprintf(pStr, "\"UNIT\":\"%s\",",Get_Unit_String(pRecord->nTestUnit));
	pStr+=strlen(pStr);

	/*11.����Ũ����ѪҺŨ��ת��ϵ��*/
	sprintf(pStr, "\"K\":%d,",pRecord->nKBrACBAC);
	pStr+=strlen(pStr);

	/*12.�ƾ�Ũ���ַ�������*/
	sprintf(pStr, "\"ALCSTRING\":\"%s%s\",",aResult,Get_Unit_String(pRecord->nTestUnit));
	pStr+=strlen(pStr);

	
	if(pRecord->fLongitude != 0 && pRecord->fLatitude != 0)
	{
	/*13.��������*/
	fLongitude=pRecord->fLongitude >= 0 ? pRecord->fLongitude : -(pRecord->fLongitude);
	sprintf(pStr, "\"LONGITUDE\":%f,",fLongitude);
	pStr+=strlen(pStr);
	
	/*14.γ������*/
	fLatitude=pRecord->fLatitude >= 0 ? pRecord->fLatitude : -(pRecord->fLatitude);
	sprintf(pStr, "\"Latitude\":%f,",fLatitude);
	pStr+=strlen(pStr);
	}
	else
	{
		/*13.��������*/
		sprintf(pStr, "\"LONGITUDE\":%s,","null");
		pStr+=strlen(pStr);
	
		/*14.γ������*/
		sprintf(pStr, "\"Latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*15.���Եص�*/
	sprintf(pStr, "\"LOCATION\":\"%s\",",(char *)pRecord->aLocation);
	pStr+=strlen(pStr);
 
	/*16.����������*/
	sprintf(pStr, "\"TESTEE\":\"%s\",",(char *)pRecord->aTestee);
	pStr+=strlen(pStr);

	/*17.��ʻ֤��*/
	sprintf(pStr, "\"DRIVERID\":\"%s\",",(char *)pRecord->aDriverLicense);
	pStr+=strlen(pStr);

	/*18.���ƺ�*/
	sprintf(pStr, "\"PLATENUMBER\":\"%s\",",(char *)pRecord->aPlateNumber);
	pStr+=strlen(pStr);

	/*19.ִ���񾯱��*/
	sprintf(pStr, "\"POLICEID\":\"%s\",",(char *)pRecord->aPoliceID);
	pStr+=strlen(pStr);

	/*20.ִ��������*/
	sprintf(pStr, "\"POLICENAME\":\"%s\",",(char *)pRecord->aPoliceName);
	pStr+=strlen(pStr);

	/*21.ִ����Ա��������*/
	sprintf(pStr, "\"DEPARTMENT\":\"%s\"",(char *)pRecord->aDepartment);
	pStr+=strlen(pStr);

	/*������*/
	*pStr++= '}';
	
	*pStr++='}';
	*pStr= 0x00;//added by lxl 20180813

	
	return (pStr - pPackageBuffer);//�������ݵ��ܳ��� 

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
        //str2��ֵΪpost������
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
        //str2��ֵΪpost������
        strcat(pStart, buffer);
        strcat(pStart, "\r\n\r\n");

   return strlen(pStart);//(pStart-pStart_TEMP);//added by lxl 
}
static short Get_HeartRate_Data(char *pPackageBuffer)
{
	char *pStr = pPackageBuffer;
	float fLongitude=0,fLatitude=0;
        float fCurrentLongitude,fCurrentLatitude;
        char s_aAdmin[] = {0xE7,0xAE,0xA1,0xE7,0x90,0x86,0xE5,0x91,0x98,0x00};   /*����Ա*/
        char s_department[5]={0x00};
	 
	fCurrentLongitude=g_bGPS_Location_Valid == 1  ? g_fLongitude : 0;
	fCurrentLatitude=g_bGPS_Location_Valid == 1  ? g_fLatitude: 0;
	
	*pStr++='{';
	sprintf(pStr,"\"heartrate\":%c",'{');
	pStr+=strlen(pStr);

	/*��¼����Ч���ݰ�����ʼ����*/
	//pStart = pStr;
	 /*1.�û���*/
	sprintf(pStr, "\"user\":%s,", username);
	pStr+=strlen(pStr);

	/*2.��¼����*/
	sprintf(pStr, "\"key\":%s,", KeyPassword);
	pStr+=strlen(pStr);

       	/*3.����ʱ��*/ 
	sprintf (pStr, "\"onlinedate\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 g_sDateTime.wYear, g_sDateTime.wMonth,
			 g_sDateTime.wDay, g_sDateTime.wHour,
			 g_sDateTime.wMinute, g_sDateTime.wSecond);
	pStr+=strlen(pStr);

	
	if(fCurrentLongitude!= 0 && fCurrentLatitude != 0)
	{
	/*4.��������*/
	fLongitude=fCurrentLongitude>= 0 ? fCurrentLongitude : -(fCurrentLongitude);
	sprintf(pStr, "\"longitude\":%f,",fLongitude);
	pStr+=strlen(pStr);
	
	/*5.γ������*/
	fLatitude=fCurrentLatitude >= 0 ? fCurrentLatitude : -(fCurrentLatitude);
	sprintf(pStr, "\"latitude\":%f,",fLatitude);
	pStr+=strlen(pStr);
	}
	else
	{
		/*4.��������*/
		sprintf(pStr, "\"longitude\":%s,","null");
		pStr+=strlen(pStr);
	
		/*5.γ������*/
		sprintf(pStr, "\"latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*6.ִ��������*/
	if(pApp->bLoginAsAdministrator)
		sprintf(pStr, "\"policename\":\"%s\",",(char *)s_aAdmin); //����Ա��¼
	else
		sprintf(pStr, "\"policename\":\"%s\",",(char *)(g_sCurrPoliceUserInfor.aPoliceName));
	pStr+=strlen(pStr);
	

	/*7.ִ����Ա��������*/
	if(pApp->bLoginAsAdministrator)
		sprintf(pStr, "\"department\":\"%s\",",s_department); //����Ա��¼
	else
		sprintf(pStr, "\"department\":\"%s\",",(char *)(g_sCurrPoliceUserInfor.aDepartment)); 
	pStr+=strlen(pStr);


	/*8.����״̬*/
	if(g_sXingJiKongGU.bDeviceState)
		sprintf(pStr, "\"state\":%d,",1); //����Ա��¼
	else
		sprintf(pStr, "\"state\":%d,",0); 
	pStr+=strlen(pStr);
	
	/*9.�豸����ַ���*/
	sprintf(pStr, "\"device\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);

	/*10.����ʱ��(S)*/
	sprintf(pStr, "\"heartbeat\":%d,", g_sHeartRateParam.byHeartRate);
	pStr+=strlen(pStr);
	
	//������
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

	/*��¼����Ч���ݰ�����ʼ����*/
	//pStart = pStr;
	 /*1.�û���*/
	sprintf(pStr, "\"user\":%s,", username);
	pStr+=strlen(pStr);

	/*2.��¼����*/
	sprintf(pStr, "\"key\":%s,", KeyPassword);
	pStr+=strlen(pStr);

       	/*3.����ʱ��*/ 
	sprintf (pStr, "\"onlinedate\":\"20%02d-%02d-%02d %02d:%02d:%02d\",",
			 g_sDateTime.wYear, g_sDateTime.wMonth,
			 g_sDateTime.wDay, g_sDateTime.wHour,
			 g_sDateTime.wMinute, g_sDateTime.wSecond);
	pStr+=strlen(pStr);

	
	if(fCurrentLongitude!= 0 && fCurrentLatitude != 0)
	{
		/*4.��������*/
		fLongitude=fCurrentLongitude>= 0 ? fCurrentLongitude : -(fCurrentLongitude);
		sprintf(pStr, "\"longitude\":%f,",fLongitude);
		pStr+=strlen(pStr);
	
		/*5.γ������*/
		fLatitude=fCurrentLatitude >= 0 ? fCurrentLatitude : -(fCurrentLatitude);
		sprintf(pStr, "\"latitude\":%f,",fLatitude);
		pStr+=strlen(pStr);
	}
	else
	{
		/*4.��������*/
		sprintf(pStr, "\"longitude\":%s,","null");
		pStr+=strlen(pStr);
	
		/*5.γ������*/
		sprintf(pStr, "\"latitude\":%s,","null");
		pStr+=strlen(pStr);
	}

	/*6.����״̬*/
	if(g_sXingJiKongGU.bDeviceState)
		sprintf(pStr, "\"state\":%d,",1); //����Ա��¼
	else
		sprintf(pStr, "\"state\":%d,",0); 
	pStr+=strlen(pStr);
	
	/*6.�豸����ַ���*/
	sprintf(pStr, "\"device\":\"%s%06d\",", g_sDeviceIDConfig.aNamePrefix, g_sDeviceIDConfig.nDeviceID);
	pStr+=strlen(pStr);

	/*8.����ʱ��(S)*/
	sprintf(pStr, "\"heartbeat\":%d,", g_sHeartRateParam.byHeartRate);
	pStr+=strlen(pStr);
	
	//������
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
        //str2��ֵΪpost������
        strcat(pStart, buffer);
        strcat(pStart, "\r\n\r\n");

   return strlen(pStart);//(pStart-pStart_TEMP);//added by lxl 
}
/*�豸����*/
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

/*��ѭ���е���*/
void Exec_GPRS_User_Loop(void)
{
	GPRS_Exce();//�����ĵ�¼�ʹ�����ݵĺ��� 20180809

	switch(g_sGPRSUser.eUserState)
	{
		case eGprsUserGetIMEI:
			if(g_sGPRSUser.bGetValidIMEI)//û������ҲҪ����
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
			//����Ƿ���������Ҫ�ϴ�,��û�н�����������ϴ����ݰ��ϴ� modified by lxl 20180930
			if(Is_ValidData_Need_Upload()&&(g_sXingJiKongGU.bResult_Packet_Flag==false))//
			{
				if(_Is_GPRS_System_PoweOn() == false)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
				}
				if(Get_New_Upload_Record_Data()) //�Ӵ˺����л�ȡ���ݼ�¼
				{
					GprsUser_Switch_State(eGprsUserUploadRecordData);// eGprsUserLoginServer Modified by lxl 20180809
					g_sXingJiKongGU.bReveiceAckFromServer=true;//g_ReveiceAckFromServer=true;//ֻҪһ�������ݾ���1, ֱ���ü�¼���ϴ����Modified by lxl 20180817
				}
			}

			break;

		case eGprsUserUploadRecordData: //�ϴ����� �����޸�Ϊhttp��Э���ʽ�����ϴ����ݣ�ֻ��Ҫ�������޸� 20180808
			if(g_sGPRSUser.bSwitchNewState)
			{
				g_sGPRSUser.bSwitchNewState = false;
				g_sGPRSUser.nStartWaitTime = Get_System_Time();
				memset(GPRS_Tx_Buffer,0,sizeof(GPRS_Tx_Buffer));//added by lxl 20180816
				g_nGprs_Tx_Cnt = Get_GPRS_To_Server_Record_Packet(GPRS_Tx_Buffer);//��ȡ���ݼ�¼ 20180704 lxl
				g_sGPRSUser.nCurrentSendPacketSize = g_nGprs_Tx_Cnt;
				//g_ReveiceAckFromServer=true;//�������ݼ�¼����־λ added by lxl 20180816
				
				TRACE_PRINTF("GPRS_Tx_Buffer:%s\r\n",GPRS_Tx_Buffer);
				TRACE_PRINTF("GPRS-->Get_GPRS_To_Server_Record_Packet! g_nGprs_Tx_Cnt = %d\r\n",g_nGprs_Tx_Cnt);
			}
                      
			if(Get_System_Time() > g_sGPRSUser.nStartWaitTime + 30000)
			{
				GprsUser_SendData_Timeout_Handle(__FILE__, __LINE__);
			}
			else if(g_nGprs_Tx_Cnt == 0)//ÿ�ϴ�һ�������ۼ�һ���ϴ���¼����
			{
				if(g_sGPRSUser.bIsReveiceAckFromServer)
				{
					Update_Already_Upload_Record_Counter();//�ϴ���¼��ɣ���ʼ��¼���ϴ���¼��
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
