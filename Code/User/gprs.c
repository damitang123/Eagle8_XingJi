#include "platform.h"
#include "delay.h"
#include "gprs.h"
#include "system_misc.h"
#include "spi_flash.h"
#include "gprs_user.h"
#include "app.h" //added by lxl 20180928
#if SERVER_USE_XING_JIKONG_GU
#include "parameter.h" //added by lxl 20181010
#endif

/***************************特别说明*********************************/
/*我在才文件中加了信号量强度表示为,大于2置一，否则为0  added  by lxl 20180816*/
/******************************************************************/

#if defined(USE_4G_PCB) && (USE_4G == 0)
#error "Please note slect 4G gprs config!"
#endif

#if USE_GPRS

sGPRS g_sGPRS;
signed char g_nGprsSingalQuality = -1;
char GPRS_Tx_Buffer[1500];
volatile unsigned short g_nGprs_Tx_Cnt;
bool g_bGprsPowerOnState = false;
ePowerCtrlParameter g_ePowerCtrlParameter;

static unsigned short s_nGprsRestartCnt = 0;
static volatile unsigned long s_nStartReceiveTime;

#if USE_G100

unsigned short s_MCC,s_MNC;
unsigned char s_nSocketNum;

/*硬件相关部分--开始*************************************************************************/

#define GPRS_POWER_ON()  			GPIOC->BSRR=GPIO_Pin_12
#define GPRS_POWER_OFF()  			GPIOC->BRR=GPIO_Pin_12	

#define GPRS_RESET_HIGH()			GPIOA->BSRR=GPIO_Pin_12
#define GPRS_RESET_LOW()			GPIOA->BRR=GPIO_Pin_12

#define GPRS_POWER_KEY_HIGH()		GPIOB->BSRR=GPIO_Pin_2
#define GPRS_POWER_KEY_LOW()		GPIOB->BRR=GPIO_Pin_2

static void GPRS_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*POWER-ON-PIN*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPRS_POWER_KEY_HIGH();

	/*GSM Reset*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPRS_RESET_LOW();
	
	/*GSM Power-EN,3.8V电源*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPRS_POWER_OFF();
}

static void GPRS_Uart_Init(unsigned long nBaudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* Configure USARTx_Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure USARTx_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure the USARTx */ 
	USART_Init(UART4, &USART_InitStructure);

	//串口接收中断开启
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); 

	/* Enable the USARTx */
	USART_Cmd(UART4, ENABLE);

	USART_ClearFlag(UART4, USART_FLAG_TC);
}


void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		GPRS_Receive_Data(USART_ReceiveData(UART4));
	}
}

void GPRS_Transmit_Data(char  *pData, unsigned short nLen)
{
	int i;
	if (nLen)
	{
		for(i = 0; i < nLen; i++)
		{
			USART_SendData(UART4, (unsigned char)pData[i]);
			while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
		}
	}
}




/*硬件相关部分--结束*************************************************************************/

#define GPRS_RECEIVE_Q_SIZE		10
sGPRSReceiveQ g_sGPRSReceiveQ[GPRS_RECEIVE_Q_SIZE];
static sGPRSReceiveQ *pCurrGPRSRevQ;
#define GPRS_SEND_Q_SIZE			3
sGPRSSendCmdQ g_sGPRSSendCmdQ[GPRS_SEND_Q_SIZE];

#define GPRS_REV_BUFFER_SIZE		1000
#define GPRS_PROCESS_BUFFER_SIZE	200
static volatile unsigned short s_nUartInCnt = 0, s_nUartOutCnt = 0;
static char GPRS_Rx_Buffer[GPRS_REV_BUFFER_SIZE];
static char GPRS_Process_Buffer[GPRS_PROCESS_BUFFER_SIZE];
static char *pGPRSProcessPtr = NULL;

static void Init_ReceiveQ(void)
{
	int i;
	pCurrGPRSRevQ = NULL;
	for(i = 0; i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		memset(&g_sGPRSReceiveQ[i],0,sizeof(sGPRSReceiveQ));
	}
}

static sGPRSReceiveQ * Get_Free_ReceiveQ(void)
{
	int i;
	for(i = 0;i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		if(g_sGPRSReceiveQ[i].bInvalidBuffer == false)
		{
			g_sGPRSReceiveQ[i].bInvalidBuffer = true;
			g_sGPRSReceiveQ[i].bFinishReceive = false;
			g_sGPRSReceiveQ[i].nStartPosition = s_nUartInCnt;
			return &g_sGPRSReceiveQ[i];
		}
	}
	return NULL;
}

static void Init_SendCmdQ(void)
{
	int i;
	for(i = 0; i < GPRS_SEND_Q_SIZE; i++)
	{
		memset(&g_sGPRSSendCmdQ[i],0,sizeof(sGPRSSendCmdQ));
	}
}

static sGPRSSendCmdQ * Get_Free_SendCmdQ(void)
{
	int i;
	for(i = 0;i < GPRS_SEND_Q_SIZE; i++)
	{
		if(g_sGPRSSendCmdQ[i].bInvalidBuffer == false)
		{
			g_sGPRSSendCmdQ[i].nRetryCnt = 10;
			g_sGPRSSendCmdQ[i].pSendCmd = NULL;
			g_sGPRSSendCmdQ[i].bIsCmdBusy = true;
			g_sGPRSSendCmdQ[i].nCmdLength = 0;
			g_sGPRSSendCmdQ[i].nStartWaitTime = Get_System_Time();
			g_sGPRSSendCmdQ[i].nWaitAckTimeout = 200;
			g_sGPRSSendCmdQ[i].bDefaultProcessTimeout = true;
			g_sGPRSSendCmdQ[i].bDefaultProcessExceedMaxRetryCnt = true;
			g_sGPRSSendCmdQ[i].bInvalidBuffer = true;
			return &g_sGPRSSendCmdQ[i];
		}
	}
	return NULL;
}

static sGPRSSendCmdQ * Get_Last_Valid_SendCmdQ(void)
{
	int i;
	sGPRSSendCmdQ *p = NULL;
	for(i = 0;i < GPRS_SEND_Q_SIZE; i++)
	{
		if(g_sGPRSSendCmdQ[i].bInvalidBuffer)
		{
			p = &g_sGPRSSendCmdQ[i];
		}
		else
		{
			break;
		}
	}
	return p;
}

/*外部调用的函数************************************************************************************/

/*此函数要被定时器中断函数调用*/
void Mark_Valid_Package_Via_Timeout(void)
{
	if(g_sGPRS.bStartReceiveData)
	{
		if(Get_System_Time() > s_nStartReceiveTime + 35)
		{	
			g_sGPRS.bStartReceiveData = false;
			if(pCurrGPRSRevQ)
			{
				pCurrGPRSRevQ->bFinishReceive = true;
				pCurrGPRSRevQ->nEndPosition = s_nUartInCnt;
			}
		}
	}
}

bool Get_Gprs_Tcp_Connect_State(void)
{
	return g_sGPRS.bTcpConnectState;
}

bool GPRS_Power_Control(ePowerCtrlParameter eArgument, const char *FileName, int nLineNumber)
{
	TRACE_PRINTF("GPRS module  is power down.(Argument = %d (0:Normal, 1:Always, 2:Restart), Call by \" %s \" ,Line = %d)\r\n",eArgument, FileName ,nLineNumber);
	
	GPRS_POWER_OFF();
	g_nGprsSingalQuality = -1;
	g_bGprsPowerOnState = false;
	Gprs_Set_UserLogin_State(false);
	GprsHal_Switch_State(eGSM_Idle);

	switch(eArgument)
	{
		case eGPRSPowerDownNormal:
			s_nGprsRestartCnt = 0;
			GprsUser_Switch_State(eGprsUserGetIMEI);
			return true;

		case eGPRSPowerDownAlways:
			s_nGprsRestartCnt = GPRS_MAX_RESTART_TIME;
			GprsUser_Switch_State(eGprsUserNothing);
			return false;

		case eGPRSPowerRestart:
			if(++s_nGprsRestartCnt < GPRS_MAX_RESTART_TIME)
			{
				GPRS_Hal_Var_Init();
				TRACE_PRINTF("s_nGprsRestartCnt = %d!\r\n",s_nGprsRestartCnt);
				GprsUser_Switch_State(eGprsUserGetIMEI);
				return true;
			}
			GprsUser_Switch_State(eGprsUserNothing);
			return false;
	}
	
	return false;
}

/*外部调用的函数************************************************************************************/

char* Get_APN_String(unsigned short nMCC, unsigned short nMNC)
{
	typedef struct
	{
		unsigned short MCC;
		unsigned short MNC;
		char	 APN_String[50];
	} APN_Lookup_Struct;

	const APN_Lookup_Struct ARRAY_APN_Lookup_Table[] =
	{
		//China
		460,		0,	"CMNET",
		460,		1,	"UNINET",

		//Russia
		250,		1,	"internet.mts.ru,mts,mts",
		250,		2,	"internet",
		250,		99,	"internet.beeline.ru,beeline,beeline",
		250,    	20, 	"internet.tele2.ru",

		//INDIA
		404,   	29,  "aircelgprs",

		//Australia
		505,  	 01,  "telstra.internet",

		//巴拉圭客户
		744,		1,	"vox.internet.py",
		744,		2,	"igprs.claro.com.py",
		744,		4,	"internet.tigo.py",
		744,		5,	"internet",
	};

	unsigned short i;
	APN_Lookup_Struct *p;

	p=(APN_Lookup_Struct *)ARRAY_APN_Lookup_Table;
	
	for(i=0;i<ARR_SIZE(ARRAY_APN_Lookup_Table);i++)
	{
		if(p ->MCC ==nMCC && p->MNC ==nMNC)
		{
			return p->APN_String;
		}
		p++;
	}

	return NULL;
}


static void GPRS_Hal_Var_Init(void)
{
	Init_SendCmdQ();
	Init_ReceiveQ();
	s_nUartInCnt = 0;
	s_nUartOutCnt = 0;
	memset(&g_sGPRS, 0, sizeof(g_sGPRS));
	g_nGprsSingalQuality = -1;
	g_bGprsPowerOnState = true;
	GprsHal_Switch_State(eGSM_PowerUp);
}

void GPRS_Receive_Data(unsigned char nData)
{
	if(!g_sGPRS.bStartReceiveData)
	{
		g_sGPRS.bStartReceiveData = true;
		pCurrGPRSRevQ = Get_Free_ReceiveQ();
	}
	
	GPRS_Rx_Buffer[s_nUartInCnt] = nData;
	s_nStartReceiveTime = Get_System_Time();
	if(++s_nUartInCnt >= GPRS_REV_BUFFER_SIZE)
	{
		s_nUartInCnt = 0;
	}
}

static unsigned char GPRS_Get_Uart_Data(void)
{
	unsigned char nTemp;
	nTemp = GPRS_Rx_Buffer[s_nUartOutCnt];
	if(++s_nUartOutCnt >= GPRS_REV_BUFFER_SIZE)
	{
		s_nUartOutCnt = 0;
	}
	return nTemp;
}

void GprsHal_Switch_State(eGSM_State eNewState)
{
	g_sGPRS.nNewState = eNewState;
}

static void  GPRS_Power_Off(eGPRSMessage eMessageType)
{
	char *pStr;
	static sGPRSSendCmdQ *p = NULL;
	
	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_PowerOff;
				p->nWaitAckTimeout = 1500;
				p->bDefaultProcessTimeout = false;
				GPRS_POWER_OFF();
				TRACE_PRINTF("GPRS module  is power down....\r\n");
			}
			break;
			
		case eGPRSATData:
			/*在关机过程中收到此消息,表明关机完成..*/
			pStr = strstr(pGPRSProcessPtr,"NORMAL POWER DOWN");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GPRS_POWER_OFF();
				GprsHal_Switch_State(eGSM_PowerOffDelay);
			}
			break;	
			
		case eGPRSCmdTimeout:
			p->bInvalidBuffer = false;
			GPRS_POWER_OFF();
			GprsHal_Switch_State(eGSM_PowerOffDelay);
			break;
	}
}

