#include "platform.h"
#include "delay.h"
#include "CustomDisplayDriver.h"
#include "system_misc.h"
#include "i2c.h"

#define LCD_RST_PIN                         (GPIO_Pin_3)
#define LCD_RST_PORT                        (GPIOD)

#define LCD_PWM_PIN                     (GPIO_Pin_7)
#define LCD_PWM_PORT                        (GPIOB)

#define LCD_BASE                                ((uint32_t)(0x6001FFFE))
#define LCD                                         ((LCD_TypeDef *) LCD_BASE)

/*PWM预分频配置.*/
#define LCD_PWM_PRESCALER                   100

/*PWM频率,单位Hz*/
#define LCD_PWM_FREQUENCY                   1000


/*LCD驱动初始化定义结束*/

void LCD_Display_On(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*PWM*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	TIM_Cmd(TIM4, ENABLE);
}

void LCD_Display_Off(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	TIM_Cmd(TIM4, DISABLE);
	
	/*PWM*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIOD->BRR=GPIO_Pin_12;
}

static unsigned short Get_Pwm_Timer_Period(void)
{
	return ((Get_System_Clk_Frequency())  /
			(LCD_PWM_PRESCALER * LCD_PWM_FREQUENCY));
}

static void Lcd_Back_Light_Pwm_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_DeInit(TIM4);

	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE);

	TIM_TimeBaseStructure.TIM_Period = Get_Pwm_Timer_Period() - 1;    /*Timer 自动重载值*/
	TIM_TimeBaseStructure.TIM_Prescaler = LCD_PWM_PRESCALER - 1;  /*预分频*/
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = Get_Pwm_Timer_Period() * 0.10;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	TIM_OC1Init(TIM4, &TIM_OCInitStructure);
}

/*Percent 值越大(最大值 为100),亮度越高.*/
void Lcd_Back_Light_Pwm_Pulse_Config(unsigned char nPercent)
{
	unsigned long nPluseValue;
	nPluseValue = (unsigned long)Get_Pwm_Timer_Period() * nPercent / (float)100;
	TIM_SetCompare1(TIM4, nPluseValue);
}

static void LCD_FSMCConfig(void)
{
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef  p;
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	/*-- GPIO Configuration ------------------------------------------------------*/
	/* SRAM Data lines configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_9 |
								  GPIO_Pin_10 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
								  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	/* SRAM Address lines configuration ---LCD_RS*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* NOE and NWE configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* NE1 configuration */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOD, &GPIO_InitStructure);


	/* Enable FSMC clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);

	/*-- FSMC Configuration ------------------------------------------------------*/
	/*----------------------- SRAM Bank 1 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM1 configuration */
	p.FSMC_AddressSetupTime = 0/* 1*/;
	p.FSMC_AddressHoldTime = 0;
	p.FSMC_DataSetupTime = 4/*9*/;
	p.FSMC_BusTurnAroundDuration = 0;
	p.FSMC_CLKDivision = 0;
	p.FSMC_DataLatency = 0;
	p.FSMC_AccessMode = FSMC_AccessMode_A;

	/*  Color LCD configuration ------------------------------------
	    LCD configured as follow:
	    - Data/Address MUX = Disable
	    - Memory Type = SRAM
	    - Data Width = 16bit
	    - Write Operation = Enable
	    - Extended Mode = Enable
	    - Asynchronous Wait = Disable */

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &p;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	/* Enable FSMC NOR/SRAM Bank1 */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}



void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue)
{
	/* Write 16-bit Index, then Write Reg */
	LCD->LCD_REG = LCD_Reg;
	/* Write 16-bit Reg */
	LCD->LCD_RAM = LCD_RegValue;
}

uint16_t LCD_ReadReg(uint8_t LCD_Reg)
{
	/* Write 16-bit Index (then Read Reg) */
	LCD->LCD_REG = LCD_Reg;
	/* Read 16-bit Reg */
	return (LCD->LCD_RAM);
}

/**
    @brief  Prepare to write to the LCD RAM.
    @param  None
    @retval None
*/
void LCD_WriteRAM_Prepare(void)
{
	LCD->LCD_REG = 0x0022;
}

