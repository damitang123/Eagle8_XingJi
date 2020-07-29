/*
*********************************************************************************************************
*                                             uC/GUI V3.98
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              礐/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUICharP.C
Purpose     : Implementation of Proportional fonts
---------------------------END-OF-HEADER------------------------------
*/

#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Private.h"

void Extern_FLASH_Read_GBK_FontImage(U16 ch);
void Extern_FLASH_Read_Unicode_FontImage(U16 ch);

#define  USE_GBK_FONT		0
#define  USE_UNICODE_FONT 	1



#if  ((USE_GBK_FONT+USE_UNICODE_FONT)!=1)
	#error "You are olny allowed to choose one between USE_GBK_FONT and USE_UNICODE_FONT"
#endif

#if USE_GBK_FONT
	#define USE_FLASH_FONT	(GUI_Context.pAFont==&GUI_Font16_GB2312|| \
							GUI_Context.pAFont==&GUI_Font16_GB2312_2SIZE||\
							GUI_Context.pAFont==&GUI_Font_FLASH)
#elif USE_UNICODE_FONT
	extern const unsigned char aUnicode_Width;
	
	extern const GUI_FONT GUI_Font16_Unicode;
	extern const GUI_FONT GUI_Font16_UTF8;
	extern const GUI_FONT GUI_Font16_UTF8X2;
	extern const GUI_FONT GUI_Font16_UTF8X3;
	
	#define USE_FLASH_FONT	(GUI_Context.pAFont==&GUI_Font16_Unicode|| \
							GUI_Context.pAFont==&GUI_Font16_UTF8||	\
							GUI_Context.pAFont==&GUI_Font16_UTF8X2||	\
							GUI_Context.pAFont==&GUI_Font16_UTF8X3)
#endif
 