static void  GPRS_Power_Off_Delay(eGPRSMessage eMessageType)
{
	static sGPRSSendCmdQ *p = NULL;
	
	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_PowerOffDelay;
				p->nWaitAckTimeout = 1500;
				p->bDefaultProcessTimeout = false;
				TRACE_PRINTF("GPRS module finish power down(delaying....,wait for restart)!\r\n");
			}
			break;
			
		case eGPRSCmdTimeout:
			p->bInvalidBuffer = false;
			if(g_ePowerCtrlParameter == eGPRSPowerRestart && s_nGprsRestartCnt < GPRS_MAX_RESTART_TIME)
			{
				GprsHal_Switch_State(eGSM_PowerUp);
			}
			else
			{
				GprsHal_Switch_State(eGSM_Idle);
			}
			break;
	}
}


static void  GPRS_Power_Up(eGPRSMessage eMessageType)
{
	char *pStr;
	static sGPRSSendCmdQ *p = NULL;
	
	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_PowerUp;
				p->nWaitAckTimeout = 3000;
				p->bDefaultProcessTimeout = false;
				GPRS_Gpio_Init();
				GPRS_Uart_Init(115200);
				GPRS_POWER_ON();
				GPRS_POWER_KEY_LOW();
				delay_ms(10);
				GPRS_RESET_HIGH();
				TRACE_PRINTF("GPRS module  is booting....\r\n");
			}
			break;

		case eGPRSATData:
			/*在启动过程中收到此消息,表明启动失败.需要重新启动.*/
			pStr = strstr(pGPRSProcessPtr,"NORMAL POWER DOWN");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_PowerOffDelay);
				g_ePowerCtrlParameter = eGPRSPowerRestart;
			}
			break;
			
		case eGPRSCmdTimeout:
			p->bInvalidBuffer = false;
			GprsHal_Switch_State(eGSM_UartSync);
			break;
	}
}

static void GPRS_UartSync(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_UartSync;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				p->nWaitAckTimeout = 1000;
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Enable_Echo);
				TRACE_PRINTF("GPRS-->Hardware is Ok.\r\n");
			}
			break;
	}
}

static void GPRS_Enable_Echo(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "ATE1\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Enable_Echo;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"ATE1\r\r\nOK\r\n");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Get_IMEI);
				TRACE_PRINTF("GPRS-->Enable Echo.\r\n");
			}
			break;
	}
}

static void GPRS_Cmd_Get_IMEI(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CGSN\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Get_IMEI;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+CGSN\r\r\n");
			if(pStr)
			{
				if(strlen(pGPRSProcessPtr) > 20)
				{
					pStr = strstr(pStr,"\r\n");
					Save_Hardware_IMEI(pStr + 2);
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_SimCard_Check);
					TRACE_PRINTF("GPRS-->IMEI = %s.\r\n",Get_Hardware_IMEI());
				}
			}
			break;
	}
}

unsigned char GPRS_Get_Signal_Quality(unsigned char nValue)
{
	if(nValue < 4 || nValue == 99)
	{
		return 0;
	}
	else if(nValue >= 4 &&  nValue < 10)
	{
		return 1;
	}
	else if(nValue >= 10 &&  nValue < 16)
	{
		return 2;
	}
	else if(nValue >= 16 &&  nValue < 22)
	{
		return 3;
	}
	else if(nValue >= 22 &&  nValue < 28)
	{
		return 4;
	}
	else if(nValue >= 28)
	{
		return 5;
	}

	return 0;
}

static void GPRS_Cmd_CheckSimcard(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CPIN?\r\n";
	static sGPRSSendCmdQ *p = NULL;
	static char nResultErrorCnt;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_SimCard_Check;
				p->nRetryCnt = 20;
				p->nWaitAckTimeout = 500;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				nResultErrorCnt = 0;
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+CPIN?\r");
			if(pStr)
			{
				if(strstr(pStr,"+CPIN: READY"))
				{
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_GET_CSQ);
					TRACE_PRINTF("GPRS-->Sim Card Ready!\r\n");
				}
				else if(strstr(pStr,"ERROR"))
				{
					TRACE_PRINTF("GPRS-->SIMCard not inserted(Can't detected the valid Simcard)!\r\n");
					if(++nResultErrorCnt > 15)
					{
						GPRS_Power_Control(eGPRSPowerDownAlways, __FILE__, __LINE__);
						Notify_User_Invalid_Simcard();
					}
				}
			}
			break;
	}
}

static void GPRS_Cmd_Get_CSQ(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CSQ\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_GET_CSQ;
				p->nRetryCnt = 40;
				p->nWaitAckTimeout = 1000;
				if(g_sGPRS.bTcpConnectState)
				{
					p->nWaitAckTimeout = 2000;
				}
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"+CSQ:");
			if(pStr)
			{
				int nValue;
				nValue = atoi(pStr + 6);
				if(nValue && (nValue != 99))
				{
					p->bIsCmdBusy = false;
					g_nGprsSingalQuality = GPRS_Get_Signal_Quality(nValue);
					TRACE_PRINTF("GPRS-->g_nGprsSingalQuality = %d (+CSQ:%d).\r\n",g_nGprsSingalQuality, nValue);
					if(g_sGPRS.bTcpConnectState)
					{
						p->nRetryCnt = 40;
					}
					else
					{
						p->bInvalidBuffer = false;
						GprsHal_Switch_State(eGSM_NetRegist_Check);
					}
				}
			}
			break;

		case eGPRSNormalRoll:
			if(p->bIsCmdBusy) return;

			if(g_sGPRS.bTcpConnectState)
			{
				if(g_nGprs_Tx_Cnt)
				{
					GprsHal_Switch_State(eGSM_Write_Socket_Phase1);
				}
			}
			break;
	}
}

static void GPRS_Cmd_CheckNetRegist_State(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	const char *pCmdStr = "AT+CGATT?\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_NetRegist_Check;
				p->nRetryCnt = 60;
				p->nWaitAckTimeout = 1000;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"AT+CGATT?"))
			{
				pStr = strstr(pGPRSProcessPtr,"+CGATT:");
				if(pStr)
				{
					TRACE_PRINTF("GPRS-->NetRegist state(%c).\r\n", *(pStr + 8));
					if(*(pStr + 8) == '1')
					{
						p->bInvalidBuffer = false;
						GprsHal_Switch_State(eGSM_Get_MCC_MNC);
					}
				}
			}
			break;
	}
}

void GPRS_Cmd_GET_MCC_MNC(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	const char *pCmdStr = "AT+CGED=3\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Get_MCC_MNC;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"MCC:");
			if(pStr)
			{
				s_MCC=atoi(pStr + 4);
				pStr = strstr(pGPRSProcessPtr,"MNC:");
				if(pStr)
				{
					s_MNC=atoi(pStr + 4);
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_Set_APN);
					TRACE_PRINTF("GPRS-->MCC =%d, MNC = %d!\r\n",s_MCC, s_MNC);
				}
			}
			break;
	}
}

void GPRS_Cmd_Set_APN(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	static char aCmdStr[60];
	static sGPRSSendCmdQ *p = NULL;
	sprintf(aCmdStr,"AT+UPSD=0,1,\"%s\"\r\n",Get_APN_String(s_MCC,s_MNC));

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Set_APN;
				p->pSendCmd = (char *)aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"OK");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Open_NET);
				TRACE_PRINTF("GPRS-->Set APN string = %s\r\n",Get_APN_String(s_MCC,s_MNC));
			}
			break;
	}
}

void GPRS_Cmd_Active_GPRS(eGPRSMessage eMessageType)
{
	const char *pCmdStr = "AT+UPSDA=0,3\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Open_NET;
				p->nRetryCnt = 3;
				p->nWaitAckTimeout = 10000;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"OK"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Socket_Create);
				TRACE_PRINTF("GPRS-->Already active socket!\r\n");
			}
			break;
	}
}

void GPRS_Cmd_Create_Socket(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	const char *pCmdStr = "AT+USOCR=6\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Socket_Create;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"+USOCR:");
			if(pStr)
			{
				s_nSocketNum = atoi(pStr + 7);
				p->bInvalidBuffer = false;
				TRACE_PRINTF("GPRS-->Already create socket(num = %d)!\r\n",s_nSocketNum);
				if(Get_Net_AccessMode())
				{
					GprsHal_Switch_State(eGSM_Domain_Name_Resolve);
				}
				else
				{
					GprsHal_Switch_State(eGSM_Connect_Server);
				}
			}
			break;
	}
}

void GPRS_Cmd_Domain_Name_Resolve(eGPRSMessage eMessageType)
{
	
}



static void GPRS_Cmd_Connect_Server(eGPRSMessage eMessageType)
{
	static char aCmdStr[80];
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Connect_Server;
				p->nRetryCnt = 2;
				p->nWaitAckTimeout = 15000;
				sprintf(aCmdStr,"AT+USOCO=%d,\"%s\",%d\r\n",s_nSocketNum,
							Get_Net_AccessMode() ? Get_Net_RemoteURL() : Get_Net_RemoteIP(),
							Get_Net_RemotePort());
				p->pSendCmd = aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				TRACE_PRINTF("Connect server Cmd string = %s",aCmdStr);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"OK"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Disable_Echo);
				g_sGPRS.bTcpConnectState = true;
				TRACE_PRINTF("GPRS-->Connect server successful!\r\n");
			}
			else if(strstr(pGPRSProcessPtr,"ERROR"))
			{
				GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
			}
			break;
	}
}

static void GPRS_Disable_Echo(eGPRSMessage eMessageType)
{
	const char *pCmdStr = "ATE0\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Disable_Echo;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"ATE0\r\r\nOK\r\n"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_GET_CSQ);
				TRACE_PRINTF("GPRS-->Disable Echo!\r\n");
			}
			break;
	}
}

static void GPRS_Cmd_Write_Socket_Phase2(eGPRSMessage eMessageType)
{
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Write_Socket_Phase2;
				p->nRetryCnt = 2;
				p->nWaitAckTimeout = 15000;
				p->pSendCmd = GPRS_Tx_Buffer;
				p->nCmdLength = g_nGprs_Tx_Cnt;
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"+USOWR:"))
			{
				g_nGprs_Tx_Cnt = 0;
				p->bInvalidBuffer = false;
				TRACE_PRINTF("Write socket data result-->OK\r\n");
			}
			else if(strstr(pGPRSProcessPtr,"ERROR"))
			{
				TRACE_PRINTF("Write socket data result-->FAIL\r\n");
				GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
			}
			break;
	}
}

static void GPRS_Cmd_Write_Socket_Phase1(eGPRSMessage eMessageType)
{
	char *pStr;
	static char aCmdStr[50];
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Write_Socket_Phase1;
				p->nRetryCnt = 2;
				sprintf(aCmdStr,"AT+USOWR=%d,%d\r\n", s_nSocketNum, g_nGprs_Tx_Cnt);
				p->pSendCmd = aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				TRACE_PRINTF("Start write socket data.\r\n");
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"@");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GPRS_Transmit_Data(GPRS_Tx_Buffer,g_nGprs_Tx_Cnt);
				g_nGprs_Tx_Cnt = 0;
			}
			break;
	}
}

