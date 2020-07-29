/*
*********************************************************************************************************
*                                                uC/GUI
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : LCDNull.C
Purpose     : Empty driver
              This driver does no perform any function. It can be
              used for measurement purpose.
----------------------------------------------------------------------   
Version-Date---Author-Explanation                                        
----------------------------------------------------------------------   
0.90.00 030627 JE     a) First release
---------------------------END-OF-HEADER------------------------------
*/
#include "platform.h"
#include "LCDConf.h" 
#include "LCD_Private.h"      /* private modul definitions & config */
#include "GUI_Private.h"
#include "GUIDebug.h"
#include <stdlib.h>
#include "CustomDisplayDriver.h"
#include "i2c.h"
#include "parameter.h"
#include "delay.h"
#include "system_misc.h"


#define ENABLE_HARDWARE_ACCELERATION	1

#if (LCD_CONTROLLER == -2)

#define LCD_BASE  	((unsigned long)(0x6001FFFE))
#define LCD1               ((LCD_TypeDef *) LCD_BASE)

#define LCD_Set_Pixel(x)	do{LCD1->LCD_RAM = x;}while(0)
#define LCD_Get_Pixel()	(LCD1->LCD_RAM & 0xffff)

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawBitLine1BPP
*/
static void  _DrawBitLine1BPP(int x, int y, U8 const GUI_UNI_PTR *p, int Diff, int xsize, const LCD_PIXELINDEX *pTrans)
{
	unsigned char bXValid = 1;
	LCD_PIXELINDEX Index0 = *(pTrans + 0);
	LCD_PIXELINDEX Index1 = *(pTrans + 1);
	x += Diff;
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))
	{
		case 0:
#if ENABLE_HARDWARE_ACCELERATION == 0
			do
			{
				LCD_L0_SetPixelIndex(x++, y, (*p & (0x80 >> Diff)) ? Index1 : Index0);
				if (++Diff == 8)
				{
					Diff = 0;
					p++;
				}
			}
			while (--xsize);
#else
		#if LCD_DRIVER_HX8347_D
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
			LCD_WriteRAM_Prepare();
		#elif LCD_DRIVER_ILI9341
		//	LCD_SetCursor(x, y);
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
		#endif
			do
			{
				int PixelIndex = (*p & (0x80 >> Diff)) ? Index1 : Index0;
				LCD_Set_Pixel(PixelIndex);
				if (++Diff == 8)
				{
					Diff = 0;
					p++;
				}
			}
			while (--xsize);
#endif
			break;
		case LCD_DRAWMODE_TRANS:
#if ENABLE_HARDWARE_ACCELERATION == 0
			do
			{
				if (*p & (0x80 >> Diff))
				{
					LCD_L0_SetPixelIndex(x, y, Index1);
				}
				x++;
				if (++Diff == 8)
				{
					Diff = 0;
					p++;
				}
			}
			while (--xsize);
#else
		#if LCD_DRIVER_HX8347_D
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
			LCD_WriteRAM_Prepare();
		#elif LCD_DRIVER_ILI9341
		//	LCD_SetCursor(x, y);
			LCD_SetWindow(x, y,LCD_X_SIZE, y);	
		#endif
			do
			{
				if (*p & (0x80 >> Diff))
				{
					if(bXValid == 0)
					{
						bXValid = 1;
					#if LCD_DRIVER_HX8347_D
						LCD_SetWindow(x, y,LCD_X_SIZE, y);
						LCD_WriteRAM_Prepare();
					#elif LCD_DRIVER_ILI9341
					//	LCD_SetCursor(x, y);
						LCD_SetWindow(x, y,LCD_X_SIZE, y);	
					#endif
					}
					LCD_Set_Pixel(Index1);
				}
				else
				{
					bXValid = 0;
				}
				x++;
				if (++Diff == 8)
				{
					Diff = 0;
					p++;
				}
			}
			while (--xsize);
#endif
			break;
		case LCD_DRAWMODE_XOR | LCD_DRAWMODE_TRANS:
		case LCD_DRAWMODE_XOR:
			do
			{
				if (*p & (0x80 >> Diff))
				{
					int Pixel = LCD_L0_GetPixelIndex(x, y);
					LCD_L0_SetPixelIndex(x, y, LCD_NUM_COLORS - 1 - Pixel);
				}
				x++;
				if (++Diff == 8)
				{
					Diff = 0;
					p++;
				}
			}
			while (--xsize);
			break;
	}
}