/**
    @brief  Writes to the LCD RAM.
    @param  RGB_Code: the pixel color in RGB mode (5-6-5).
    @retval None
*/
void LCD_WriteRAM(uint16_t RGB_Code)
{
	/* Write 16-bit GRAM Reg */
	LCD->LCD_RAM = RGB_Code;
}

/**
    @brief  Reads the LCD RAM.
    @param  None
    @retval LCD RAM Value.
*/
uint16_t LCD_ReadRAM(void)
{
	/* Write 16-bit Index (then Read Reg) */
	LCD->LCD_REG = 0x22; /* Select GRAM Reg */
	/* Read 16-bit Reg */
	return LCD->LCD_RAM;
}

/*********************************************************************
    Function:  void LCD_Init()

    PreCondition: none

    Input: none

    Output: none

    Side Effects: none

    Overview: resets LCD, initializes PMP

    Note: none

********************************************************************/
void LCD_Driver_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/*LCD Reset*/
	GPIO_InitStructure.GPIO_Pin = LCD_RST_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(LCD_RST_PORT, &GPIO_InitStructure);

	LCD_FSMCConfig();
	Lcd_Back_Light_Pwm_Init();

	delay_ms(100);

#if LCD_DRIVER_HX8347_D
	/* Reset LCD */
	GPIO_ResetBits(LCD_RST_PORT, LCD_RST_PIN);
	delay_ms(100); /* delay 100 ms */
	GPIO_SetBits(LCD_RST_PORT, LCD_RST_PIN);
	delay_ms(100);
	
	#define DelayMs(x) delay_ms(x)
	
	// Driving ability setting
	LCD_WriteReg(0xEA,0x00);	 
	LCD_WriteReg(0xEB,0x20);	 
	LCD_WriteReg(0xEC,0x0C);	 
	LCD_WriteReg(0xED,0xC4);	 
	LCD_WriteReg(0xE8,0x40);	 
	LCD_WriteReg(0xE9,0x38);	 
	LCD_WriteReg(0xF1,0x01);	 
	LCD_WriteReg(0xF2,0x10);	 
	LCD_WriteReg(0x27,0xA3);	 

	LCD_WriteReg(0x2E,0x007F);
	LCD_WriteReg(0xEA,0x0000);
	LCD_WriteReg(0xEB,0x0020);
	LCD_WriteReg(0xEC,0x003C);
	LCD_WriteReg(0xED,0x00C4);
	LCD_WriteReg(0xE8,0x0040);
	LCD_WriteReg(0xE9,0x0038);
	LCD_WriteReg(0xF1,0x0001);
	LCD_WriteReg(0xF2,0x0008);

	//Gamma 2.2 Setting
	LCD_WriteReg(0x40,0x0001);
	LCD_WriteReg(0x41,0x001F);
	LCD_WriteReg(0x42,0x0025);
	LCD_WriteReg(0x43,0x002C);
	LCD_WriteReg(0x44,0x001E);
	LCD_WriteReg(0x45,0x0026);
	LCD_WriteReg(0x46,0x0005);
	LCD_WriteReg(0x47,0x0070);
	LCD_WriteReg(0x48,0x0006);
	LCD_WriteReg(0x49,0x0014);
	LCD_WriteReg(0x4A,0x0019);
	LCD_WriteReg(0x4B,0x001A);
	LCD_WriteReg(0x4C,0x0017);
	LCD_WriteReg(0x50,0x0019);
	LCD_WriteReg(0x51,0x0021);
	LCD_WriteReg(0x52,0x001D);
	LCD_WriteReg(0x53,0x001A);
	LCD_WriteReg(0x54,0x0020);
	LCD_WriteReg(0x55,0x003E);
	LCD_WriteReg(0x56,0x000A);
	LCD_WriteReg(0x57,0x004A);
	LCD_WriteReg(0x58,0x0008);
	LCD_WriteReg(0x59,0x0005);
	LCD_WriteReg(0x5A,0x0006);
	LCD_WriteReg(0x5B,0x000B);
	LCD_WriteReg(0x5C,0x0019);
	LCD_WriteReg(0x5D,0x00CC);
	//Power Voltage Setting
	LCD_WriteReg(0x1B,0x001B);
	LCD_WriteReg(0x1A,0x0005);
	LCD_WriteReg(0x24,0x0070);
	LCD_WriteReg(0x25,0x0058);
	//****VCOM offset**///

	LCD_WriteReg(0x16,0x88); ////#2 修改镜像方向

	LCD_WriteReg(0x23,0x0068);//0x0080
	//Power on Setting
	LCD_WriteReg(0x18,0x003C);//0x0036
	LCD_WriteReg(0x19,0x0001);
	LCD_WriteReg(0x01,0x0000);
	LCD_WriteReg(0x1F,0x0088);
	DelayMs(5);
	LCD_WriteReg(0x1F,0x0080);
	DelayMs(5);
	LCD_WriteReg(0x1F,0x0090);
	DelayMs(5);
	LCD_WriteReg(0x1F,0x00D0);
	DelayMs(5);
	//262k/65k color selection
	LCD_WriteReg(0x17,0x0005);
	//SET PANEL
	LCD_WriteReg(0x36,0x0000);
	//Display ON Setting
	LCD_WriteReg(0x28,0x0038);
	delay_ms(40);
	LCD_WriteReg(0x28,0x003C);
	//Set GRAM Area
	LCD_WriteReg(0x02,0x0000);
	LCD_WriteReg(0x03,0x0000);
	LCD_WriteReg(0x04,0x0000);
	LCD_WriteReg(0x05,0x00EF);
	LCD_WriteReg(0x06,0x0000);
	LCD_WriteReg(0x07,0x0000);
	LCD_WriteReg(0x08,0x0001);
	LCD_WriteReg(0x09,0x003F);
	