/************************************************************************************************************************/
bool GPRS_Processed_Receive_Data(void)
{
	int i;
	char * pStr;
	sGPRSReceiveQ *pPrcoQ;
	static unsigned short nLen = 0;
	for(i = 0; i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		if(g_sGPRSReceiveQ[i].bFinishReceive)
		{
			pPrcoQ = &g_sGPRSReceiveQ[i];
			g_sGPRSReceiveQ[i].bFinishReceive = false;
			goto _Next;
		}
	}
	return false;
	
_Next:
	/*求当前包的长度*/
	if(pPrcoQ->nEndPosition >= pPrcoQ->nStartPosition)
	{
		nLen = pPrcoQ->nEndPosition - pPrcoQ->nStartPosition;
	}
	else
	{
		nLen = pPrcoQ->nEndPosition + GPRS_REV_BUFFER_SIZE - pPrcoQ->nStartPosition;
	}
	
	if(nLen > GPRS_PROCESS_BUFFER_SIZE)
	{
		TRACE_PRINTF("GPRS-->Not enough buffer to process this message!\r\n");
		return false;
	}

	/*复制数据到处理的内存.*/
	s_nUartOutCnt = pPrcoQ->nStartPosition;
	memset(GPRS_Process_Buffer,0,sizeof(GPRS_Process_Buffer));
	for(i = 0;i < nLen; i++)
	{
		if(i >= GPRS_PROCESS_BUFFER_SIZE)
		{	
			break;
		}
		GPRS_Process_Buffer[i] = GPRS_Get_Uart_Data();
	}
	pPrcoQ->bInvalidBuffer = false;

	pGPRSProcessPtr = GPRS_Process_Buffer;
#if GPRS_FULL_ECHO_MESSAGE
	TRACE_PRINTF("++Gprs_Receive(Bytes=%02d)>\r\n%s\r\n",nLen,pGPRSProcessPtr);
#endif

	do
	{
		pStr = strstr(pGPRSProcessPtr,"\r\n");
		if(pStr)
		{
			unsigned short nLineLen;
			nLineLen =  pStr - pGPRSProcessPtr + 2;
			if(nLineLen == 2)
			{
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+UUSORD:"))
			{
				char aCmdString[30];
				char *pStrTemp = NULL;
				unsigned short nDataLength,nSocketNum;
				pStrTemp = strstr(pGPRSProcessPtr,"+UUSORD:");
				nSocketNum = (unsigned short)atoi(pStrTemp + 8);
				nDataLength = (unsigned short)atoi(strstr(pStrTemp, ",") + 1);
				sprintf(aCmdString,"AT+USORD=%d,%d\r\n", nSocketNum, nDataLength);
				GPRS_Transmit_Data(aCmdString, strlen(aCmdString));
				TRACE_PRINTF("GPRS-->Start get socket data!\r\n(%s)",aCmdString);
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+USORD:"))
			{
				char *pStrTemp = NULL;
				unsigned short nDataLength;
				pStrTemp = strstr(pGPRSProcessPtr,"+USORD:");
				pStrTemp = strstr(pStrTemp, ",");
				nDataLength = (unsigned short)atoi(pStrTemp + 1);
				pStrTemp = strstr(pStrTemp,",");
				Gprs_Analyse_Server_Package_Data(strstr(pStrTemp, "\"") + 1, nDataLength);
				pGPRSProcessPtr += (strstr(pStrTemp, "\"") - strstr(pGPRSProcessPtr,"+USORD:") + 2 + nDataLength);
			}
			else if(strstr(pGPRSProcessPtr,"+UUSOCL"))
			{
				if(g_bGprsPowerOnState)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
					TRACE_PRINTF("GPRS-->Tcp ConnectState is closed by remote server!\r\n");
				}
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"RING"))
			{
				/*直接挂电话,拒接任何电话*/
				GPRS_Transmit_Data("ATH\r\n",9);
				TRACE_PRINTF("GPRS-->Someone call in, auto hung up!\r\n");
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+CMTI: \"SM\""))
			{
				GPRS_Transmit_Data("AT+CMGD=1,4\r\n",9);
				TRACE_PRINTF("GPRS-->Receice  SMS message, auto delete!\r\n");
				pGPRSProcessPtr += nLineLen;
			}
			else
			{
				GPRS_FSM_Sub(eGPRSATData);
				pGPRSProcessPtr += nLineLen;
			}
		}
		else
		{
			if(strstr(pGPRSProcessPtr,"+USORD:"))
			{
				char *pStrTemp = NULL;
				unsigned short nDataLength;
				pStrTemp = strstr(pGPRSProcessPtr,"+USORD:");
				pStrTemp = strstr(pStrTemp, ",");
				nDataLength = (unsigned short)atoi(pStrTemp + 1);
				pStrTemp = strstr(pStrTemp,",");
				Gprs_Analyse_Server_Package_Data(strstr(pStrTemp, "\"") + 1, nDataLength);
				pGPRSProcessPtr += (strstr(pStrTemp, "\"") - strstr(pGPRSProcessPtr,"+USORD:") + 2 + nDataLength);
			}
			else
			{
				GPRS_FSM_Sub(eGPRSATData);
				pGPRSProcessPtr += nLen;
			}
		}
	}while((pGPRSProcessPtr - GPRS_Process_Buffer) <  nLen);
	
	return true;
}


void GPRS_FSM_Sub(eGPRSMessage eMessageType)
{
	switch(g_sGPRS.nNowState)
	{
		case eGSM_Idle:
			break;

		case eGSM_PowerOff:
			GPRS_Power_Off(eMessageType);
			break;

		case eGSM_PowerOffDelay:
			GPRS_Power_Off_Delay(eMessageType);
			break;
			
		case eGSM_PowerUp:
			GPRS_Power_Up(eMessageType);
			break;

		case eGSM_UartSync:
			GPRS_UartSync(eMessageType);
			break;

		case eGSM_Enable_Echo:
			GPRS_Enable_Echo(eMessageType);
			break;

		case eGSM_Get_IMEI:
			GPRS_Cmd_Get_IMEI(eMessageType);
			break;
			
		case eGSM_SimCard_Check:
			GPRS_Cmd_CheckSimcard(eMessageType);
			break;

		case eGSM_GET_CSQ:
			GPRS_Cmd_Get_CSQ(eMessageType);
			break;

		case eGSM_NetRegist_Check:
			GPRS_Cmd_CheckNetRegist_State(eMessageType);
			break;

		case eGSM_Get_MCC_MNC:
			GPRS_Cmd_GET_MCC_MNC(eMessageType);
			break;

		case eGSM_Set_APN:
			GPRS_Cmd_Set_APN(eMessageType);
			break;

		case eGSM_Open_NET:
			GPRS_Cmd_Active_GPRS(eMessageType);
			break;

		case eGSM_Socket_Create:
			GPRS_Cmd_Create_Socket(eMessageType);
			break;

		case eGSM_Domain_Name_Resolve:
			GPRS_Cmd_Domain_Name_Resolve(eMessageType);
			break;

		case eGSM_Connect_Server:
			GPRS_Cmd_Connect_Server(eMessageType);
			break;

		case eGSM_Disable_Echo:
			GPRS_Disable_Echo(eMessageType);
			break;

		case eGSM_Write_Socket_Phase1:
			GPRS_Cmd_Write_Socket_Phase1(eMessageType);
			break;

		case eGSM_Write_Socket_Phase2:
			GPRS_Cmd_Write_Socket_Phase2(eMessageType);
			break;
	}
}

const char * GPRS_Cmd_StateString[] =
{
	"eGSM_None",						/*0*/
	"eGSM_Idle",
	"eGSM_PowerOff",
	"eGSM_PowerOffDelay",
	"eGSM_PowerUp",					
	"eGSM_UartSync",					/*5*/
	"eGSM_ChangeBaudRate",
	"eGSM_Enable_Echo",				
	"eGSM_Disable_Echo",				
	"eGSM_SimCard_Check",			
	"eGSM_Get_IMEI",					/*10*/	
	"eGSM_GET_CSQ",
	"eGSM_CheckCOPS",
	"eGSM_NetRegist_Check",			
	"eGSM_Get_MCC_MNC",				
	"eGSM_Set_APN",					/*15*/		
	"eGSM_Socket_Create",				
	"eGSM_Open_NET",				
	"eGSM_Domain_Name_Resolve",	
	"eGSM_Connect_Server",			
	"eGSM_Write_Socket_Phase1",		/*20*/	
	"eGSM_Write_Socket_Phase2",		
	"eGSM_Read_Socket",
	"eGSM_Add_MessageHead",
	"eGSM_Boot_Delay",
};

void GPRS_Exce(void)
{
	if(g_sGPRS.nNowState != eGSM_Idle)
	{
		sGPRSSendCmdQ *p;
		p = Get_Last_Valid_SendCmdQ();
		if(p)
		{
			g_sGPRS.nNowState = p->nNowState;
			if(Get_System_Time() > p->nStartWaitTime + p->nWaitAckTimeout)
			{
				if(p->nRetryCnt)
				{
					p->nRetryCnt--;
					p->nStartWaitTime = Get_System_Time();
					if(p->bDefaultProcessTimeout)
					{
						p->bIsCmdBusy = true;
						GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
					}
					else
					{
						GPRS_FSM_Sub(eGPRSCmdTimeout);
					}
				}
				else
				{
					if(p->bDefaultProcessExceedMaxRetryCnt)
					{
						TRACE_PRINTF("AT Cmd timeout, cmd state = %s.\r\n", GPRS_Cmd_StateString[g_sGPRS.nNowState]);
						GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
					}
					else
					{
						GPRS_FSM_Sub(eGPRSExceedMaxRetryCnt);
					}
				}
			}
			else
			{
				GPRS_FSM_Sub(eGPRSNormalRoll);
			}
		}

		GPRS_Processed_Receive_Data();

		if(g_sGPRS.nNewState != eGSM_None)
		{
			g_sGPRS.nNowState = g_sGPRS.nNewState;
			g_sGPRS.nNewState = eGSM_None;
			GPRS_FSM_Sub(eGPRSChangeState);
		}
	}
}

#elif USE_SIM800C

#define GPRS_POWER_ON()  			
#define GPRS_POWER_OFF()  				

#define GPRS_MODULE_POWER_ON()  	GPIOC->BSRR=GPIO_Pin_12
#define GPRS_MODULE_POWER_OFF()  	GPIOC->BRR=GPIO_Pin_12

#define GPRS_POWER_KEY_PIN_HIGH()	GPIOC->BSRR=GPIO_Pin_15
#define GPRS_POWER_KEY_PIN_LOW()	GPIOC->BRR=GPIO_Pin_15

#define GPRS_RESET_HIGH()			
#define GPRS_RESET_LOW()			


static void GPRS_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*GSM Power-EN*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPRS_MODULE_POWER_OFF();

	/*POWER-KEY-PIN*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPRS_POWER_KEY_PIN_LOW();
}

static void GPRS_Uart_Init(unsigned long nBaudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* Configure USARTx_Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure USARTx_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure the USARTx */ 
	USART_Init(UART4, &USART_InitStructure);

	//串口接收中断开启
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); 

	/* Enable the USARTx */
	USART_Cmd(UART4, ENABLE);

	USART_ClearFlag(UART4, USART_FLAG_TC);
}


void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		GPRS_Receive_Data(USART_ReceiveData(UART4));
	}
}

void GPRS_Transmit_Data(char  *pData, unsigned short nLen)
{
	int i;
	if (nLen)
	{
		for(i = 0; i < nLen; i++)
		{
			USART_SendData(UART4, (unsigned char)pData[i]);
			while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
		}
	}
}

/*硬件相关部分--结束*************************************************************************/