/*********************************************************************

        Draw Bitmap 2 BPP
*/
#if (LCD_MAX_LOG_COLORS > 2)
static void  _DrawBitLine2BPP(int x, int y, U8 const GUI_UNI_PTR *p, int Diff, int xsize, const LCD_PIXELINDEX *pTrans)
{
	volatile unsigned char bXValid = 1;
	LCD_PIXELINDEX Pixels = *p;
	int CurrentPixel = Diff;
	x += Diff;
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))
	{
		case 0:
#if ENABLE_HARDWARE_ACCELERATION == 0
			if (pTrans)
			{
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
					LCD_L0_SetPixelIndex(x++, y, PixelIndex);
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					LCD_L0_SetPixelIndex(x++, y, Index);
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#else
			if (pTrans)
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);	
			#endif
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
					LCD_Set_Pixel(PixelIndex);
					x++;
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);	
			#endif
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					LCD_Set_Pixel(Index);
					x++;
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#endif
			break;
		case LCD_DRAWMODE_TRANS:
#if ENABLE_HARDWARE_ACCELERATION == 0
			if (pTrans)
			{
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					if (Index)
					{
						LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
						LCD_L0_SetPixelIndex(x, y, PixelIndex);
					}
					x++;
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					if (Index)
					{
						LCD_L0_SetPixelIndex(x, y, Index);
					}
					x++;
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#else
			if (pTrans)
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);	
			#endif
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					if (Index)
					{
						LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
						if(bXValid == 0)
						{
							bXValid = 1;
						#if LCD_DRIVER_HX8347_D
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
							LCD_WriteRAM_Prepare();
						#elif LCD_DRIVER_ILI9341
						//	LCD_SetCursor(x, y);
							LCD_SetWindow(x, y,LCD_X_SIZE, y);	
						#endif
						}
						LCD_Set_Pixel(PixelIndex);
					}
					else
					{
						bXValid = 0;
					}
					x++;
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);	
			#endif
				do
				{
					int Shift = (3 - CurrentPixel) << 1;
					int Index = (Pixels & (0xC0 >> (6 - Shift))) >> Shift;
					if (Index)
					{
						if(bXValid == 0)
						{
							bXValid = 1;
						#if LCD_DRIVER_HX8347_D
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
							LCD_WriteRAM_Prepare();
						#elif LCD_DRIVER_ILI9341
						//	LCD_SetCursor(x, y);
							LCD_SetWindow(x, y,LCD_X_SIZE, y);	
						#endif
						}
						LCD_Set_Pixel(Pixels);
					}
					else
					{
						bXValid = 0;
					}
					x++;
					if (++CurrentPixel == 4)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#endif
			break;
	}
}
#endif

/*********************************************************************

        Draw Bitmap 4 BPP
*/
#if (LCD_MAX_LOG_COLORS > 4)
static void  _DrawBitLine4BPP(int x, int y, U8 const GUI_UNI_PTR *p, int Diff, int xsize, const LCD_PIXELINDEX *pTrans)
{
	volatile unsigned char bXValid = 1;
	LCD_PIXELINDEX Pixels = *p;
	int CurrentPixel = Diff;
	x += Diff;
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))
	{
		case 0:
#if ENABLE_HARDWARE_ACCELERATION == 0
			if (pTrans)
			{
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
					LCD_L0_SetPixelIndex(x++, y, PixelIndex);
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					LCD_L0_SetPixelIndex(x++, y, Index);
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#else
			if (pTrans)
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
					LCD_Set_Pixel(PixelIndex);
					x++;
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					LCD_Set_Pixel(Index);
					x++;
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#endif
			break;
		case LCD_DRAWMODE_TRANS:
#if ENABLE_HARDWARE_ACCELERATION == 0
			if (pTrans)
			{
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					if (Index)
					{
						LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
						LCD_L0_SetPixelIndex(x, y, PixelIndex);
					}
					x++;
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					if (Index)
					{
						LCD_L0_SetPixelIndex(x, y, Index);
					}
					x++;
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#else
			if (pTrans)
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					if (Index)
					{
						LCD_PIXELINDEX PixelIndex = *(pTrans + Index);
						if(bXValid == 0)
						{
							bXValid = 1;
						#if LCD_DRIVER_HX8347_D
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
							LCD_WriteRAM_Prepare();
						#elif LCD_DRIVER_ILI9341
						//	LCD_SetCursor(x, y);
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
						#endif
						}
						LCD_Set_Pixel(PixelIndex);
					}
					else
					{
						bXValid = 0;
					}
					x++;
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
			else
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				do
				{
					int Shift = (1 - CurrentPixel) << 2;
					int Index = (Pixels & (0xF0 >> (4 - Shift))) >> Shift;
					if (Index)
					{
						if(bXValid == 0)
						{
							bXValid = 1;
						#if LCD_DRIVER_HX8347_D
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
							LCD_WriteRAM_Prepare();
						#elif LCD_DRIVER_ILI9341
						//	LCD_SetCursor(x, y);
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
						#endif
						}
						LCD_Set_Pixel(Index);
					}
					else
					{
						bXValid = 0;
					}
					x++;
					if (++CurrentPixel == 2)
					{
						CurrentPixel = 0;
						Pixels = *(++p);
					}
				}
				while (--xsize);
			}
#endif
			break;
	}
}
#endif

