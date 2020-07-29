#include "platform.h"
#include "system_misc.h"
#include "keypad.h"
#include "buzzer.h"
#include "app.h"
#include "gpio.h"
#include "gprs.h"

#define OS_TICKS_PER_SEC       1000

volatile unsigned long g_nSystemTime; 
volatile unsigned long g_nSystemClkFrequence;

volatile sTimerEvent g_s10msTimer;
volatile sTimerEvent g_s50msTimer;
volatile sTimerEvent g_s100msTimer;
volatile sTimerEvent g_s500msTimer;
volatile sTimerEvent g_s1000msTimer;

extern volatile int OS_TimeMS;

#define USE_DFU 

void Timer_10ms_Hook(void)
{
	Buzzer_Ctrl();
	if(pApp->bSystemPowerOn)
	{
		Key_Scan();
		if(pApp->eWorkState == eWorkNormal)
		{
			System_Led_Flash();
		}
	}
}

void SysTick_Handler(void)
{
	OS_TimeMS++;
	g_nSystemTime ++;

	Mark_Valid_Package_Via_Timeout();

	if(++ g_s10msTimer.nCnt >= 10)
	{
		g_s10msTimer.nCnt = 0;
		g_s10msTimer.bIntervalEvent = true;

		Timer_10ms_Hook();

		if(++ g_s50msTimer.nCnt >= 5)
		{
			g_s50msTimer.nCnt = 0;
			g_s50msTimer.bIntervalEvent = true;

			if(++ g_s100msTimer.nCnt >= 2)
			{
				g_s100msTimer.nCnt = 0;
				g_s100msTimer.bIntervalEvent = true;
				
				if(++ g_s500msTimer.nCnt >= 5)
				{
					g_s500msTimer.nCnt = 0;
					g_s500msTimer.bIntervalEvent = true;

					if(++ g_s1000msTimer.nCnt >= 2)
					{
						g_s1000msTimer.nCnt = 0;
						g_s1000msTimer.bIntervalEvent = true;
					}
				}
			}
		}
	}
}

unsigned long Get_System_Time(void)
{
	return g_nSystemTime;
}

unsigned long Get_System_Clk_Frequency(void)
{
	return g_nSystemClkFrequence;
}

void Get_CPU_Clock_Info(void)
{
	RCC_ClocksTypeDef  Cpu;

	RCC_GetClocksFreq(&Cpu);

	g_nSystemClkFrequence = Cpu.SYSCLK_Frequency;

	TRACE_PRINTF("Cpu.SYSCLK_Frequency = %ld\r\n",Cpu.SYSCLK_Frequency);
	TRACE_PRINTF("Cpu.HCLK_Frequency = %ld\r\n",Cpu.HCLK_Frequency);
	TRACE_PRINTF("Cpu.PCLK1_Frequency = %ld\r\n",Cpu.PCLK1_Frequency);
	TRACE_PRINTF("Cpu.PCLK2_Frequency = %ld\r\n",Cpu.PCLK2_Frequency);
}

void  System_Tick_Init (void)
{
	g_nSystemTime = 0;

	memset((void *)&g_s10msTimer,0,sizeof(sTimerEvent));
	memset((void *)&g_s50msTimer,0,sizeof(sTimerEvent));
	memset((void *)&g_s100msTimer,0,sizeof(sTimerEvent));
	memset((void *)&g_s500msTimer,0,sizeof(sTimerEvent));
	memset((void *)&g_s1000msTimer,0,sizeof(sTimerEvent));

	Get_CPU_Clock_Info();
	
	SysTick_Config(g_nSystemClkFrequence / OS_TICKS_PER_SEC);

	__enable_irq();
}

void Internal_Clk_Init(void)
{
	RCC_HSICmd(ENABLE);

	while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);

	FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
	FLASH_SetLatency(FLASH_Latency_2);

	RCC_HCLKConfig(RCC_SYSCLK_Div1);   
	RCC_PCLK1Config(RCC_HCLK_Div1);
	RCC_PCLK2Config(RCC_HCLK_Div1);

	RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);                
	RCC_PLLCmd(ENABLE);
	while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);  
	while(RCC_GetSYSCLKSource() != 0x08);
}

void RCC_Configuration(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC|
								RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE
								|RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO, ENABLE);

	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable , ENABLE);
}

void NVIC_Configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	#ifdef  VECT_TAB_RAM
	/* Set the Vector Table base location at 0x20000000 */
	NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0);
	#else  /* VECT_TAB_FLASH  */
	/* Set the Vector Table base location at 0x08000000 */
		#if defined(USE_DFU)
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x3000);
		#else
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
		#endif
	#endif

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);

	/*PC UART*/
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/*GPS*/
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*蓝牙打印机*/
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*GPRS*/
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/*通用定时器中断*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/*ADC 扫描中断.*/
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x05;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}