#define GPRS_RECEIVE_Q_SIZE		10
sGPRSReceiveQ g_sGPRSReceiveQ[GPRS_RECEIVE_Q_SIZE];
static sGPRSReceiveQ *pCurrGPRSRevQ;
#define GPRS_SEND_Q_SIZE			3
sGPRSSendCmdQ g_sGPRSSendCmdQ[GPRS_SEND_Q_SIZE];

#define GPRS_REV_BUFFER_SIZE		1000
#define GPRS_PROCESS_BUFFER_SIZE	200
static volatile unsigned short s_nUartInCnt = 0, s_nUartOutCnt = 0;
static char GPRS_Rx_Buffer[GPRS_REV_BUFFER_SIZE];
static char GPRS_Process_Buffer[GPRS_PROCESS_BUFFER_SIZE];
static char *pGPRSProcessPtr = NULL;

static void Init_ReceiveQ(void)
{
	int i;
	pCurrGPRSRevQ = NULL;
	for(i = 0; i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		memset(&g_sGPRSReceiveQ[i],0,sizeof(sGPRSReceiveQ));
	}
}

static sGPRSReceiveQ * Get_Free_ReceiveQ(void)
{
	int i;
	for(i = 0;i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		if(g_sGPRSReceiveQ[i].bInvalidBuffer == false)
		{
			g_sGPRSReceiveQ[i].bInvalidBuffer = true;
			g_sGPRSReceiveQ[i].bFinishReceive = false;
			g_sGPRSReceiveQ[i].nStartPosition = s_nUartInCnt;
			return &g_sGPRSReceiveQ[i];
		}
	}
	return NULL;
}

static void Init_SendCmdQ(void)
{
	int i;
	for(i = 0; i < GPRS_SEND_Q_SIZE; i++)
	{
		memset(&g_sGPRSSendCmdQ[i],0,sizeof(sGPRSSendCmdQ));
	}
}

static sGPRSSendCmdQ * Get_Free_SendCmdQ(void)
{
	int i;
	for(i = 0;i < GPRS_SEND_Q_SIZE; i++)
	{
		if(g_sGPRSSendCmdQ[i].bInvalidBuffer == false)
		{
			g_sGPRSSendCmdQ[i].nRetryCnt = 10;
			g_sGPRSSendCmdQ[i].pSendCmd = NULL;
			g_sGPRSSendCmdQ[i].bIsCmdBusy = true;
			g_sGPRSSendCmdQ[i].nCmdLength = 0;
			g_sGPRSSendCmdQ[i].nStartWaitTime = Get_System_Time();
			g_sGPRSSendCmdQ[i].nWaitAckTimeout = 200;
			g_sGPRSSendCmdQ[i].bDefaultProcessTimeout = true;
			g_sGPRSSendCmdQ[i].bDefaultProcessExceedMaxRetryCnt = true;
			g_sGPRSSendCmdQ[i].bInvalidBuffer = true;
			return &g_sGPRSSendCmdQ[i];
		}
	}
	return NULL;
}

static sGPRSSendCmdQ * Get_Last_Valid_SendCmdQ(void)
{
	int i;
	sGPRSSendCmdQ *p = NULL;
	for(i = 0;i < GPRS_SEND_Q_SIZE; i++)
	{
		if(g_sGPRSSendCmdQ[i].bInvalidBuffer)
		{
			p = &g_sGPRSSendCmdQ[i];
		}
		else
		{
			break;
		}
	}
	return p;
}

/*外部调用的函数************************************************************************************/

/*此函数要被定时器中断函数调用*/
void Mark_Valid_Package_Via_Timeout(void)
{
	if(g_sGPRS.bStartReceiveData)
	{
		if(Get_System_Time() > s_nStartReceiveTime + 4)
		{	
			g_sGPRS.bStartReceiveData = false;
			if(pCurrGPRSRevQ)
			{
				pCurrGPRSRevQ->bFinishReceive = true;
				pCurrGPRSRevQ->nEndPosition = s_nUartInCnt;
			}
		}
	}
}

bool Get_Gprs_Tcp_Connect_State(void)
{
	return g_sGPRS.bTcpConnectState;
}

bool GPRS_Power_Control(ePowerCtrlParameter eArgument, const char *FileName, int nLineNumber)
{
	TRACE_PRINTF("GPRS module  is power down.(Argument = %d (0:Normal, 1:Always, 2:Restart), Call by \" %s \" ,Line = %d)\r\n",eArgument, FileName ,nLineNumber);
	
	GPRS_MODULE_POWER_OFF();
	g_nGprsSingalQuality = -1;
	g_bGprsPowerOnState = false;
	Gprs_Set_UserLogin_State(false);
	GprsHal_Switch_State(eGSM_Idle);

	switch(eArgument)
	{
		case eGPRSPowerDownNormal:
			s_nGprsRestartCnt = 0;
			GprsUser_Switch_State(eGprsUserGetIMEI);
			return true;

		case eGPRSPowerDownAlways:
			s_nGprsRestartCnt = GPRS_MAX_RESTART_TIME;
			GprsUser_Switch_State(eGprsUserNothing);
			return false;

		case eGPRSPowerRestart:
			if(++s_nGprsRestartCnt < GPRS_MAX_RESTART_TIME)
			{
				GPRS_Hal_Var_Init();
				TRACE_PRINTF("s_nGprsRestartCnt = %d!\r\n",s_nGprsRestartCnt);
				GprsUser_Switch_State(eGprsUserGetIMEI);
				return true;
			}
			GprsUser_Switch_State(eGprsUserNothing);
			return false;
	}
	
	return false;
}

/*外部调用的函数************************************************************************************/


char* Get_APN_String(unsigned short nMCC, unsigned short nMNC)
{
	typedef struct
	{
		unsigned short MCC;
		unsigned short MNC;
		char	 APN_String[50];
	} APN_Lookup_Struct;

	const APN_Lookup_Struct ARRAY_APN_Lookup_Table[] =
	{
		//China
		460,		0,	"CMNET",
		460,		1,	"3GNET",

		//Russia
		250,		1,	"internet.mts.ru,mts,mts",
		250,		2,	"internet",
		250,		99,	"internet.beeline.ru,beeline,beeline",
		250,    	20, 	"internet.tele2.ru",

		//Kenya
		639,		2,	"safaricom,saf,data",

		//INDIA
		404,   	29,  	"aircelgprs",

		//Australia
		505,   	01,  	"telstra.internet"
	};

	unsigned short i;
	APN_Lookup_Struct *p;

	p=(APN_Lookup_Struct *)ARRAY_APN_Lookup_Table;
	
	for(i=0;i<ARR_SIZE(ARRAY_APN_Lookup_Table);i++)
	{
		if(p ->MCC ==nMCC && p->MNC ==nMNC)
		{
			return p->APN_String;
		}
		p++;
	}

	return NULL;
}

static void GPRS_Hal_Var_Init(void)
{
	Init_SendCmdQ();
	Init_ReceiveQ();
	s_nUartInCnt = 0;
	s_nUartOutCnt = 0;
	memset(&g_sGPRS, 0, sizeof(g_sGPRS));
	g_nGprsSingalQuality = -1;
	g_bGprsPowerOnState = true;
	GprsHal_Switch_State(eGSM_PowerUp);
}

void GPRS_Receive_Data(unsigned char nData)
{
	if(!g_sGPRS.bStartReceiveData)
	{
		g_sGPRS.bStartReceiveData = true;
		pCurrGPRSRevQ = Get_Free_ReceiveQ();
	}
	
	GPRS_Rx_Buffer[s_nUartInCnt] = nData;
	s_nStartReceiveTime = Get_System_Time();
	if(++s_nUartInCnt >= GPRS_REV_BUFFER_SIZE)
	{
		s_nUartInCnt = 0;
	}
}

static unsigned char GPRS_Get_Uart_Data(void)
{
	unsigned char nTemp;
	nTemp = GPRS_Rx_Buffer[s_nUartOutCnt];
	if(++s_nUartOutCnt >= GPRS_REV_BUFFER_SIZE)
	{
		s_nUartOutCnt = 0;
	}
	return nTemp;
}

void GprsHal_Switch_State(eGSM_State eNewState)
{
	g_sGPRS.nNewState = eNewState;
}

static void  GPRS_Power_Up(eGPRSMessage eMessageType)
{
	static sGPRSSendCmdQ *p = NULL;
	
	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_PowerUp;
				p->nWaitAckTimeout = 5000;
				p->bDefaultProcessTimeout = false;
				
				GPRS_Gpio_Init();
				GPRS_Uart_Init(115200);
				GPRS_MODULE_POWER_ON();
				GPRS_RESET_LOW();
				delay_ms(10);
				GPRS_RESET_HIGH();
				GPRS_POWER_KEY_PIN_HIGH();
				TRACE_PRINTF("GPRS module  is booting....\r\n");
			}
			break;
			
		case eGPRSCmdTimeout:
			p->bInvalidBuffer = false;
			GPRS_POWER_KEY_PIN_LOW();
			GprsHal_Switch_State(eGSM_UartSync);
			break;
	}
}

static void GPRS_UartSync(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_UartSync;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT\r\r\nOK\r\n");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_ChangeBaudRate);
				TRACE_PRINTF("GPRS-->Hardware is Ok.\r\n");
			}
			break;
	}
}

static void GPRS_Change_Baudrate(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+IPR=115200\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_ChangeBaudRate;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+IPR=115200\r\r\nOK\r\n");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GPRS_Uart_Init(115200);
				GprsHal_Switch_State(eGSM_Enable_Echo);
				TRACE_PRINTF("GPRS-->Change new baudrate = 115200.\r\n");
			}
			break;
	}
}



static void GPRS_Enable_Echo(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "ATE1\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Enable_Echo;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"ATE1\r\r\nOK\r\n");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Get_IMEI);
				TRACE_PRINTF("GPRS-->Enable Echo.\r\n");
			}
			break;
	}
}


static void GPRS_Cmd_Get_IMEI(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CGSN\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Get_IMEI;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+CGSN\r\r\n");
			if(pStr)
			{
				if(strlen(pGPRSProcessPtr) > 20)
				{
					pStr = strstr(pStr,"\r\n");
					Save_Hardware_IMEI(pStr + 2);
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_SimCard_Check);
					TRACE_PRINTF("GPRS-->IMEI = %s.\r\n",Get_Hardware_IMEI());
				}
			}
			break;
	}
}


unsigned char GPRS_Get_Signal_Quality(unsigned char nValue)
{
	if(nValue < 4 || nValue == 99)
	{
		return 0;
	}
	else if(nValue >= 4 &&  nValue < 10)
	{
		return 1;
	}
	else if(nValue >= 10 &&  nValue < 16)
	{
		return 2;
	}
	else if(nValue >= 16 &&  nValue < 22)
	{
		return 3;
	}
	else if(nValue >= 22 &&  nValue < 28)
	{
		return 4;
	}
	else if(nValue >= 28)
	{
		return 5;
	}

	return 0;
}

static void GPRS_Cmd_CheckSimcard(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CPIN?\r\n";
	static sGPRSSendCmdQ *p = NULL;
	static char nResultErrorCnt;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_SimCard_Check;
				p->nRetryCnt = 20;
				p->nWaitAckTimeout = 500;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				nResultErrorCnt = 0;
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+CPIN?\r");
			if(pStr)
			{
				if(strstr(pStr,"+CPIN: READY"))
				{
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_Add_MessageHead);
					TRACE_PRINTF("GPRS-->Sim Card Ready!\r\n");
				}
				else if(strstr(pStr,"ERROR"))
				{
					TRACE_PRINTF("GPRS-->SIMCard not inserted(Can't detected the valid Simcard)!\r\n");
					if(++nResultErrorCnt > 15)
					{
						GPRS_Power_Control(eGPRSPowerDownAlways, __FILE__, __LINE__);
						Notify_User_Invalid_Simcard();
					}
				}
			}
			break;
	}
}