/*********************************************************************

        Draw Bitmap 8 BPP
*/
#if (LCD_MAX_LOG_COLORS > 16)
static void  _DrawBitLine8BPP(int x, int y, U8 const GUI_UNI_PTR *p, int xsize, const LCD_PIXELINDEX *pTrans)
{
	volatile unsigned char bXValid = 1;
	LCD_PIXELINDEX Pixel;
#if ENABLE_HARDWARE_ACCELERATION == 0
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))
	{
		case 0:
			if (pTrans)
			{
				for (; xsize > 0; xsize--, x++, p++)
				{
					Pixel = *p;
					LCD_L0_SetPixelIndex(x, y, *(pTrans + Pixel));
				}
			}
			else
			{
				for (; xsize > 0; xsize--, x++, p++)
				{
					LCD_L0_SetPixelIndex(x, y, *p);
				}
			}
			break;
		case LCD_DRAWMODE_TRANS:
			if (pTrans)
			{
				for (; xsize > 0; xsize--, x++, p++)
				{
					Pixel = *p;
					if (Pixel)
					{
						LCD_L0_SetPixelIndex(x, y, *(pTrans + Pixel));
					}
				}
			}
			else
			{
				for (; xsize > 0; xsize--, x++, p++)
				{
					Pixel = *p;
					if (Pixel)
					{
						LCD_L0_SetPixelIndex(x, y, Pixel);
					}
				}
			}
			break;
	}
#else
	switch (GUI_Context.DrawMode & (LCD_DRAWMODE_TRANS | LCD_DRAWMODE_XOR))
	{
		case 0:
			if (pTrans)
			{
				int intPiexel;
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				for (; xsize > 0; xsize--, x++, p++)
				{
					Pixel = *p;
					intPiexel = *(pTrans + Pixel);
					LCD_Set_Pixel(intPiexel);
				}
			}
			else
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				for (; xsize > 0; xsize--, x++, p++)
				{
					LCD_Set_Pixel(*p);
				}
			}
			break;
		case LCD_DRAWMODE_TRANS:
			if (pTrans)
			{
				int intPiexel;
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				for (; xsize > 0; xsize--, x++, p++)
				{
					Pixel = *p;
					if (Pixel)
					{
						intPiexel = *(pTrans + Pixel);
						if(bXValid == 0)
						{
							bXValid = 1;
						#if LCD_DRIVER_HX8347_D
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
							LCD_WriteRAM_Prepare();
						#elif LCD_DRIVER_ILI9341
						//	LCD_SetCursor(x, y);
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
						#endif
						}
						LCD_Set_Pixel(intPiexel);
					}
					else
					{
						bXValid = 0;
					}
				}
			}
			else
			{
			#if LCD_DRIVER_HX8347_D
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
				LCD_WriteRAM_Prepare();
			#elif LCD_DRIVER_ILI9341
			//	LCD_SetCursor(x, y);
				LCD_SetWindow(x, y,LCD_X_SIZE, y);
			#endif
				for (; xsize > 0; xsize--, x++, p++)
				{
					Pixel = *p;
					if (Pixel)
					{
						if(bXValid == 0)
						{
							bXValid = 1;
						#if LCD_DRIVER_HX8347_D
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
							LCD_WriteRAM_Prepare();
						#elif LCD_DRIVER_ILI9341
						//	LCD_SetCursor(x, y);
							LCD_SetWindow(x, y,LCD_X_SIZE, y);
						#endif
						}
						LCD_Set_Pixel(Pixel);
					}
					else
					{
						bXValid = 0;
					}
				}
			}
			break;
	}
