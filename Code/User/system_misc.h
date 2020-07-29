#ifndef _SYSTEM_MISC_H_
#define _SYSTEM_MISC_H_

#include "platform.h"

#define SYS_ENTER_CRITICAL		do{__disable_irq();}while(0)
#define SYS_EXIT_CRITICAL		do{__enable_irq();}while(0)

void Internal_Clk_Init(void);
void RCC_Configuration(void);
void NVIC_Configuration(void);
void Internal_Clk_Init(void);
void Get_CPU_Clock_Info(void);
void System_Tick_Init (void);
unsigned long Get_System_Time(void);
unsigned long Get_System_Clk_Frequency(void);

typedef struct
{
	bool bIntervalEvent;
	unsigned char nCnt;
}sTimerEvent;


extern volatile unsigned long g_nSystemTime;

extern volatile sTimerEvent g_s10msTimer;
extern volatile sTimerEvent g_s50msTimer;
extern volatile sTimerEvent g_s100msTimer;
extern volatile sTimerEvent g_s500msTimer;
extern volatile sTimerEvent g_s1000msTimer;

#endif