static void GPRS_Cmd_Add_MessageHead(eGPRSMessage eMessageType)
{
	const char *pCmdStr = "AT+CIPHEAD=1\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Add_MessageHead;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"AT+CIPHEAD=1\r\r\nOK\r\n"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_GET_CSQ);
				TRACE_PRINTF("GPRS-->Add server message header is Ok.\r\n");
			}
			break;
	}
}

static void GPRS_Cmd_Get_CSQ(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CSQ\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_GET_CSQ;
				p->nRetryCnt = 20;
				p->nWaitAckTimeout = 1000;
				if(g_sGPRS.bTcpConnectState)
				{
					p->nWaitAckTimeout = 2000;
				}
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"+CSQ:");
			if(pStr)
			{
				int nValue;
				nValue = atoi(pStr + 6);
				if(nValue)
				{
					p->bIsCmdBusy = false;
					g_nGprsSingalQuality = GPRS_Get_Signal_Quality(nValue);
					TRACE_PRINTF("GPRS-->g_nGprsSingalQuality = %d (+CSQ:%d).\r\n",g_nGprsSingalQuality, nValue);
					if(g_sGPRS.bTcpConnectState)
					{
						p->nRetryCnt = 20;
					}
					else
					{
						p->bInvalidBuffer = false;
						GprsHal_Switch_State(eGSM_NetRegist_Check);
					}
				}
			}
			break;

		case eGPRSNormalRoll:
			if(p->bIsCmdBusy) return;

			if(g_sGPRS.bTcpConnectState)
			{
				if(g_nGprs_Tx_Cnt)
				{
					GprsHal_Switch_State(eGSM_Write_Socket_Phase1);
				}
			}
			break;
	}
}


void GPRS_Cmd_GET_MCC_MNC(bool bUnprocessMessage)
{
	char *pStr = "AT+CPSI?\r\n";
	GPRS_Transmit_Data(pStr,strlen(pStr));
}

static void GPRS_Cmd_CheckNetRegist_State(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	const char *pCmdStr = "AT+CGATT?\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_NetRegist_Check;
				p->nRetryCnt = 80;
				p->nWaitAckTimeout = 1000;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"AT+CGATT?"))
			{
				pStr = strstr(pGPRSProcessPtr,"+CGATT:");
				if(pStr)
				{
					TRACE_PRINTF("GPRS-->NetRegist state(%c).\r\n", *(pStr + 8));
					if(*(pStr + 8) == '1')
					{
						p->bInvalidBuffer = false;
						GprsHal_Switch_State(eGSM_Connect_Server);
					}
				}
			}
			break;
	}
}

static void GPRS_Cmd_Connect_Server(eGPRSMessage eMessageType)
{
	static char aCmdStr[80];
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Connect_Server;
				p->nRetryCnt = 2;
				p->nWaitAckTimeout = 15000;
				sprintf(aCmdStr,"AT+CIPSTART=\"TCP\",%s,\"%d\"\r\n",
						Get_Net_AccessMode() ? Get_Net_RemoteURL() : Get_Net_RemoteIP(), Get_Net_RemotePort());
				p->pSendCmd = aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				TRACE_PRINTF("Connect server Cmd string = %s",aCmdStr);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"CONNECT OK") || strstr(pGPRSProcessPtr,"ALREADY CONNECT"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Disable_Echo);
				g_sGPRS.bTcpConnectState = true;
				TRACE_PRINTF("GPRS-->Connect server successful!\r\n");
			}
			else if(strstr(pGPRSProcessPtr,"+CIPOPEN: 0") || strstr(pGPRSProcessPtr,"ERROR") )
			{
				GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
			}
			break;
	}
}


static void GPRS_Disable_Echo(eGPRSMessage eMessageType)
{
	const char *pCmdStr = "ATE0\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Disable_Echo;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"ATE0\r\r\nOK\r\n"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_GET_CSQ);
				TRACE_PRINTF("GPRS-->Disable Echo!\r\n");
			}
			break;
	}
}


static void GPRS_Cmd_Write_Socket_Phase2(eGPRSMessage eMessageType)
{
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Write_Socket_Phase2;
				p->nRetryCnt = 2;
				p->nWaitAckTimeout = 15000;
				p->pSendCmd = GPRS_Tx_Buffer;
				p->nCmdLength = g_nGprs_Tx_Cnt;
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"SEND OK\r\n"))
			{
				g_nGprs_Tx_Cnt = 0;
				p->bInvalidBuffer = false;
				TRACE_PRINTF("Write socket data result-->OK\r\n");
			}
			else if(strstr(pGPRSProcessPtr,"SEND FAIL:"))
			{
				TRACE_PRINTF("Write socket data result-->FAIL\r\n");
				GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
			}
			break;
	}
}

static void GPRS_Cmd_Write_Socket_Phase1(eGPRSMessage eMessageType)
{
	char *pStr;
	static char aCmdStr[50];
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Write_Socket_Phase1;
				p->nRetryCnt = 2;
				sprintf(aCmdStr,"AT+CIPSEND=%d\r\n",g_nGprs_Tx_Cnt);
				p->pSendCmd = aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				TRACE_PRINTF("Start write socket data.\r\n");
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,">");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GPRS_Transmit_Data(GPRS_Tx_Buffer,g_nGprs_Tx_Cnt);
				g_nGprs_Tx_Cnt = 0;
			}
			break;
	}
}


/************************************************************************************************************************/
bool GPRS_Processed_Receive_Data(void)
{
	int i;
	char * pStr;
	sGPRSReceiveQ *pPrcoQ;
	static unsigned short nLen = 0;
	for(i = 0; i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		if(g_sGPRSReceiveQ[i].bFinishReceive)
		{
			pPrcoQ = &g_sGPRSReceiveQ[i];
			g_sGPRSReceiveQ[i].bFinishReceive = false;
			goto _Next;
		}
	}
	return false;
	
_Next:
	/*求当前包的长度*/
	if(pPrcoQ->nEndPosition >= pPrcoQ->nStartPosition)
	{
		nLen = pPrcoQ->nEndPosition - pPrcoQ->nStartPosition;
	}
	else
	{
		nLen = pPrcoQ->nEndPosition + GPRS_REV_BUFFER_SIZE - pPrcoQ->nStartPosition;
	}
	
	if(nLen > GPRS_PROCESS_BUFFER_SIZE)
	{
		TRACE_PRINTF("GPRS-->Not enough buffer to process this message!\r\n");
		return false;
	}

	/*复制数据到处理的内存.*/
	s_nUartOutCnt = pPrcoQ->nStartPosition;
	memset(GPRS_Process_Buffer,0,sizeof(GPRS_Process_Buffer));
	for(i = 0;i < nLen; i++)
	{
		if(i >= GPRS_PROCESS_BUFFER_SIZE)
		{	
			break;
		}
		GPRS_Process_Buffer[i] = GPRS_Get_Uart_Data();
	}
	pPrcoQ->bInvalidBuffer = false;

	pGPRSProcessPtr = GPRS_Process_Buffer;
#if GPRS_FULL_ECHO_MESSAGE
	TRACE_PRINTF("++Gprs_Receive(Bytes=%02d)>\r\n%s\r\n",nLen,pGPRSProcessPtr);
#endif

	do
	{
		pStr = strstr(pGPRSProcessPtr,"\r\n");
		if(pStr)
		{
			unsigned short nLineLen;
			nLineLen =  pStr - pGPRSProcessPtr + 2;
			if(nLineLen == 2)
			{
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+PDP DEACT"))
			{
				if(g_bGprsPowerOnState)
				{
					GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
					TRACE_PRINTF("GPRS-->Tcp ConnectState is closed by remote server!\r\n");
				}
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"RING"))
			{
				/*直接挂电话,拒接任何电话*/
				GPRS_Transmit_Data("ATH\r\n",9);
				TRACE_PRINTF("GPRS-->Someone call in, auto hung up!\r\n");
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+CMTI: \"SM\""))
			{
				GPRS_Transmit_Data("AT+CMGD=1,4\r\n",9);
				TRACE_PRINTF("GPRS-->Receice  SMS message, auto delete!\r\n");
				pGPRSProcessPtr += nLineLen;
			}
			else
			{
				GPRS_FSM_Sub(eGPRSATData);
				pGPRSProcessPtr += nLineLen;
			}
		}
		else
		{
			if(strstr(pGPRSProcessPtr,"+IPD,"))
			{
				char *pStrTemp;
				int nServerDataLen;
				pStrTemp = strstr(pGPRSProcessPtr,"+IPD,");
				nServerDataLen = atoi(pStrTemp + 5);
				pStrTemp = strstr(pStrTemp,":");
				Gprs_Analyse_Server_Package_Data(pStrTemp + 1, nServerDataLen);
				pGPRSProcessPtr = pStrTemp + 2 + nServerDataLen;
			}
			else
			{
				GPRS_FSM_Sub(eGPRSATData);
				pGPRSProcessPtr += nLen;
			}
		}
	}while((pGPRSProcessPtr - GPRS_Process_Buffer) <  nLen);
	
	return true;
}



void GPRS_FSM_Sub(eGPRSMessage eMessageType)
{
	switch(g_sGPRS.nNowState)
	{
		case eGSM_Idle:
			break;
			
		case eGSM_PowerUp:
			GPRS_Power_Up(eMessageType);
			break;

		case eGSM_UartSync:
			GPRS_UartSync(eMessageType);
			break;

		case eGSM_ChangeBaudRate:
			GPRS_Change_Baudrate(eMessageType);
			break;

		case eGSM_Enable_Echo:
			GPRS_Enable_Echo(eMessageType);
			break;

		case eGSM_Get_IMEI:
			GPRS_Cmd_Get_IMEI(eMessageType);
			break;
			
		case eGSM_SimCard_Check:
			GPRS_Cmd_CheckSimcard(eMessageType);
			break;

		case eGSM_Add_MessageHead:
			GPRS_Cmd_Add_MessageHead(eMessageType);
			break;

		case eGSM_GET_CSQ:
			GPRS_Cmd_Get_CSQ(eMessageType);
			break;

		case eGSM_NetRegist_Check:
			GPRS_Cmd_CheckNetRegist_State(eMessageType);
			break;

		case eGSM_Connect_Server:
			GPRS_Cmd_Connect_Server(eMessageType);
			break;

		case eGSM_Disable_Echo:
			GPRS_Disable_Echo(eMessageType);
			break;

		case eGSM_Write_Socket_Phase1:
			GPRS_Cmd_Write_Socket_Phase1(eMessageType);
			break;

		case eGSM_Write_Socket_Phase2:
			GPRS_Cmd_Write_Socket_Phase2(eMessageType);
			break;
	}
}

const char * GPRS_Cmd_StateString[] =
{
	"eGSM_None",						/*0*/
	"eGSM_Idle",
	"eGSM_PowerOff",
	"eGSM_PowerOffDelay",
	"eGSM_PowerUp",					
	"eGSM_UartSync",					/*5*/
	"eGSM_ChangeBaudRate",
	"eGSM_Enable_Echo",				
	"eGSM_Disable_Echo",				
	"eGSM_SimCard_Check",			
	"eGSM_Get_IMEI",					/*10*/	
	"eGSM_GET_CSQ",
	"eGSM_CheckCOPS",
	"eGSM_NetRegist_Check",			
	"eGSM_Get_MCC_MNC",				
	"eGSM_Set_APN",					/*15*/		
	"eGSM_Socket_SET",				
	"eGSM_Open_NET",				
	"eGSM_Domain_Name_Resolve",	
	"eGSM_Connect_Server",			
	"eGSM_Write_Socket_Phase1",		/*20*/	
	"eGSM_Write_Socket_Phase2",		
	"eGSM_Read_Socket",
	"eGSM_Add_MessageHead",
	"eGSM_Boot_Delay",
};