#endif
}
#endif

/*********************************************************************

        Draw Bitmap 16 BPP
*/
#if (LCD_BITSPERPIXEL > 8)
static void  _DrawBitLine16BPP(int x, int y, U16 const GUI_UNI_PTR *p, int xsize, const LCD_PIXELINDEX *pTrans)
{
	volatile unsigned char bXValid = 1;
	LCD_PIXELINDEX pixel;
#if ENABLE_HARDWARE_ACCELERATION == 0
	if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0)
	{
		if (pTrans)
		{
			for (; xsize > 0; xsize--, x++, p++)
			{
				pixel = *p;
				LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
			}
		}
		else
		{
			for (; xsize > 0; xsize--, x++, p++)
			{
				LCD_L0_SetPixelIndex(x, y, *p);
			}
		}
	}
	else
	{
		if (pTrans)
		{
			for (; xsize > 0; xsize--, x++, p++)
			{
				pixel = *p;
				if (pixel)
				{
					LCD_L0_SetPixelIndex(x, y, *(pTrans + pixel));
				}
			}
		}
		else
		{
			for (; xsize > 0; xsize--, x++, p++)
			{
				pixel = *p;
				if (pixel)
				{
					LCD_L0_SetPixelIndex(x, y, pixel);
				}
			}
		}
	}
#else
	if ((GUI_Context.DrawMode & LCD_DRAWMODE_TRANS) == 0)
	{
		if (pTrans)
		{
			int intPiexel;
		#if LCD_DRIVER_HX8347_D
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
			LCD_WriteRAM_Prepare();
		#elif LCD_DRIVER_ILI9341
		//	LCD_SetCursor(x, y);
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
		#endif
			for (; xsize > 0; xsize--, x++, p++)
			{
				pixel = *p;
				intPiexel = *(pTrans + pixel);
				LCD_Set_Pixel(intPiexel);
			}
		}
		else
		{
		#if LCD_DRIVER_HX8347_D
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
			LCD_WriteRAM_Prepare();
		#elif LCD_DRIVER_ILI9341
		//	LCD_SetCursor(x, y);
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
		#endif
			for (; xsize > 0; xsize--, x++, p++)
			{
				LCD_Set_Pixel(*p);
			}
		}
	}
	else
	{
		if (pTrans)
		{
			int intPiexel;
		#if LCD_DRIVER_HX8347_D
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
			LCD_WriteRAM_Prepare();
		#elif LCD_DRIVER_ILI9341
		//	LCD_SetCursor(x, y);
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
		#endif
			for (; xsize > 0; xsize--, x++, p++)
			{
				pixel = *p;
				if (pixel)
				{
					intPiexel = *(pTrans + pixel);
					if(bXValid == 0)
					{
						bXValid = 1;
					#if LCD_DRIVER_HX8347_D
						LCD_SetWindow(x, y,LCD_X_SIZE, y);
						LCD_WriteRAM_Prepare();
					#elif LCD_DRIVER_ILI9341
					//	LCD_SetCursor(x, y);
						LCD_SetWindow(x, y,LCD_X_SIZE, y);
					#endif
					}
					LCD_Set_Pixel(intPiexel);
				}
				else
				{
					bXValid = 0;
				}
			}
		}
		else
		{
		#if LCD_DRIVER_HX8347_D
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
			LCD_WriteRAM_Prepare();
		#elif LCD_DRIVER_ILI9341
		//	LCD_SetCursor(x, y);
			LCD_SetWindow(x, y,LCD_X_SIZE, y);
		#endif
			for (; xsize > 0; xsize--, x++, p++)
			{
				pixel = *p;
				if (pixel)
				{
					if(bXValid == 0)
					{
						bXValid = 1;
					#if LCD_DRIVER_HX8347_D
						LCD_SetWindow(x, y,LCD_X_SIZE, y);
						LCD_WriteRAM_Prepare();
					#elif LCD_DRIVER_ILI9341
					//	LCD_SetCursor(x, y);
						LCD_SetWindow(x, y,LCD_X_SIZE, y);
					#endif
					}
					LCD_Set_Pixel(pixel);
				}
				else
				{
					bXValid = 0;
				}
			}
		}
	}
