#include "platform.h"
#include "buzzer.h"
#include "gpio.h"

/*是否使用PWM控制蜂鸣器*/
#if defined(USE_4G_PCB)
#define BUZZER_USE_PWM_CTRL		1
#else
#define BUZZER_USE_PWM_CTRL		1 //Modified by lxll
#endif

volatile sBuzzerConfig s_sBuzzerConfig;

const sBuzzerActionStruct *pBuzzerCtrl;
static bool s_bBuzzerCurrentState;
static unsigned char s_nActionStep = 0;
static unsigned char s_nActionTimeCnt = 0;

#if BUZZER_USE_PWM_CTRL

void Buzzer_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
	TIM_DeInit(TIM3);

	/*Timer时基设置外设时钟频率*/
	TIM_TimeBaseStructure.TIM_Period = 2370;	/*Timer 自动重载值*/
	TIM_TimeBaseStructure.TIM_Prescaler = 9;	/*预分频*/
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; /**/
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 2370 / 2;	/*占空比.要符合脉冲小于周期的原则*/
	TIM_OC3Init(TIM3, &TIM_OCInitStructure); 
}

void Buzzer_On(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	TIM_Cmd(TIM3,ENABLE);
}

void Buzzer_Off(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	TIM_Cmd(TIM3,DISABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIOB->BRR=GPIO_Pin_0;
}

#else

void Buzzer_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	Buzzer_Off();
}

void Buzzer_On(void)
{
	GPIOB->BSRR=GPIO_Pin_0;
}

void Buzzer_Off(void)
{
	GPIOB->BRR=GPIO_Pin_0;
}

#endif

void _Beep(unsigned short  nOnDuration, unsigned short  nOffDuration, unsigned short  nRepeatCnt)
{
	s_sBuzzerConfig.bIsBusy = true;
	s_sBuzzerConfig.nTimeCnt = 0;
	s_sBuzzerConfig.eCurrentState = eBuzzerOn;
	s_sBuzzerConfig.nOnDuration = nOnDuration / 10;
	s_sBuzzerConfig.nOffDuration = nOffDuration / 10;
	s_sBuzzerConfig.nRepeatCnt = nRepeatCnt;
}

#if 0
void Buzzer_Beep(EBuzzerType eBeep)
{
	if(eBeep == eBUZ_START_TEST)
	{
		_Beep(400, 400, 3);
	}
	else if(eBeep == eBUZ_DEVICE_OK)
	{
		_Beep(600, 0, 1);
	}
	else if(eBeep == eBUZ_BLOW_BREAK)
	{
		_Beep(200, 200, 2);
	}
	else if(eBeep == eBUZ_REPORT_PASS)
	{
		_Beep(500, 0, 1);
	}
	else if(eBeep == eBUZ_REPORT_FAIL)
	{
		_Beep(200, 150, 3);
	}
	else if(eBeep == eBUZ_REPORT)
	{
		_Beep(120, 50, 3);
	}
	else if(eBeep == eBUZ_BLOW_OK)
	{
		_Beep(500, 0, 1);
	}
	else if(eBeep == eBUZ_ALRAM)
	{
		_Beep(700, 0, 1);
	}
	else if(eBeep == eBUZ_ERROR)
	{
		_Beep(100, 0, 1);
	}
	else if(eBeep == eBUZ_CRITICAL)
	{
		_Beep(100, 50, 3);
	}
	else if(eBeep == eBUZ_OK)
	{
		_Beep(120, 0, 1);
	}
	else if(eBeep == eBUZ_START)
	{
		_Beep(300, 0, 1);
	}
	else if(eBeep == eBUZ_BREAK)
	{
		_Beep(100, 50, 3);
	}
	else if(eBeep == eBUZ_TIMEOUT)
	{
		_Beep(100, 50, 3);
	}
	else if(eBeep == eBUZ_LINK)
	{
		_Beep(30, 0, 1);
	}
	else if(eBeep == eBUZ_PRESSOK)
	{
		_Beep(50, 0, 1);
	}
	else if(eBeep == eOP)
	{
		_Beep(110, 0, 1);
	}
}
#else

static const sBuzzerActionStruct s_BuzStartTest =
{
	true,
	false,
	{40,40,40,40,40,40,0}
};

static const sBuzzerActionStruct s_BuzDeviceOK =
{
	true,
	false,
	{60,0}
};

static const sBuzzerActionStruct s_BuzOK =
{
	true,
	false,
	{12,0}
};

static const sBuzzerActionStruct s_BuzBlowBreak =
{
	false,
	false,
	{10,20,10,10,0}
};

static const sBuzzerActionStruct s_BuzReportPass =
{
	true,
	false,
	{50,0}
};

