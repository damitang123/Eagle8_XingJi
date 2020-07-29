#include "platform.h"
#include "uart.h"
#include "uart_process.h"

unsigned char g_nCurrentPrintfUART = eUSE_UART1_PRINTF;

/**************************************************************************************/

void Debug_Uart_Init(void)
{

}

void Setup_USART1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA|RCC_APB2Periph_AFIO, ENABLE);

	/* Configure USARTx_Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure USARTx_Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate = 230400;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_2;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Configure the USARTx */ 
	USART_Init(USART1, &USART_InitStructure);

	/* Enable the USARTx */
	USART_Cmd(USART1, ENABLE);

	//串口接收中断开启
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 
	USART_ClearITPendingBit(USART1, USART_IT_RXNE); 

	USART_ClearFlag(USART1, USART_FLAG_TC);

	USART_GetFlagStatus(USART1, USART_FLAG_TC);
	
	g_nCurrentPrintfUART= eUSE_UART1_PRINTF;
}


/*TRACE_PRINTF 重映射函数*/
int fputc(int ch, FILE *f)
{
	switch(g_nCurrentPrintfUART)
	{
		case eUSE_UART1_PRINTF:
			USART1->DR = (ch & 0xFF);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
			break;
		case eUSE_UART2_PRINTF:
			USART2->DR =  (ch & 0xFF);
			while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
			break;
		case eUSE_UART3_PRINTF:
			USART3->DR =  (ch & 0xFF);
			while(USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
			break;
		case eUSE_UART4_PRINTF:
			UART4->DR =  (ch & 0xFF);
			while(USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET);
			break;
		case eUSE_UART5_PRINTF:
			UART5->DR =  (ch & 0xFF);
			while(USART_GetFlagStatus(UART5, USART_FLAG_TC) == RESET);
			break;
		default:
			USART1->DR =  (ch & 0xFF);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
			break;
	}
	return ch;
}


void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET) 
	{   	
		Receive_Data_From_UART1(USART_ReceiveData(USART1));
	}
}