#endif
}
#endif

/*********************************************************************
*
*       Exported functions
*
**********************************************************************
*/

/*********************************************************************
*
*       LCD_L0_SetPixelIndex
*/
void LCD_L0_SetPixelIndex(int x, int y, int PixelIndex) 
{
#if LCD_DRIVER_HX8347_D
	LCD_SetWindow(x,y,x,y);
	LCD_WriteRAM_Prepare();
	LCD_Set_Pixel(PixelIndex);
#elif LCD_DRIVER_ILI9341
	LCD_SetCursor(x,y);
	LCD_Set_Pixel(PixelIndex);
#endif
}

/*********************************************************************
*
*       LCD_L0_GetPixelIndex
*/
unsigned int LCD_L0_GetPixelIndex(int x, int y) {
	unsigned int r;
#if LCD_DRIVER_HX8347_D
	LCD_SetWindow(x,y,x,y);
	LCD_WriteRAM_Prepare();
#elif LCD_DRIVER_ILI9341
	LCD_SetCursor(x,y);
#endif

	r = LCD_Get_Pixel();
	r = LCD_Get_Pixel();

	return r;
}

/*********************************************************************
*
*       LCD_L0_XorPixel
*/
void LCD_L0_XorPixel(int x, int y) {
	LCD_PIXELINDEX PixelIndex = LCD_L0_GetPixelIndex(x, y);
	LCD_L0_SetPixelIndex(x, y, LCD_NUM_COLORS - PixelIndex - 1);
}

/*********************************************************************
*
*       LCD_L0_DrawHLine
*/
void LCD_L0_DrawHLine(int x0, int y,  int x1) {
	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR)
	{
		for (; x0 <= x1; x0++)
		{
			LCD_L0_XorPixel(x0, y);
		}
	}
	else
	{
#if ENABLE_HARDWARE_ACCELERATION == 0
		for (; x0 <= x1; x0++)
		{
			LCD_L0_SetPixelIndex(x0, y, LCD_COLORINDEX);
		}
#else
	#if LCD_DRIVER_HX8347_D
		LCD_SetWindow(x0, y, x1, y);
		LCD_WriteRAM_Prepare();
	#elif LCD_DRIVER_ILI9341
	//	LCD_SetCursor(x0, y);
		LCD_SetWindow(x0, y,LCD_X_SIZE, y);
	#endif
		for (; x0 <= x1; x0++)
		{
			LCD_Set_Pixel(LCD_COLORINDEX);
		}
#endif
	}
}

/*********************************************************************
*
*       LCD_L0_DrawVLine
*/
void LCD_L0_DrawVLine(int x, int y0,  int y1) {
	int i;

	if (GUI_Context.DrawMode & LCD_DRAWMODE_XOR) {
		for (i=y0; i<=y1; i++){
			LCD_L0_XorPixel(x, i);
		}
	} else {
		for (i=y0; i<=y1; i++) {
			LCD_L0_SetPixelIndex(x, i, LCD_COLORINDEX);
		}		
	}
}

/*********************************************************************
*
*       LCD_L0_FillRect
*/
void LCD_L0_FillRect(int x0, int y0, int x1, int y1) {
	int i;
	for (i=y0; i<=y1; i++) {
		LCD_L0_DrawHLine(x0, i, x1);
	}
}