#elif LCD_DRIVER_ILI9341

	#define delayms(x)				delay_ms(x)
	#define LCD_ILI9341_CMD(x) 		LCD->LCD_REG = x
	#define LCD_ILI9341_INDEX(x)	LCD->LCD_RAM = x

	GPIO_ResetBits(LCD_RST_PORT, LCD_RST_PIN);
	delayms(100);
	GPIO_SetBits(LCD_RST_PORT, LCD_RST_PIN);
	delayms(100);

	LCD_ILI9341_CMD(0xCF);  
	LCD_ILI9341_INDEX(0x00);   
	LCD_ILI9341_INDEX(0xC1);
	LCD_ILI9341_INDEX(0x30);

	LCD_ILI9341_CMD(0xED);  
	LCD_ILI9341_INDEX(0x64);   
	LCD_ILI9341_INDEX(0x03);
	LCD_ILI9341_INDEX(0x12);
	LCD_ILI9341_INDEX(0x81);

	LCD_ILI9341_CMD(0xE8);  
	LCD_ILI9341_INDEX(0x85);   
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x7A);

	LCD_ILI9341_CMD(0xCB);  
	LCD_ILI9341_INDEX(0x39);   
	LCD_ILI9341_INDEX(0x2C);
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x34);
	LCD_ILI9341_INDEX(0x02);

	LCD_ILI9341_CMD(0xF7);
	LCD_ILI9341_INDEX(0x20);

	LCD_ILI9341_CMD(0xEA);  
	LCD_ILI9341_INDEX(0x00);   
	LCD_ILI9341_INDEX(0x00);

	LCD_ILI9341_CMD(0xc0);
	LCD_ILI9341_INDEX(0x21);

	LCD_ILI9341_CMD(0xc1);
	LCD_ILI9341_INDEX(0x11);

	LCD_ILI9341_CMD(0xc5);
	LCD_ILI9341_INDEX(0x20);
	LCD_ILI9341_INDEX(0x37);

	LCD_ILI9341_CMD(0xc7);  
	LCD_ILI9341_INDEX(0xb0); //20170714

	LCD_ILI9341_CMD(0x36);
	LCD_ILI9341_INDEX(0xC8 );//旋转180

	LCD_ILI9341_CMD(0x2A);     //set the page address 竖屏幕设置
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0xEF);
	LCD_ILI9341_CMD(0x2B);    //set the column address
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x01);
	LCD_ILI9341_INDEX(0x3F); 

	LCD_ILI9341_CMD(0xb6);
	LCD_ILI9341_INDEX(0x0a);
	LCD_ILI9341_INDEX(0xA2);

	LCD_ILI9341_CMD(0xb1);
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x1B);

	LCD_ILI9341_CMD(0xf2);
	LCD_ILI9341_INDEX(0x00);

	LCD_ILI9341_CMD(0x26);
	LCD_ILI9341_INDEX(0x01);

	LCD_ILI9341_CMD(0x3a);
	LCD_ILI9341_INDEX(0x55);

	LCD_ILI9341_CMD(0xE0);
	LCD_ILI9341_INDEX(0x0f);
	LCD_ILI9341_INDEX(0x2D);
	LCD_ILI9341_INDEX(0x0e);
	LCD_ILI9341_INDEX(0x08);
	LCD_ILI9341_INDEX(0x12);
	LCD_ILI9341_INDEX(0x0a);
	LCD_ILI9341_INDEX(0x3d);
	LCD_ILI9341_INDEX(0x95);
	LCD_ILI9341_INDEX(0x31);
	LCD_ILI9341_INDEX(0x04);
	LCD_ILI9341_INDEX(0x10);
	LCD_ILI9341_INDEX(0x09);
	LCD_ILI9341_INDEX(0x09);
	LCD_ILI9341_INDEX(0x0d);
	LCD_ILI9341_INDEX(0x00);

	LCD_ILI9341_CMD(0xE1);
	LCD_ILI9341_INDEX(0x00);
	LCD_ILI9341_INDEX(0x12);
	LCD_ILI9341_INDEX(0x17);
	LCD_ILI9341_INDEX(0x03);
	LCD_ILI9341_INDEX(0x0d);
	LCD_ILI9341_INDEX(0x05);
	LCD_ILI9341_INDEX(0x2c);
	LCD_ILI9341_INDEX(0x44);
	LCD_ILI9341_INDEX(0x41);
	LCD_ILI9341_INDEX(0x05);
	LCD_ILI9341_INDEX(0x0f);
	LCD_ILI9341_INDEX(0x0a);
	LCD_ILI9341_INDEX(0x30);
	LCD_ILI9341_INDEX(0x32);
	LCD_ILI9341_INDEX(0x0F);

	LCD_ILI9341_CMD(0x11);
	delayms(120);  
	LCD_ILI9341_CMD(0x29);
