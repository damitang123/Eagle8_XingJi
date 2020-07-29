#include "platform.h"
#include "printer.h"
#include "system_misc.h"
#include "app.h"

sPrinter g_sPrinter;

#define	IO_EX_Printer_Power_On()	GPIOE->BSRR = GPIO_Pin_2
#define	IO_EX_Printer_Power_Off()	GPIOE->BRR = GPIO_Pin_2

#define UART_BUFFER_SIZE		500
#define UART_CMD_BUFFER_SIZE	200
#define UART_CMD_TIMOUT		500	/*单位ms*/
static unsigned short s_nCmdProcessBufferCnt;
static unsigned char g_aUartCmdRevBuffer[UART_BUFFER_SIZE];
static unsigned char s_aCmdProcessBuffer[UART_CMD_BUFFER_SIZE];
static eUARTRecievePhase s_eUARTRecievePhase = eUARTPhaseHead0;
static volatile unsigned short s_nUartInCnt = 0, s_nUartOutCnt = 0;
static volatile bool s_bStartRevCmd = false;
static volatile unsigned long g_nStartRevCmdTime = 0;

/*此函数要被中断调用*/
void Receive_Data_From_UART(unsigned char RevData)
{
	g_aUartCmdRevBuffer[s_nUartInCnt] = RevData;
	if(++s_nUartInCnt >= UART_BUFFER_SIZE)
	{
		s_nUartInCnt = 0;
	}
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

void USART3_IRQHandler(void)
{
	if(USART_GetFlagStatus(USART3, USART_IT_RXNE) == SET)
	{
		Receive_Data_From_UART(USART_ReceiveData(USART3));
	}
}

void Printer_Uart_Send(unsigned char ch)
{
	USART_SendData(USART3, ch);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
}

void BlueTooth_MasterSlaveConfig(bool bMaster)
{
	Printer_Uart_Send(0x01);
	Printer_Uart_Send(0xFC);
	Printer_Uart_Send(0x0D);
	Printer_Uart_Send(0x01);
	if(bMaster)
	{
		Printer_Uart_Send(0x01);
	}
	else
	{
		Printer_Uart_Send(0x00);
	}
}

void Printer_SetPWD(char *pPWD)
{
	Printer_Uart_Send(0x01);
	Printer_Uart_Send(0xFC);
	Printer_Uart_Send(0x05);
	Printer_Uart_Send(4);

	while(*pPWD != 0)
	{
		Printer_Uart_Send(*pPWD++);
	}
}

static void Printer_Uart_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	/* Configure USARTx_Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure USARTx_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure the USARTx */ 
	USART_Init(USART3, &USART_InitStructure);

	//串口接收中断开启
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); 

	/* Enable the USARTx */
	USART_Cmd(USART3, ENABLE);

	USART_ClearFlag(USART3, USART_FLAG_TC);
}


void Printer_Init(void)
{
	Printer_Uart_Init();
	memset(&g_sPrinter, 0 ,sizeof(g_sPrinter));
	IO_EX_Printer_Power_On();
}

void Printer_Shutdown(void)
{
	IO_EX_Printer_Power_Off();
	USART_Cmd(USART3, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, DISABLE);
}

static void  Uart_Recieve_Error_Handle(int nLine)
{
	s_bStartRevCmd = false;
	s_eUARTRecievePhase = eUARTPhaseHead0;

	TRACE_PRINTF("Uart_Recieve_Error_Handle( Line = %d)\r\n",nLine);
}

static void Uart_Cmd_Parser_Sub(void)
{
	unsigned char nCmd;
	unsigned short nCmdLen;
	unsigned char nValue;
	unsigned char *p = &s_aCmdProcessBuffer[0];
	
	nCmd = *p;
	nCmdLen = p[1];
	nValue = p[2];

	nCmdLen = nCmdLen;
	
	switch(nCmd)
	{
		case 0x81:
			if(nValue == 0x01)
			{
				g_sPrinter.bReadyOK = true;
			}
			else if(nValue == 0x03)
			{
				g_sPrinter.bOutOfPaper = true;
			}
			else if(nValue == 0x04)
			{
				g_sPrinter.bFinishPrint = true;
			}
			break;
			
		case 0x82:
			break;
	
		case 0x83:
			break;

		case 0x80:
			break;
	}
}

void Printer_Process_Command(void)
{
	static unsigned char nCheckSum;
	static unsigned short nDataLength;
	
	while(s_nUartOutCnt != s_nUartInCnt)
	{
		switch(s_eUARTRecievePhase)
		{
		case eUARTPhaseHead0:
			if(Get_Uart_Data() == 0xBC)
			{
				s_bStartRevCmd = true;
				s_nCmdProcessBufferCnt = 0;
				g_nStartRevCmdTime = Get_System_Time();
				s_eUARTRecievePhase = eUARTPhaseHead1;
			}
			break;

		case eUARTPhaseHead1:
			if(Get_Uart_Data() == 0xFD)
			{
				s_eUARTRecievePhase = eUARTPhaseCmd;
			}
			break;

		case eUARTPhaseCmd:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			nDataLength = 0;
			nCheckSum = 0;
			s_eUARTRecievePhase = eUARTPhaseDataLength;
			break;

		case eUARTPhaseDataLength:
			nDataLength = Get_Uart_Data();
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = nDataLength;
			
			if(nDataLength >= UART_BUFFER_SIZE)
			{
				Uart_Recieve_Error_Handle(__LINE__);
			}
			else if(nDataLength)
			{
				s_eUARTRecievePhase = eUARTPhaseData;
			}
			else
			{
				s_eUARTRecievePhase = eUARTPhaseCrc1;
			}
			break;

		case eUARTPhaseData:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			nCheckSum += s_aCmdProcessBuffer[s_nCmdProcessBufferCnt - 1];
			if(s_nCmdProcessBufferCnt >= UART_CMD_BUFFER_SIZE)
			{
				Uart_Recieve_Error_Handle(__LINE__);
			}
			if(--nDataLength == 0)
			{
				s_eUARTRecievePhase = eUARTPhaseCrc1;
			}
			break;

		case eUARTPhaseCrc1:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			if(s_nCmdProcessBufferCnt >= UART_CMD_BUFFER_SIZE)
			{
				Uart_Recieve_Error_Handle(__LINE__);
				break;
			}
			s_eUARTRecievePhase = eUARTPhaseCrc2;
			break;

		case eUARTPhaseCrc2:
			s_aCmdProcessBuffer[s_nCmdProcessBufferCnt++] = Get_Uart_Data();
			if(s_nCmdProcessBufferCnt >= UART_CMD_BUFFER_SIZE)
			{
				Uart_Recieve_Error_Handle(__LINE__);
				break;
			}
			s_eUARTRecievePhase = eUARTPhaseEnd;
			break;

		case eUARTPhaseEnd:
			if(Get_Uart_Data() == 0xAE)
			{
				Uart_Cmd_Parser_Sub();
				pApp->nStartIdleTime = Get_System_Time();
				s_bStartRevCmd = false;
				s_eUARTRecievePhase = eUARTPhaseHead0;
			}
			break;
		}
	}

	if(s_bStartRevCmd && Get_System_Time()  > g_nStartRevCmdTime +  UART_CMD_TIMOUT)
	{
		Uart_Recieve_Error_Handle(__LINE__);
	}
}


