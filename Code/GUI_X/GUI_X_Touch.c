/*
*********************************************************************************************************
                                                 uC/GUI
                         Universal graphic software for embedded applications

                        (c) Copyright 2002, Micrium Inc., Weston, FL
                        (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH

               µC/GUI is protected by international copyright laws. Knowledge of the
               source code may not be used to write a similar product. This file may
               only be used in accordance with a license and should not be redistributed
               in any way. We appreciate your understanding and fairness.

    ----------------------------------------------------------------------
    File        : GUI_TOUCH_X.C
    Purpose     : Config / System dependent externals for GUI
    ---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUI_X.h"
#include "stm32f10x.h"
#include "i2c.h"
#include "public_function.h"
#include "stdlib.h"

#define   ADDR_TSC2003                      	0x90
#define  TOUCH_SHAKE_RANGE              100

#define     Read_TSC2003_PENIRQ             (GPIOB->IDR & GPIO_Pin_5)

volatile short xAdc = -1;
volatile short yAdc = -1;


void GUI_TOUCH_X_ActivateX(void)
{
}

void GUI_TOUCH_X_ActivateY(void)
{
}

int  GUI_TOUCH_X_MeasureX(void)
{
	char i;
	unsigned short x[5];

	if(Read_TSC2003_PENIRQ == 0)
	{
		for(i = 0; i < 5; i++)
		{
			if(Read_TSC2003_PENIRQ == 0)
			{
				I2C_Block_Read(ADDR_TSC2003, (unsigned char *)&x[i], 0xC2, 2);
				x[i] >>= 4;
			}
			else
			{
				xAdc = -1;
				return (int)xAdc;
			}
		}

		Array_Sort_By_Ascending(x, sizeof(x) / sizeof(unsigned short));
		if(abs(x[1] - x[2]) < TOUCH_SHAKE_RANGE && abs(x[2] - x[3]) < TOUCH_SHAKE_RANGE && abs(x[1] - x[3]) < TOUCH_SHAKE_RANGE)
		{
			xAdc = (x[1] + x[2] + x[3]) / 3;
		}
		else
		{
			xAdc = -1;
			return (int)xAdc;
		}
	}
	else
	{
		xAdc = -1;
	}

	return (int)xAdc;

}

int  GUI_TOUCH_X_MeasureY(void)
{
	char i;
	unsigned short y[5];
	if(Read_TSC2003_PENIRQ == 0)
	{
		for(i = 0; i < 5; i++)
		{
			if(Read_TSC2003_PENIRQ == 0)
			{
				I2C_Block_Read(ADDR_TSC2003, (unsigned char *)&y[i], 0xD2, 2);
				y[i] >>= 4;
			}
			else
			{
				yAdc = -1;
				return (int)yAdc;
			}
		}

		Array_Sort_By_Ascending(y, sizeof(y) / sizeof(unsigned short));
		if(abs(y[1] - y[2]) < TOUCH_SHAKE_RANGE && abs(y[2] - y[3]) < TOUCH_SHAKE_RANGE && abs(y[1] - y[3]) < TOUCH_SHAKE_RANGE)
		{
			yAdc = (y[1] + y[2] + y[3]) / 3;
		}
		else
		{
			yAdc = -1;
			return (int)yAdc;
		}
	}
	else
	{
		yAdc = -1;
	}

	return (int)yAdc;
}