#endif

	delay_ms(200);
}


void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos)
{
	LCD->LCD_REG = (0x002a);
	LCD->LCD_RAM = (Xpos>>8)&0x00ff;
	LCD->LCD_RAM = Xpos&0x00ff;
	LCD->LCD_RAM = (Xpos>>8)&0x00ff;
	LCD->LCD_RAM = Xpos&0x00ff;
	LCD->LCD_REG = (0x002b);
	LCD->LCD_RAM = (Ypos>>8)&0x00ff;
	LCD->LCD_RAM = Ypos&0x00ff;
	LCD->LCD_RAM = (Ypos>>8)&0x00ff;
	LCD->LCD_RAM = Ypos&0x00ff;
	LCD->LCD_REG = (0x002c);
}

void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
#if LCD_DRIVER_HX8347_D
	LCD->LCD_REG = 0x02;
	LCD->LCD_RAM = x0>>8;
	LCD->LCD_REG = 0x03;
	LCD->LCD_RAM = x0;
	LCD->LCD_REG = 0x04;
	LCD->LCD_RAM = x1>>8;
	LCD->LCD_REG = 0x05;
	LCD->LCD_RAM = x1;
	LCD->LCD_REG = 0x06;
	LCD->LCD_RAM = y0>>8;
	LCD->LCD_REG = 0x07;
	LCD->LCD_RAM = y0;
	LCD->LCD_REG = 0x08;
	LCD->LCD_RAM = y1>>8;
	LCD->LCD_REG = 0x09;
	LCD->LCD_RAM = y1;
#else
	LCD->LCD_REG = (0x002a);
	LCD->LCD_RAM = (x0>>8)&0x00ff;
	LCD->LCD_RAM = x0&0x00ff;
	LCD->LCD_RAM = (x1>>8)&0x00ff;
	LCD->LCD_RAM = x1&0x00ff;
	LCD->LCD_REG = (0x002b);
	LCD->LCD_RAM = (y0>>8)&0x00ff;
	LCD->LCD_RAM = y0&0x00ff;
	LCD->LCD_RAM = (y1>>8)&0x00ff;
	LCD->LCD_RAM = y1&0x00ff;
	LCD->LCD_REG = (0x002c);
#endif
}

void Touch_IC_Init(void)
{
	I2C_Write_Command(0x90, 0xC2);
}


