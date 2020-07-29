#ifndef _GPIO_H_
#define _GPIO_H_

typedef enum
{
	eGpio_Power_On,				/*ϵͳ��Դ*/
	eGpio_System_Work_Led,		/*����ָʾ��*/
	eGpio_Gps_Power_On,			/*Gps��Դ����*/
	eGpio_Lcd_BackLight,			/*������ָʾ��*/
	eGpio_Sample_Power_On,		/*�������Դ�ܿ���*/
	eGpio_Pump_Charge_Sw_Ctrl,	/*�ó�翪��*/
	eGpio_Pump_On_Sw_Ctrl,			/*��ʹ�ܿ���*/
	eGpio_Alc_Clean_Sw_Ctrl,		/*���㿪��*/
	eGpio_QuickTest_Alc_Clean_Sw_Ctrl,/*�������㿪��*/
	eGpio_Buzzer_Ctrl,				/*BUZZER����*/
	eGpio_Heat_Sw_Ctrl,				/*���ȿ���*/
	eGpio_Moto_Ctrl,					/*ѭ�������ÿ���*/
	eGpio_IR_Sw_Ctrl,				/*���պ���ƿ���*/
}eGpioControls;

void Gpio_Init(void);
void Gpio_Ctrl(eGpioControls eGpio,bool bNewState);
void System_Led_Flash(void);

#endif