void GPRS_Exce(void)
{
	if(g_sGPRS.nNowState != eGSM_Idle)
	{
		sGPRSSendCmdQ *p;
		p = Get_Last_Valid_SendCmdQ();
		if(p)
		{
			g_sGPRS.nNowState = p->nNowState;
			if(Get_System_Time() > p->nStartWaitTime + p->nWaitAckTimeout)
			{
				if(p->nRetryCnt)
				{
					p->nRetryCnt--;
					p->nStartWaitTime = Get_System_Time();
					if(p->bDefaultProcessTimeout)
					{
						p->bIsCmdBusy = true;
						GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
					}
					else
					{
						GPRS_FSM_Sub(eGPRSCmdTimeout);
					}
				}
				else
				{
					if(p->bDefaultProcessExceedMaxRetryCnt)
					{
						TRACE_PRINTF("AT Cmd timeout, cmd state = %s.\r\n", GPRS_Cmd_StateString[g_sGPRS.nNowState]);
						GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
					}
					else
					{
						GPRS_FSM_Sub(eGPRSExceedMaxRetryCnt);
					}
				}
			}
			else
			{
				GPRS_FSM_Sub(eGPRSNormalRoll);
			}
		}

		GPRS_Processed_Receive_Data();

		if(g_sGPRS.nNewState != eGSM_None)
		{
			g_sGPRS.nNowState = g_sGPRS.nNewState;
			g_sGPRS.nNewState = eGSM_None;
			GPRS_FSM_Sub(eGPRSChangeState);
		}
	}
}


#elif USE_4G

unsigned short s_MCC,s_MNC;
bool g_bGprsUseNewImagePort = false; 

#define GPRS_POWER_ON()  			
#define GPRS_POWER_OFF()  				

#define GPRS_MODULE_POWER_ON()  	GPIOC->BSRR=GPIO_Pin_12
#define GPRS_MODULE_POWER_OFF()  	GPIOC->BRR=GPIO_Pin_12

#define GPRS_POWER_KEY_PIN_HIGH()	GPIOC->BSRR=GPIO_Pin_15
#define GPRS_POWER_KEY_PIN_LOW()	GPIOC->BRR=GPIO_Pin_15

#define GPRS_RESET_HIGH()			GPIOC->BSRR=GPIO_Pin_14
#define GPRS_RESET_LOW()			GPIOC->BRR=GPIO_Pin_14


static void GPRS_Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*GSM Power-EN*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPRS_MODULE_POWER_OFF();

	/*POWER-KEY-PIN*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPRS_POWER_KEY_PIN_LOW();

	/*RESET KEY*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPRS_RESET_HIGH();
}

static void GPRS_Uart_Init(unsigned long nBaudrate)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* Configure USARTx_Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Configure USARTx_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure the USARTx */ 
	USART_Init(UART4, &USART_InitStructure);

	//串口接收中断开启
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE); 

	/* Enable the USARTx */
	USART_Cmd(UART4, ENABLE);

	USART_ClearFlag(UART4, USART_FLAG_TC);

}


void UART4_IRQHandler(void)
{
	if(USART_GetITStatus(UART4, USART_IT_RXNE) == SET)
	{
		GPRS_Receive_Data(USART_ReceiveData(UART4));
	}
}

void GPRS_Transmit_Data(char  *pData, unsigned short nLen)
{
	int i;
	if (nLen)
	{
		for(i = 0; i < nLen; i++)
		{
			USART_SendData(UART4, (unsigned char)pData[i]);
			while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
		}
	}
}

/*硬件相关部分--结束*************************************************************************/

#define GPRS_RECEIVE_Q_SIZE		10
sGPRSReceiveQ g_sGPRSReceiveQ[GPRS_RECEIVE_Q_SIZE];
static sGPRSReceiveQ *pCurrGPRSRevQ;
#define GPRS_SEND_Q_SIZE			3
sGPRSSendCmdQ g_sGPRSSendCmdQ[GPRS_SEND_Q_SIZE];

#define GPRS_REV_BUFFER_SIZE		1000
#define GPRS_PROCESS_BUFFER_SIZE	1000//200  Modified by lxl 20180814
static volatile unsigned short s_nUartInCnt = 0, s_nUartOutCnt = 0;
static char GPRS_Rx_Buffer[GPRS_REV_BUFFER_SIZE];
static char GPRS_Process_Buffer[GPRS_PROCESS_BUFFER_SIZE];
static char *pGPRSProcessPtr = NULL;

static void Init_ReceiveQ(void)
{
	int i;
	pCurrGPRSRevQ = NULL;
	for(i = 0; i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		memset(&g_sGPRSReceiveQ[i],0,sizeof(sGPRSReceiveQ));
	}
}

static sGPRSReceiveQ * Get_Free_ReceiveQ(void)
{
	int i;
	for(i = 0;i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		if(g_sGPRSReceiveQ[i].bInvalidBuffer == false)
		{
			g_sGPRSReceiveQ[i].bInvalidBuffer = true;
			g_sGPRSReceiveQ[i].bFinishReceive = false;
			g_sGPRSReceiveQ[i].nStartPosition = s_nUartInCnt;
			return &g_sGPRSReceiveQ[i];
		}
	}
	return NULL;
}

static void Init_SendCmdQ(void)
{
	int i;
	for(i = 0; i < GPRS_SEND_Q_SIZE; i++)
	{
		memset(&g_sGPRSSendCmdQ[i],0,sizeof(sGPRSSendCmdQ));
	}
}

static sGPRSSendCmdQ * Get_Free_SendCmdQ(void)
{
	int i;
	for(i = 0;i < GPRS_SEND_Q_SIZE; i++)
	{
		if(g_sGPRSSendCmdQ[i].bInvalidBuffer == false)
		{
			g_sGPRSSendCmdQ[i].nRetryCnt = 10;
			g_sGPRSSendCmdQ[i].pSendCmd = NULL;
			g_sGPRSSendCmdQ[i].bIsCmdBusy = true;
			g_sGPRSSendCmdQ[i].nCmdLength = 0;
			g_sGPRSSendCmdQ[i].nStartWaitTime = Get_System_Time();
			g_sGPRSSendCmdQ[i].nWaitAckTimeout = 200;
			g_sGPRSSendCmdQ[i].bDefaultProcessTimeout = true;
			g_sGPRSSendCmdQ[i].bDefaultProcessExceedMaxRetryCnt = true;
			g_sGPRSSendCmdQ[i].bInvalidBuffer = true;
			return &g_sGPRSSendCmdQ[i];
		}
	}
	return NULL;
}

static sGPRSSendCmdQ * Get_Last_Valid_SendCmdQ(void)
{
	int i;
	sGPRSSendCmdQ *p = NULL;
	for(i = 0;i < GPRS_SEND_Q_SIZE; i++)
	{
		if(g_sGPRSSendCmdQ[i].bInvalidBuffer)
		{
			p = &g_sGPRSSendCmdQ[i];
		}
		else
		{
			break;
		}
	}
	return p;
}

/*外部调用的函数************************************************************************************/

/*此函数要被定时器中断函数调用*/
void Mark_Valid_Package_Via_Timeout(void)
{
	if(g_sGPRS.bStartReceiveData)
	{
		if(Get_System_Time() > s_nStartReceiveTime + 4)
		{	
			g_sGPRS.bStartReceiveData = false;
			if(pCurrGPRSRevQ)
			{
				pCurrGPRSRevQ->bFinishReceive = true;
				pCurrGPRSRevQ->nEndPosition = s_nUartInCnt;
			}
		}
	}
}

bool Get_Gprs_Tcp_Connect_State(void)
{
	return g_sGPRS.bTcpConnectState;
}

bool GPRS_Power_Control(ePowerCtrlParameter eArgument, const char *FileName, int nLineNumber)
{
	TRACE_PRINTF("GPRS module  is power down.(Argument = %d (0:Normal, 1:Always, 2:Restart), Call by \" %s \" ,Line = %d)\r\n",eArgument, FileName ,nLineNumber);
	
	GPRS_MODULE_POWER_OFF();
	g_nGprsSingalQuality = -1;
	g_bGprsPowerOnState = false;
	Gprs_Set_UserLogin_State(false);
	GprsHal_Switch_State(eGSM_Idle);

	switch(eArgument)
	{
		case eGPRSPowerDownNormal:
			s_nGprsRestartCnt = 0;
			GprsUser_Switch_State(eGprsUserGetIMEI);
			return true;

		case eGPRSPowerDownAlways:
			s_nGprsRestartCnt = GPRS_MAX_RESTART_TIME;
			GprsUser_Switch_State(eGprsUserNothing);
			return false;

		case eGPRSPowerRestart:
		#if SERVER_USE_XING_JIKONG_GU
		if(1)//if(++s_nGprsRestartCnt < GPRS_MAX_RESTART_TIME) //Modified bylxl 
		#else
		if(++s_nGprsRestartCnt < GPRS_MAX_RESTART_TIME)
		#endif
			{
				GPRS_Hal_Var_Init();
				TRACE_PRINTF("s_nGprsRestartCnt = %d!\r\n",s_nGprsRestartCnt);
				GprsUser_Switch_State(eGprsUserGetIMEI);
				return true;
			}
			GprsUser_Switch_State(eGprsUserNothing);
			return false;
	}
	
	return false;
}

/*外部调用的函数************************************************************************************/

typedef struct
	{
		unsigned short MCC;
		unsigned short MNC;
		char	 APN_String[50];
	} APN_Lookup_Struct;

const APN_Lookup_Struct ARRAY_APN_Lookup_Table[] =
{
		//China
	460,		0,	"CMNET",
	460,		1,	"3GNET",
	460,		3,	"CTNET",
	460,		11,	"CTNET",

	//Russia
	250,		1,	"internet.mts.ru,mts,mts",
	250,		2,	"internet",
	250,		99,	"internet.beeline.ru,beeline,beeline",
	250,    	20, 	"internet.tele2.ru",

	//Kenya
	639,		2,	"safaricom,saf,data",

	//INDIA
	404,   	29,  	"aircelgprs",

	//Australia
	505,   	01,  	"telstra.internet"
};
#if SERVER_USE_XING_JIKONG_GU
char* Get_APN_String(unsigned short nMCC, unsigned short nMNC)
{
	if(strlen(g_sServerParam.aAPNString) == 0)
	{
		if(nMCC == 460 && nMNC == 0)
		{
			return "CMNET";
		}
		else if(nMCC == 460 && nMNC == 1)
		{
			return "3GNET";
		}
	}
			
	return g_sServerParam.aAPNString;
}
#else

char* Get_APN_String(unsigned short nMCC, unsigned short nMNC)
{
	

	unsigned short i;
	APN_Lookup_Struct *p;

	p=(APN_Lookup_Struct *)ARRAY_APN_Lookup_Table;
	
	for(i=0;i<ARR_SIZE(ARRAY_APN_Lookup_Table);i++)
	{
		if(p ->MCC ==nMCC && p->MNC ==nMNC)
		{
			return p->APN_String;
		}
		p++;
	}

	return NULL;
}
#endif

static void GPRS_Hal_Var_Init(void)
{
	Init_SendCmdQ();
	Init_ReceiveQ();
	s_nUartInCnt = 0;
	s_nUartOutCnt = 0;
	memset(&g_sGPRS, 0, sizeof(g_sGPRS));
	g_nGprsSingalQuality = -1;
	g_bGprsPowerOnState = true;
	GprsHal_Switch_State(eGSM_PowerUp);
	TRACE_PRINTF("GPRS_Hal_Var_Init\r\n");
}

