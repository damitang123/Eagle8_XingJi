#ifndef _GPRS_H_
#define _GPRS_H_

#define GPRS_FULL_ECHO_MESSAGE	1
#define GPRS_MAX_RESTART_TIME		10

typedef enum
{
	eGSM_None,						/*0*/
	eGSM_Idle,
	eGSM_PowerOff,
	eGSM_PowerOffDelay,
	eGSM_PowerUp,					
	eGSM_UartSync,					/*5*/
	eGSM_ChangeBaudRate,
	eGSM_Enable_Echo,				
	eGSM_Disable_Echo,				
	eGSM_SimCard_Check,			
	eGSM_Get_IMEI,					/*10*/	
	eGSM_GET_CSQ,
	eGSM_CheckCOPS,
	eGSM_NetRegist_Check,			
	eGSM_Get_MCC_MNC,				
	eGSM_Set_APN,					/*15*/			
	eGSM_Open_NET,
	eGSM_Socket_Create,
	eGSM_Domain_Name_Resolve,	
	eGSM_Connect_Server,			
	eGSM_Write_Socket_Phase1,		/*20*/	
	eGSM_Write_Socket_Phase2,		
	eGSM_Read_Socket,
	eGSM_Add_MessageHead,
	eGSM_Boot_Delay,
} eGSM_State;

typedef enum
{
	eGPRSPowerDownNormal,
	eGPRSPowerDownAlways,
	eGPRSPowerRestart,
}ePowerCtrlParameter;

typedef struct
{
	eGSM_State		nNowState;
	eGSM_State		nNewState;
	bool				bChangeNewState;
	bool				bInvalidSimCard;	/*SIM卡状况*/
	bool				bTcpConnectState;	/*true表示已连接,false表示未连接*/	
	bool				bStartReceiveData;
} __attribute__ ((packed))sGPRS;

typedef struct
{
	unsigned short	nStartPosition;
	unsigned short	nEndPosition;
	bool 			bFinishReceive;
	bool				bInvalidBuffer;
} __attribute__ ((packed))sGPRSReceiveQ;

typedef struct
{
	eGSM_State		nNowState;
	char				*pSendCmd;
	unsigned short	nCmdLength;
	bool				bIsCmdBusy;
	unsigned char 	nRetryCnt;
	unsigned long 		nStartWaitTime;
	unsigned short	nWaitAckTimeout;
	bool				bDefaultProcessTimeout;					/*是否超时默认处理*/
	bool				bDefaultProcessExceedMaxRetryCnt;		/*最大尝试次数用完是否默认处理.*/
	bool				bInvalidBuffer;
} __attribute__ ((packed))sGPRSSendCmdQ;

typedef enum
{
	eGPRSNormalRoll,
	eGPRSChangeState,
	eGPRSATData,
	eGPRSCmdTimeout,
	eGPRSExceedMaxRetryCnt,
	eGPRSReturnCallOK,
	eGPRSReturnCallFail,
}eGPRSMessage;

extern sGPRS g_sGPRS;
extern bool g_bGprsPowerOnState;
extern signed char g_nGprsSingalQuality;
extern char GPRS_Tx_Buffer[1500];
extern volatile unsigned short g_nGprs_Tx_Cnt;

void GPRS_Exce(void);
void GPRS_Hal_Var_Init(void);
void GPRS_Receive_Data(unsigned char nData);
bool Get_Gprs_Tcp_Connect_State(void);
bool GPRS_Power_Control(ePowerCtrlParameter eArgument, const char *FileName, int nLineNumber);
void Mark_Valid_Package_Via_Timeout(void);
void GprsHal_Switch_State(eGSM_State eNewState);
void GPRS_FSM_Sub(eGPRSMessage eMessageType);

#endif