static const sBuzzerActionStruct s_BuzReportFail =
{
	true,
	false,
	{20,15,20,15,20,15,0}
};

static const sBuzzerActionStruct s_BuzReport =
{
	true,
	false,
	{12,5,12,5,12,5,0}
};

static const sBuzzerActionStruct s_BuzAlarm =
{
	true,
	false,
	{70,0}
};

static const sBuzzerActionStruct s_BuzPressOK =
{
	true,
	false,
	{10,0}
};

static const sBuzzerActionStruct s_BuzError =
{
	true,
	false,
	{30,0}
};

static const sBuzzerActionStruct s_BuzIdentifyFail =
{
	false,
	false,
	{10,20,10,10,0}
};

void _BeepConfig(const sBuzzerActionStruct *pCtrl)
{
	if(pCtrl)
	{
		s_nActionStep = 0;
		s_nActionTimeCnt = 1;

		if(pCtrl->bBuzzerDefaultState)
		{
			Buzzer_On();
			s_bBuzzerCurrentState= true;
		}
		else
		{
			Buzzer_Off();
			s_bBuzzerCurrentState= false;
		}
		pBuzzerCtrl = pCtrl;
	}
}

void Buzzer_Beep(EBuzzerType eBeep)
{
	if(eBeep == eBUZ_START_TEST)
	{
		_BeepConfig( &s_BuzStartTest);
	}
	else if(eBeep == eBUZ_DEVICE_OK)
	{
		_BeepConfig( &s_BuzDeviceOK);
	}
	else if(eBeep == eBUZ_BLOW_BREAK)
	{
		_BeepConfig(&s_BuzBlowBreak);
	}
	else if(eBeep == eBUZ_REPORT_PASS)
	{
		_BeepConfig(&s_BuzReportPass);
	}
	else if(eBeep == eBUZ_REPORT_FAIL)
	{
		_BeepConfig(&s_BuzReportFail);
	}
	else if(eBeep == eBUZ_REPORT)
	{
		_BeepConfig(&s_BuzReport);
	}
	else if(eBeep == eBUZ_BLOW_OK)
	{
		_Beep(500, 0, 1);
	}
	else if(eBeep == eBUZ_ALRAM)
	{
		_BeepConfig(&s_BuzAlarm);
	}
	else if(eBeep == eBUZ_ERROR)
	{
		_BeepConfig(&s_BuzError);
	}
	else if(eBeep == eBUZ_CRITICAL)
	{
		_Beep(100, 50, 3);
	}
	else if(eBeep == eBUZ_OK)
	{
		_BeepConfig( &s_BuzOK);
	}
	else if(eBeep == eBUZ_START)
	{
		_Beep(300, 0, 1);
	}
	else if(eBeep == eBUZ_BREAK)
	{
		_Beep(100, 50, 3);
	}
	else if(eBeep == eBUZ_TIMEOUT)
	{
		_Beep(100, 50, 3);
	}
	else if(eBeep == eBUZ_LINK)
	{
		_Beep(30, 0, 1);
	}
	else if(eBeep == eBUZ_PRESSOK)
	{
		_BeepConfig( &s_BuzPressOK);
	}
	else if(eBeep == eBUZ_FP_IDENTIFY_FAIL)
	{
		_BeepConfig( &s_BuzIdentifyFail);
	}
	else if(eBeep == eOP)
	{
		_Beep(110, 0, 1);
	}
	else
	{
		
	}
}	
#endif

/*被10ms 中断调用.*/
void Buzzer_Ctrl(void)
{
	if(pBuzzerCtrl == NULL)
	{
		return;
	}

	if(s_nActionTimeCnt == pBuzzerCtrl->aCtrlTable[s_nActionStep])
	{
		s_nActionStep ++;
		s_nActionTimeCnt = 1;
		if(pBuzzerCtrl->aCtrlTable[s_nActionStep] == 0)
		{
			if(pBuzzerCtrl->bNeedRepeat)
			{
				/*步数从头开始*/
				s_nActionStep = 0;
				if(pBuzzerCtrl->bBuzzerDefaultState)
				{
					Buzzer_On();
					s_bBuzzerCurrentState= true;
				}
				else
				{
					Buzzer_Off();
					s_bBuzzerCurrentState= false;
				}
			}
			else
			{
				Buzzer_Off();
				pBuzzerCtrl = NULL;
				return;
			}
		}
		else
		{
			if(s_bBuzzerCurrentState)
			{
				Buzzer_Off();
				s_bBuzzerCurrentState = false;
			}
			else
			{
				Buzzer_On();
				s_bBuzzerCurrentState = true;
			}
		}
	}
	else
	{
		s_nActionTimeCnt ++;
	}
}