void GPRS_Receive_Data(unsigned char nData)
{
	if(!g_sGPRS.bStartReceiveData)
	{
		g_sGPRS.bStartReceiveData = true;
		pCurrGPRSRevQ = Get_Free_ReceiveQ();
	}
	
	GPRS_Rx_Buffer[s_nUartInCnt] = nData;
	s_nStartReceiveTime = Get_System_Time();
	if(++s_nUartInCnt >= GPRS_REV_BUFFER_SIZE)
	{
		s_nUartInCnt = 0;
	}
}

static unsigned char GPRS_Get_Uart_Data(void)
{
	unsigned char nTemp;
	nTemp = GPRS_Rx_Buffer[s_nUartOutCnt];
	if(++s_nUartOutCnt >= GPRS_REV_BUFFER_SIZE)
	{
		s_nUartOutCnt = 0;
	}
	return nTemp;
}

void GprsHal_Switch_State(eGSM_State eNewState)
{
	g_sGPRS.nNewState = eNewState;
}

static void  GPRS_Power_Up(eGPRSMessage eMessageType)
{
	static sGPRSSendCmdQ *p = NULL;
	
	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_PowerUp;
				p->nWaitAckTimeout = 12000;
				p->bDefaultProcessTimeout = false;
				
				GPRS_Gpio_Init();
				GPRS_Uart_Init(115200);
				GPRS_MODULE_POWER_ON();
				GPRS_RESET_HIGH();
				delay_ms(10);
				GPRS_RESET_LOW();
				GPRS_POWER_KEY_PIN_HIGH();
				TRACE_PRINTF("GPRS module is booting....\r\n");
			}
			break;
			
		case eGPRSCmdTimeout:
			p->bInvalidBuffer = false;
			GPRS_POWER_KEY_PIN_LOW();
			GprsHal_Switch_State(eGSM_UartSync);
			break;
	}
}

static void GPRS_UartSync(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{	
				p->nWaitAckTimeout = 500;
				p->nNowState = eGSM_UartSync;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT\r\r\nOK\r\n");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_ChangeBaudRate);
				TRACE_PRINTF("GPRS-->Hardware is Ok.\r\n");
			}
			break;
	}
}

static void GPRS_Change_Baudrate(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+IPR=115200\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_ChangeBaudRate;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+IPR=115200\r\r\nOK\r\n");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GPRS_Uart_Init(115200);
				GprsHal_Switch_State(eGSM_Enable_Echo);
				TRACE_PRINTF("GPRS-->Change new baudrate = 115200.\r\n");
			}
			break;
	}
}



static void GPRS_Enable_Echo(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "ATE1\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Enable_Echo;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"ATE1\r\r\nOK\r\n");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Get_IMEI);
				TRACE_PRINTF("GPRS-->Enable Echo.\r\n");
			}
			break;
	}
}


static void GPRS_Cmd_Get_IMEI(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CGSN\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Get_IMEI;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+CGSN\r\r\n");//开机获取IMEI
			if(pStr)
			{
				if(strlen(pGPRSProcessPtr) > 20)
				{
					pStr = strstr(pStr,"\r\n");
					Save_Hardware_IMEI(pStr + 2);
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_SimCard_Check);
					TRACE_PRINTF("GPRS-->IMEI = %s.\r\n",Get_Hardware_IMEI());
				}
			}
			break;
	}
}


unsigned char GPRS_Get_Signal_Quality(unsigned char nValue)
{
	if(nValue < 4 || nValue == 99)
	{
		return 0;
	}
	else if(nValue >= 4 &&  nValue < 10)
	{
		return 1;
	}
	else if(nValue >= 10 &&  nValue < 16)
	{
		return 2;
	}
	else if(nValue >= 16 &&  nValue < 22)
	{
		return 3;
	}
	else if(nValue >= 22 &&  nValue < 28)
	{
		return 4;
	}
	else if(nValue >= 28)
	{
		return 5;
	}

	return 0;
}

static void GPRS_Cmd_CheckSimcard(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CPIN?\r\n";
	static sGPRSSendCmdQ *p = NULL;
	static char nResultErrorCnt;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_SimCard_Check;
				p->nRetryCnt = 20;
				p->nWaitAckTimeout = 500;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				nResultErrorCnt = 0;
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"AT+CPIN?\r");
			if(pStr)
			{
				if(strstr(pStr,"+CPIN: READY"))
				{
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_Add_MessageHead);
					TRACE_PRINTF("GPRS-->Sim Card Ready!\r\n");
				}
				else if(strstr(pStr,"ERROR"))
				{
					TRACE_PRINTF("GPRS-->SIMCard not inserted(Can't detected the valid Simcard)!\r\n");
					if(++nResultErrorCnt > 15)
					{
						GPRS_Power_Control(eGPRSPowerDownAlways, __FILE__, __LINE__);
						Notify_User_Invalid_Simcard();
					}
				}
			}
			break;
	}
}

static void GPRS_Cmd_Add_MessageHead(eGPRSMessage eMessageType)
{
	const char *pCmdStr = "AT+CIPHEAD=1\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Add_MessageHead;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"AT+CIPHEAD=1\r\r\nOK\r\n"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_GET_CSQ);
				TRACE_PRINTF("GPRS-->Add server message header is Ok.\r\n");
			}
			break;
	}
}

static void GPRS_Cmd_Get_CSQ(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+CSQ\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_GET_CSQ;
				p->nRetryCnt = 20;
				p->nWaitAckTimeout = 1000;
				if(g_sGPRS.bTcpConnectState)
				{
					p->nWaitAckTimeout = 2000;
				}
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"+CSQ:");
			if(pStr)
			{
				int nValue;
				nValue = atoi(pStr + 6);
				if(nValue && nValue != 99)
				{
					p->bIsCmdBusy = false;
					g_nGprsSingalQuality = GPRS_Get_Signal_Quality(nValue);
					
					#if SERVER_USE_XING_JIKONG_GU
					if(g_nGprsSingalQuality>0)//分析当前信号强度added by lxl 20180816
					{
						g_sXingJiKongGU.bSingal_Strength=true;//g_Singal_Strength=true;
					}
					else
					{
						g_sXingJiKongGU.bSingal_Strength=false;//g_Singal_Strength=false;
					}
					#endif
					TRACE_PRINTF("GPRS-->g_nGprsSingalQuality = %d (+CSQ:%d).\r\n",g_nGprsSingalQuality, nValue);
					if(g_sGPRS.bTcpConnectState)
					{
						p->nRetryCnt = 20;
					}
					else
					{
						p->bInvalidBuffer = false;
						GprsHal_Switch_State(eGSM_CheckCOPS);
					}
				}
			}
			break;

		case eGPRSNormalRoll:
			if(p->bIsCmdBusy) return;

			if(g_sGPRS.bTcpConnectState)
			{
				if(g_nGprs_Tx_Cnt)
				{
					GprsHal_Switch_State(eGSM_Write_Socket_Phase1);
				}
			}
			break;
	}
}

static void GPRS_Cmd_CheckCOPS(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	const char *pCmdStr = "AT+COPS?\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_CheckCOPS;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"+COPS:");
			if(pStr)
			{
				char *pStr1 = strstr(pStr,"\r");
				if(pStr1) *pStr1 = 0;
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Get_MCC_MNC);
				TRACE_PRINTF("GPRS-->%s.\r\n", pStr);
			}
			break;
	}
}


void GPRS_Cmd_GET_MCC_MNC(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	const char *pCmdStr = "AT+CPSI?\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Get_MCC_MNC;
				#if SERVER_USE_XING_JIKONG_GU //物联网卡不能正常连接而添加
				p->nRetryCnt=20;
				p->nWaitAckTimeout=1000;
				#endif
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"+CPSI:");
			if(pStr)
			{
				s_MCC = s_MNC = 0;
				pStr = strstr(pStr,",");
				pStr = strstr(pStr + 1,",");
				s_MCC=atoi(pStr + 1);
				s_MNC=atoi(pStr + 1 + 4);
				if(s_MCC != 0 || s_MNC != 0)
				{
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_Set_APN);
					TRACE_PRINTF("GPRS-->MCC = %d, MNC = %d!\r\n",s_MCC, s_MNC);
				}
			}
			break;
	}
}

void GPRS_Cmd_Set_APN(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	static char aCmdStr[60];
	static sGPRSSendCmdQ *p = NULL;
	sprintf(aCmdStr,"AT+CGDCONT=1,\"IP\",\"%s\"\r\n",Get_APN_String(s_MCC,s_MNC));

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Set_APN;
				p->pSendCmd = (char *)aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"OK");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_NetRegist_Check);
				TRACE_PRINTF("GPRS-->Set APN string = %s\r\n",Get_APN_String(s_MCC,s_MNC));
			}
			break;
	}
}

static void GPRS_Cmd_CheckNetRegist_State(eGPRSMessage eMessageType)
{
	char *pStr = NULL;
	const char *pCmdStr = "AT+CGATT?\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_NetRegist_Check;
				p->nRetryCnt = 80;
				p->nWaitAckTimeout = 1000;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"AT+CGATT?"))
			{
				pStr = strstr(pGPRSProcessPtr,"+CGATT:");
				if(pStr)
				{
					TRACE_PRINTF("GPRS-->NetRegist state(%c).\r\n", *(pStr + 8));
					if(*(pStr + 8) == '1')
					{
						p->bInvalidBuffer = false;
						GprsHal_Switch_State(eGSM_Open_NET);
					}
				}
			}
			break;
	}
}

static void GPRS_Cmd_Open_Net(eGPRSMessage eMessageType)
{
	char *pStr;
	const char *pCmdStr = "AT+NETOPEN\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Open_NET;
				p->nRetryCnt = 1;
				p->nWaitAckTimeout = 15000;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,"+NETOPEN:");
			if(pStr)
			{
				int nValue;
				nValue = atoi(pStr + 9);
				if(nValue == 0)
				{
					p->bInvalidBuffer = false;
					GprsHal_Switch_State(eGSM_Connect_Server);
					TRACE_PRINTF("GPRS-->Open Net successful!\r\n");
				}
				else
				{
					TRACE_PRINTF("GPRS-->Open Net failed!\r\n");
				}
			}
			break;
	}
}


static void GPRS_Cmd_Connect_Server(eGPRSMessage eMessageType)
{
	static char aCmdStr[80];
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Connect_Server;
				p->nRetryCnt = 2;
				p->nWaitAckTimeout = 15000;
				sprintf(aCmdStr,"AT+CIPOPEN=0,\"TCP\",\"%s\",%d\r\n",
						Get_Net_AccessMode() ? Get_Net_RemoteURL() : Get_Net_RemoteIP(), Get_Net_RemotePort());
				p->pSendCmd = aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"+CIPOPEN: 0,0"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Disable_Echo);
				g_sGPRS.bTcpConnectState = true;
				TRACE_PRINTF("GPRS-->Connect server successful!\r\n");
			}
			else if(strstr(pGPRSProcessPtr,"+CIPOPEN: 0") || strstr(pGPRSProcessPtr,"ERROR") )
			{
				GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
			}
			break;
	}
}



static void GPRS_Disable_Echo(eGPRSMessage eMessageType)
{
	const char *pCmdStr = "ATE0\r\n";
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Disable_Echo;
				p->pSendCmd = (char *)pCmdStr;
				p->nCmdLength = strlen(pCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"ATE0\r\r\nOK\r\n"))
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_GET_CSQ);
				TRACE_PRINTF("GPRS-->Disable Echo!\r\n");
			}
			break;
	}
}