/*********************************************************************
*
*       Static code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIPROP_FindChar
*/
static const GUI_FONT_PROP GUI_UNI_PTR * GUIPROP_FindChar(const GUI_FONT_PROP GUI_UNI_PTR* pProp, U16P c) {
#if USE_GBK_FONT
	if(USE_FLASH_FONT)
	{
		/*如果是外部FLASH汉字,则不需要遍历链表.*/
		return pProp;
	}
#elif USE_UNICODE_FONT
	if(USE_FLASH_FONT)
	{
		/*如果是外部FLASH汉字,则不需要遍历链表.*/
		Extern_FLASH_Read_Unicode_FontImage(c);
		return pProp;
	}
#endif
else
{
  for (; pProp; pProp = pProp->pNext) {
    if ((c>=pProp->First) && (c<=pProp->Last))
      break;
  }
}
  return pProp;
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       GUIPROP_DispChar
*
* Purpose:
*   This is the routine that displays a character. It is used by all
*   other routines which display characters as a subroutine.
*/
void GUIPROP_DispChar(U16P c) {
  int BytesPerLine;
  GUI_DRAWMODE DrawMode = GUI_Context.TextMode;
  const GUI_FONT_PROP GUI_UNI_PTR * pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
  if (pProp) {
    GUI_DRAWMODE OldDrawMode;
	const GUI_CHARINFO GUI_UNI_PTR * pCharInfo;
#if USE_GBK_FONT
	if(USE_FLASH_FONT)
	{
		/*获取字符c的实际宽度.字体图形矩阵等信息*/
		if(c<0x1000)
		 	pCharInfo= pProp->paCharInfo;
		else if((c>>8)>=0x81)
			pCharInfo= pProp->paCharInfo+1;

		/*从FLASH中获取字符字体图形.*/
		Extern_FLASH_Read_GBK_FontImage(c);
	}
#elif USE_UNICODE_FONT
	if(USE_FLASH_FONT)
	{
		/*获取字符c的实际宽度.字体图形矩阵等信息*/
		if(aUnicode_Width)
			pCharInfo= pProp->paCharInfo+1;
		else
			pCharInfo= pProp->paCharInfo;

		/*从FLASH中获取字符字体图形.*/
		Extern_FLASH_Read_Unicode_FontImage(c);
	}
#endif
	else
	{
	   	pCharInfo = pProp->paCharInfo+(c-pProp->First);
	}
    BytesPerLine = pCharInfo->BytesPerLine;
    OldDrawMode  = LCD_SetDrawMode(DrawMode);
    LCD_DrawBitmap( GUI_Context.DispPosX, GUI_Context.DispPosY,
                       pCharInfo->XSize,
											 GUI_Context.pAFont->YSize,
                       GUI_Context.pAFont->XMag,
											 GUI_Context.pAFont->YMag,
                       1,     /* Bits per Pixel */
                       BytesPerLine,
                       pCharInfo->pData,
                       &LCD_BKCOLORINDEX
                       );
    /* Fill empty pixel lines */
    if (GUI_Context.pAFont->YDist > GUI_Context.pAFont->YSize) {
      int YMag = GUI_Context.pAFont->YMag;
      int YDist = GUI_Context.pAFont->YDist * YMag;
      int YSize = GUI_Context.pAFont->YSize * YMag;
      if (DrawMode != LCD_DRAWMODE_TRANS) {
        LCD_COLOR OldColor = GUI_GetColor();
        GUI_SetColor(GUI_GetBkColor());
        LCD_FillRect(GUI_Context.DispPosX, 
                     GUI_Context.DispPosY + YSize, 
                     GUI_Context.DispPosX + pCharInfo->XSize, 
                     GUI_Context.DispPosY + YDist);
        GUI_SetColor(OldColor);
      }
    }
    LCD_SetDrawMode(OldDrawMode); /* Restore draw mode */
    GUI_Context.DispPosX += pCharInfo->XDist * GUI_Context.pAFont->XMag;
  }
}

/*********************************************************************
*
*       GUIPROP_GetCharDistX
*/
int GUIPROP_GetCharDistX(U16P c) {
  const GUI_FONT_PROP GUI_UNI_PTR * pProp = GUIPROP_FindChar(GUI_Context.pAFont->p.pProp, c);
#if USE_GBK_FONT
if(USE_FLASH_FONT)
{
	if(c<0x1000)
		return (pProp) ? (pProp->paCharInfo+0)->XSize * GUI_Context.pAFont->XMag : 0;
	else if((c>>8)>=0x81)
		return (pProp) ? (pProp->paCharInfo+1)->XSize * GUI_Context.pAFont->XMag : 0;
}
#elif USE_UNICODE_FONT
if(USE_FLASH_FONT)
{
	if(aUnicode_Width)
		return (pProp) ? (pProp->paCharInfo+1)->XSize * GUI_Context.pAFont->XMag : 0;
	else
		return (pProp) ? (pProp->paCharInfo+0)->XSize * GUI_Context.pAFont->XMag : 0;
}
#endif
else
{
  	return (pProp) ? (pProp->paCharInfo+(c-pProp->First))->XSize * GUI_Context.pAFont->XMag : 0;
}
}

/*********************************************************************
*
*       GUIPROP_GetFontInfo
*/
void GUIPROP_GetFontInfo(const GUI_FONT GUI_UNI_PTR * pFont, GUI_FONTINFO* pfi) {
  GUI_USE_PARA(pFont);
  pfi->Flags = GUI_FONTINFO_FLAG_PROP;
}

/*********************************************************************
*
*       GUIPROP_IsInFont
*/
char GUIPROP_IsInFont(const GUI_FONT GUI_UNI_PTR * pFont, U16 c) {
  const GUI_FONT_PROP GUI_UNI_PTR * pProp = GUIPROP_FindChar(pFont->p.pProp, c);
  return (pProp==NULL) ? 0 : 1;
}

/*************************** End of file ****************************/
	 	 			 		    	 				 	  			   	 	 	 	 	 	  	  	      	   		 	 	 		  		  	 		 	  	  			     			       	   	 			  		    	 	     	 				  	 					 	 			   	  	  			 				 		 	 	 			     			 
