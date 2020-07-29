#ifndef _GPIO_H_
#define _GPIO_H_

typedef enum
{
	eGpio_Power_On,				/*系统电源*/
	eGpio_System_Work_Led,		/*工作指示灯*/
	eGpio_Gps_Power_On,			/*Gps电源开关*/
	eGpio_Lcd_BackLight,			/*屏背光指示灯*/
	eGpio_Sample_Power_On,		/*采样板电源总开关*/
	eGpio_Pump_Charge_Sw_Ctrl,	/*泵充电开关*/
	eGpio_Pump_On_Sw_Ctrl,			/*泵使能开关*/
	eGpio_Alc_Clean_Sw_Ctrl,		/*清零开关*/
	eGpio_QuickTest_Alc_Clean_Sw_Ctrl,/*快排清零开关*/
	eGpio_Buzzer_Ctrl,				/*BUZZER开关*/
	eGpio_Heat_Sw_Ctrl,				/*加热开关*/
	eGpio_Moto_Ctrl,					/*循环抽气泵开关*/
	eGpio_IR_Sw_Ctrl,				/*拍照红外灯开关*/
}eGpioControls;

void Gpio_Init(void);
void Gpio_Ctrl(eGpioControls eGpio,bool bNewState);
void System_Led_Flash(void);

#endif