/*********************************************************************
*
*       LCD_L0_FillRectWithData
*/
void LCD_L0_FillRect_WithData(int x0, int y0, int x1, int y1,void *pData) {
	GUI_RECT r;
	int i,j,PixelIndex;
	unsigned short *p = (unsigned short *)pData;
	
	if(x0 < 0)
	{
		r.x0 = 0;
	}
	else if(x0 >= LCD_XSIZE)
	{
		r.x0 = LCD_XSIZE - 1;
	}
	else
	{
		r.x0 = x0;
	}

	if(y0 < 0)
	{
		r.y0 = 0;
	}
	else if(y0 >= LCD_YSIZE)
	{
		r.y0 = LCD_YSIZE - 1;
	}
	else
	{
		r.y0 = y0;
	}

	if(x1 < 0)
	{
		r.x1 = 0;
	}
	else if(x1 >= LCD_XSIZE)
	{
		r.x1 = LCD_XSIZE - 1;
	}
	else
	{
		r.x1 = x1;
	}

	if(y1 < 0)
	{
		r.y1 = 0;
	}
	else if(y1 >= LCD_YSIZE)
	{
		r.y1 = LCD_YSIZE - 1;
	}
	else
	{
		r.y1 = y1;
	}

	if(r.x0 == r.x1 ||r.y0 == r.y1)
	{
		return;
	}
	
	LCD_SetWindow(r.x0, r.y0, r.x1, r.y1);
	
	for(i = y0; i <= y1; i++)
	{
		for (j = x0; j <= x1; j++)
		{
			if(i >= 0 && i < LCD_YSIZE && j >= 0 && j < LCD_XSIZE)
			{
				memcpy((void*)&PixelIndex, (void *)p, 2);
				LCD_Set_Pixel(PixelIndex);
			}
			p++;
		}
	}
}

/*********************************************************************
*
*       LCD_L0_DrawBitmap
*/
void LCD_L0_DrawBitmap(int x0, int y0,
                       int xsize, int ysize,
                       int BitsPerPixel, 
                       int BytesPerLine,
                       const U8 GUI_UNI_PTR * pData, int Diff,
                       const LCD_PIXELINDEX* pTrans)
{
	int i;
	/* Use _DrawBitLineXBPP */
	for (i = 0; i < ysize; i++)
	{
		switch (BitsPerPixel)
		{
			case 1:
				_DrawBitLine1BPP(x0, i + y0, pData, Diff, xsize, pTrans);
				break;
#if (LCD_MAX_LOG_COLORS > 2)
			case 2:
				_DrawBitLine2BPP(x0, i + y0, pData, Diff, xsize, pTrans);
				break;
#endif
#if (LCD_MAX_LOG_COLORS > 4)
			case 4:
				_DrawBitLine4BPP(x0, i + y0, pData, Diff, xsize, pTrans);
				break;
#endif
#if (LCD_MAX_LOG_COLORS > 16)
			case 8:
				_DrawBitLine8BPP(x0, i + y0, pData, xsize, pTrans);
				break;
#endif
#if (LCD_BITSPERPIXEL > 8)
			case 16:
				_DrawBitLine16BPP(x0, i + y0, (const U16 *)pData, xsize, pTrans);
				break;
#endif
		}
		pData += BytesPerLine;
	}
}

/*********************************************************************
*
*       LCD_L0_SetOrg
*/
void LCD_L0_SetOrg(int x, int y) {
  GUI_USE_PARA(x);
  GUI_USE_PARA(y);
}

/*********************************************************************
*
*       LCD_On / LCD_Off
*/
void LCD_On (void) 
{
	LCD_Display_On();
	Lcd_Back_Light_Pwm_Pulse_Config(Get_Actual_LcdBackLight_Percent(g_sSystemParam.nBackLightPercent));
}
void LCD_Off(void) 
{
	Lcd_Back_Light_Pwm_Pulse_Config(0);
	LCD_Display_Off();
}

/*********************************************************************
*
*       LCD_L0_Init
*/
int LCD_L0_Init(void) 
{
	unsigned long T;
	I2C_Hal_Init();
	LCD_Driver_Init();

	/*¶Á´¥ÃþÆÁ×ÜÏß*/
	T = Get_System_Time();
	do
	{
		Touch_IC_Init();
		if(g_bI2CValidAck == false)
		{
			delay_ms(40);
		}
		else
		{
			break;
		}
	}while(Get_System_Time() > T + 200);
	
  	return 0;
}

/*********************************************************************
*
*       LCD_L0_SetLUTEntry
*/
void LCD_L0_SetLUTEntry(U8 Pos, LCD_COLOR Color) {
  GUI_USE_PARA(Pos);
  GUI_USE_PARA(Color);
}


/*********************************************************************

        LCD_L0_GetDevFunc
*/
void *LCD_L0_GetDevFunc(int Index)
{
	GUI_USE_PARA(Index);
	return NULL;
}

#else

void LCDNull_c(void);
void LCDNull_c(void) {} /* avoid empty object files */

#endif /* (LCD_CONTROLLER == -2) */
