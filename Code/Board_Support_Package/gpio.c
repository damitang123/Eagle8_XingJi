#include "platform.h"
#include "gpio.h"

void Gpio_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*System Power IO*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_Power_On, false);

	/*LED指示灯*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_System_Work_Led, true);

	/*Gps 电源使能*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_Gps_Power_On, true);

	/*采样板电源总开关.*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_Sample_Power_On, true);

	/*ALCOHOL SENSOR CLEAN*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_Alc_Clean_Sw_Ctrl, true);

	/*PUMP Discharge switch,放电开关*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_Pump_On_Sw_Ctrl, false);

	/*PUMP Charge EN,充电开关*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_Pump_Charge_Sw_Ctrl, false);

#if QUICK_TEST
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_Moto_Ctrl, false);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	Gpio_Ctrl(eGpio_QuickTest_Alc_Clean_Sw_Ctrl, true);
#endif	

	/*AD8320 CS*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIOA->BSRR = GPIO_Pin_6;

	/*RTC CS*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIOB->BRR = GPIO_Pin_12;

	/*Extern printer*/
	GPIOE->BRR = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	GPIOE->BRR = GPIO_Pin_2;

	/*触摸屏探测IO.*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Gpio_Ctrl(eGpioControls eGpio, bool bNewState)
{
	switch(eGpio)
	{
		case eGpio_Power_On:
			if(bNewState)
			{
				GPIOA->BSRR = GPIO_Pin_0;
			}
			else
			{
				GPIOA->BRR = GPIO_Pin_0;
			}
			break;

		case eGpio_System_Work_Led:
			if(bNewState)
			{
				GPIOD->BSRR = GPIO_Pin_13;
			}
			else
			{
				GPIOD->BRR = GPIO_Pin_13;
			}
			break;

		case eGpio_Gps_Power_On:
			if(bNewState)
			{
				GPIOA->BSRR = GPIO_Pin_1;
			}
			else
			{
				GPIOA->BRR = GPIO_Pin_1;
			}
			break;

		case eGpio_Lcd_BackLight:
			if(bNewState)
			{
				GPIOE->BSRR = GPIO_Pin_6;
			}
			else
			{
				GPIOE->BRR = GPIO_Pin_6;
			}
			break;

		case eGpio_Sample_Power_On:
			if(bNewState)
			{
				GPIOB->BSRR = GPIO_Pin_1;
			}
			else
			{
				GPIOB->BRR = GPIO_Pin_1;
			}
			break;

		case eGpio_Pump_Charge_Sw_Ctrl:
			if(bNewState)
			{
				GPIOC->BSRR = GPIO_Pin_8;
			}
			else
			{
				GPIOC->BRR = GPIO_Pin_8;
			}
			break;

		case eGpio_Pump_On_Sw_Ctrl:
			if(bNewState)
			{
				GPIOC->BSRR = GPIO_Pin_7;
			}
			else
			{
				GPIOC->BRR = GPIO_Pin_7;
			}
			break;

		case eGpio_Alc_Clean_Sw_Ctrl:
			/*低时清零..高时可采样实时酒精ADC*/
			if(bNewState)
			{
				GPIOB->BRR = GPIO_Pin_15;
			}
			else
			{
				GPIOB->BSRR = GPIO_Pin_15;
			}
			break;

		case eGpio_QuickTest_Alc_Clean_Sw_Ctrl:
			/*低时清零..高时可采样实时酒精ADC*/
			if(bNewState)
			{
				GPIOE->BRR = GPIO_Pin_6;
			}
			else
			{
				GPIOE->BSRR = GPIO_Pin_6;
			}
			break;

		case eGpio_Moto_Ctrl:
			if(bNewState)
			{
				GPIOA->BSRR = GPIO_Pin_15;
			}
			else
			{
				GPIOA->BRR = GPIO_Pin_15;
			}
			break;
	}
}

void System_Led_Flash(void)
{
	static unsigned char nLedFlashCnt = 0;
	switch(nLedFlashCnt++)
	{
		case 0:
			Gpio_Ctrl(eGpio_System_Work_Led, true);
			break;
		case 20:
			Gpio_Ctrl(eGpio_System_Work_Led, false);
			break;
		case 40:
			Gpio_Ctrl(eGpio_System_Work_Led, true);
			break;
		case 50:
			Gpio_Ctrl(eGpio_System_Work_Led, false);
			break;
		case 60:
			Gpio_Ctrl(eGpio_System_Work_Led, true);
			break;
		case 70:
			Gpio_Ctrl(eGpio_System_Work_Led, false);
			break;
		case 200:
			nLedFlashCnt = 0;
			break;
	}
}