static void GPRS_Cmd_Write_Socket_Phase2(eGPRSMessage eMessageType)
{
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Write_Socket_Phase2;
				p->nRetryCnt = 2;
				p->nWaitAckTimeout = 1000;
				p->pSendCmd = GPRS_Tx_Buffer;
				p->nCmdLength = g_nGprs_Tx_Cnt;
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
			}
			break;
			
		case eGPRSATData:
			if(strstr(pGPRSProcessPtr,"+CIPSEND:"))
			{
				g_nGprs_Tx_Cnt = 0;
				g_bGprsUseNewImagePort = false;
				p->bInvalidBuffer = false;
				g_sGPRS.nNowState = eGSM_GET_CSQ;
				TRACE_PRINTF("Write socket data result-->OK\r\n");
			}
			else if(strstr(pGPRSProcessPtr,"+CIPERROR:"))
			{
				GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
			}
			break;
	}
}

static void GPRS_Cmd_Write_Socket_Phase1(eGPRSMessage eMessageType)
{
	char *pStr;
	static char aCmdStr[50];
	static sGPRSSendCmdQ *p = NULL;

	switch(eMessageType)
	{
		case eGPRSChangeState:
			p = Get_Free_SendCmdQ();
			if(p)
			{
				p->nNowState = eGSM_Write_Socket_Phase1;
				p->nRetryCnt = 2;
				sprintf(aCmdStr,"AT+CIPSEND=%d,%d\r\n", g_bGprsUseNewImagePort ? 1 : 0, g_nGprs_Tx_Cnt);
				p->pSendCmd = aCmdStr;
				p->nCmdLength = strlen(aCmdStr);
				GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
				TRACE_PRINTF("Start write socket data.\r\n");
			}
			break;
			
		case eGPRSATData:
			pStr = strstr(pGPRSProcessPtr,">");
			if(pStr)
			{
				p->bInvalidBuffer = false;
				GprsHal_Switch_State(eGSM_Write_Socket_Phase2);
			}
			break;
	}
}



/************************************************************************************************************************/
bool GPRS_Processed_Receive_Data(void)
{
	int i;
	char * pStr;
	sGPRSReceiveQ *pPrcoQ;
	static unsigned short nLen = 0;
	for(i = 0; i < GPRS_RECEIVE_Q_SIZE; i++)
	{
		if(g_sGPRSReceiveQ[i].bFinishReceive)
		{
			pPrcoQ = &g_sGPRSReceiveQ[i];
			g_sGPRSReceiveQ[i].bFinishReceive = false;
			goto _Next;
		}
	}
	return false;
	
_Next:
	/*求当前包的长度*/
	if(pPrcoQ->nEndPosition >= pPrcoQ->nStartPosition)
	{
		nLen = pPrcoQ->nEndPosition - pPrcoQ->nStartPosition;
	}
	else
	{
		nLen = pPrcoQ->nEndPosition + GPRS_REV_BUFFER_SIZE - pPrcoQ->nStartPosition;
	}
	
	if(nLen > GPRS_PROCESS_BUFFER_SIZE)
	{
		TRACE_PRINTF("GPRS-->Not enough buffer to process this message!\r\n");
		return false;
	}

	/*复制数据到处理的内存.*/
	s_nUartOutCnt = pPrcoQ->nStartPosition;
	memset(GPRS_Process_Buffer,0,sizeof(GPRS_Process_Buffer));
	for(i = 0;i < nLen; i++)
	{
		if(i >= GPRS_PROCESS_BUFFER_SIZE)
		{	
			break;
		}
		GPRS_Process_Buffer[i] = GPRS_Get_Uart_Data();
	}
	pPrcoQ->bInvalidBuffer = false;

	pGPRSProcessPtr = GPRS_Process_Buffer;
#if GPRS_FULL_ECHO_MESSAGE
	TRACE_PRINTF("++Gprs_Receive(Bytes=%02d)>\r\n%s\r\n",nLen,pGPRSProcessPtr);
#endif

	do
	{
		pStr = strstr(pGPRSProcessPtr,"\r\n");
		if(pStr)
		{
			unsigned short nLineLen;
			nLineLen =  pStr - pGPRSProcessPtr + 2;
			if(nLineLen == 2)
			{
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+IPD") && (strstr(pGPRSProcessPtr,"+IPD") < pStr))
			{
				char *pStrTemp;
				int nServerDataLen;
				pStrTemp = strstr(pGPRSProcessPtr,"+IPD");
				nServerDataLen = atoi(pStrTemp + 4);
				pStrTemp = strstr(pStrTemp,"\r\n");
				Gprs_Analyse_Server_Package_Data(pStrTemp + 2, nServerDataLen);
				pGPRSProcessPtr = pStrTemp + 2 + nServerDataLen;
			}
			else if(strstr(pGPRSProcessPtr,"RDY") && g_sGPRS.nNewState < eGSM_UartSync)
			{
				GprsHal_Switch_State(eGSM_UartSync);
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+IPCLOSE:"))
			{
				s_nGprsRestartCnt++;
			#if SERVER_USE_XING_JIKONG_GU
				if(1)//if(s_nGprsRestartCnt < GPRS_MAX_RESTART_TIME)//Modified by lxll 20180814
			#else
				if(s_nGprsRestartCnt < GPRS_MAX_RESTART_TIME)
			#endif
				{
					GprsHal_Switch_State(eGSM_Connect_Server);
				}
				else
				{
					GPRS_Power_Control(eGPRSPowerDownAlways, __FILE__, __LINE__);
				}
				g_sGPRS.bTcpConnectState = false;
				Gprs_Set_UserLogin_State(false);
				TRACE_PRINTF("GPRS-->Tcp ConnectState is closed by remote server!\r\n");
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"RING"))
			{
				/*直接挂电话,拒接任何电话*/
				GPRS_Transmit_Data("ATH\r\n",9);
				TRACE_PRINTF("GPRS-->Someone call in, auto hung up!\r\n");
				pGPRSProcessPtr += nLineLen;
			}
			else if(strstr(pGPRSProcessPtr,"+CMTI: \"SM\""))
			{
				GPRS_Transmit_Data("AT+CMGD=1,4\r\n",9);
				TRACE_PRINTF("GPRS-->Receice  SMS message, auto delete!\r\n");
				pGPRSProcessPtr += nLineLen;
			}
			else
			{
				GPRS_FSM_Sub(eGPRSATData);
				pGPRSProcessPtr += nLineLen;
			}
		}
		else
		{
			GPRS_FSM_Sub(eGPRSATData);
			pGPRSProcessPtr += nLen;
		}
	}while((pGPRSProcessPtr - GPRS_Process_Buffer) <  nLen);
	
	return true;
}



void GPRS_FSM_Sub(eGPRSMessage eMessageType)
{
	switch(g_sGPRS.nNowState)
	{
		case eGSM_Idle:
			break;
			
		case eGSM_PowerUp:
			GPRS_Power_Up(eMessageType);
			break;

		case eGSM_UartSync:
			GPRS_UartSync(eMessageType);
			break;

		case eGSM_ChangeBaudRate:
			GPRS_Change_Baudrate(eMessageType);
			break;

		case eGSM_Enable_Echo:
			GPRS_Enable_Echo(eMessageType);
			break;

		case eGSM_Get_IMEI:
			GPRS_Cmd_Get_IMEI(eMessageType);
			break;
			
		case eGSM_SimCard_Check:
			GPRS_Cmd_CheckSimcard(eMessageType);
			break;

		case eGSM_Add_MessageHead:
			GPRS_Cmd_Add_MessageHead(eMessageType);
			break;

		case eGSM_GET_CSQ:
			GPRS_Cmd_Get_CSQ(eMessageType);
			break;

		case eGSM_CheckCOPS:
			GPRS_Cmd_CheckCOPS(eMessageType);
			break;

		case eGSM_Get_MCC_MNC:
			GPRS_Cmd_GET_MCC_MNC(eMessageType);
			break;

		case eGSM_Set_APN:
			GPRS_Cmd_Set_APN(eMessageType);
			break;

		case eGSM_NetRegist_Check:
			GPRS_Cmd_CheckNetRegist_State(eMessageType);
			break;

		case eGSM_Open_NET:
			GPRS_Cmd_Open_Net(eMessageType);
			break;
			
		case eGSM_Connect_Server:
			GPRS_Cmd_Connect_Server(eMessageType);
			break;

		case eGSM_Disable_Echo:
			GPRS_Disable_Echo(eMessageType);
			break;

		case eGSM_Write_Socket_Phase1:
			GPRS_Cmd_Write_Socket_Phase1(eMessageType);
			break;

		case eGSM_Write_Socket_Phase2:
			GPRS_Cmd_Write_Socket_Phase2(eMessageType);
			break;
	}
}

const char * GPRS_Cmd_StateString[] =
{
	"eGSM_None",						/*0*/
	"eGSM_Idle",
	"eGSM_PowerOff",
	"eGSM_PowerOffDelay",
	"eGSM_PowerUp",					
	"eGSM_UartSync",					/*5*/
	"eGSM_ChangeBaudRate",
	"eGSM_Enable_Echo",				
	"eGSM_Disable_Echo",				
	"eGSM_SimCard_Check",			
	"eGSM_Get_IMEI",					/*10*/	
	"eGSM_GET_CSQ",
	"eGSM_CheckCOPS",
	"eGSM_NetRegist_Check",			
	"eGSM_Get_MCC_MNC",				
	"eGSM_Set_APN",					/*15*/		
	"eGSM_Socket_SET",				
	"eGSM_Open_NET",				
	"eGSM_Domain_Name_Resolve",	
	"eGSM_Connect_Server",			
	"eGSM_Write_Socket_Phase1",		/*20*/	
	"eGSM_Write_Socket_Phase2",		
	"eGSM_Read_Socket",
	"eGSM_Add_MessageHead",
	"eGSM_Boot_Delay",
};

void GPRS_Exce(void)
{
       //TRACE_PRINTF("nNowState:%d\r\n",g_sGPRS.nNowState);
	if(g_sGPRS.nNowState != eGSM_Idle)
	{
		sGPRSSendCmdQ *p;
		p = Get_Last_Valid_SendCmdQ();
		if(p)
		{
			g_sGPRS.nNowState = p->nNowState;
			if(Get_System_Time() > p->nStartWaitTime + p->nWaitAckTimeout)
			{
				if(p->nRetryCnt)
				{
					p->nRetryCnt--;
					p->nStartWaitTime = Get_System_Time();
					if(p->bDefaultProcessTimeout)
					{
						p->bIsCmdBusy = true;
						GPRS_Transmit_Data(p->pSendCmd,p->nCmdLength);
					}
					else
					{
						GPRS_FSM_Sub(eGPRSCmdTimeout);
					}
				}
				else
				{
					if(p->bDefaultProcessExceedMaxRetryCnt)
					{
						TRACE_PRINTF("AT Cmd timeout, cmd state = %s.\r\n", GPRS_Cmd_StateString[g_sGPRS.nNowState]);
						GPRS_Power_Control(eGPRSPowerRestart, __FILE__, __LINE__);
					}
					else
					{
						GPRS_FSM_Sub(eGPRSExceedMaxRetryCnt);
					}
				}
			}
			else
			{
				GPRS_FSM_Sub(eGPRSNormalRoll);
			}
		}

		GPRS_Processed_Receive_Data();

		if(g_sGPRS.nNewState != eGSM_None)
		{
			g_sGPRS.nNowState = g_sGPRS.nNewState;
			g_sGPRS.nNewState = eGSM_None;
			GPRS_FSM_Sub(eGPRSChangeState);
		}
	}
}


#endif

#else

void GPRS_Exce(void) {}
void Mark_Valid_Package_Via_Timeout(void) {}

#endif

