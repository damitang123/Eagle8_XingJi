#ifndef _CUSTOM_DISPLAY_DRIVER_H
#define _CUSTOM_DISPLAY_DRIVER_H

#include "GenericTypeDefs.h"

typedef struct
{
	volatile uint16_t LCD_REG;
	volatile uint16_t LCD_RAM;
} LCD_TypeDef;

#define LCD_BACK_LIGHT_MIN_PWM_DUTY		10
#define LCD_BACK_LIGHT_MAX_PWM_DUTY		100

void LCD_Display_On(void);
void LCD_Display_Off(void);
void Lcd_Back_Light_Pwm_Pulse_Config(unsigned char nPercent);
void LCD_Driver_Init(void);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);	
uint16_t LCD_ReadReg(uint8_t LCD_Reg);
void LCD_WriteReg(uint8_t LCD_Reg, uint16_t LCD_RegValue);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
uint16_t LCD_ReadRAM(void);
void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

void Touch_IC_Init(void);

#endif

