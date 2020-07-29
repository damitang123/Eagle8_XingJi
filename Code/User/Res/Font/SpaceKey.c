/*
  C-file generated by �C/GUI-FontConvert Demo  V2.16b
  Compiled:    Jul 23 2002 at 13:13:49

  (c) 2002  Micrium, Inc.
  www.micrium.com

  (c) 1998-2002  






http://www.ucgui.com
2005/07/24
www.ucgui.com


  Source file: SpaceKey.c
  Font:        __
  Height:      16
*/

#include "GUI.H"

#ifndef GUI_FLASH
  #define GUI_FLASH
#endif

/* The following line needs to be included in any file selecting the
   font. A good place would be GUIConf.H
*/
extern GUI_FLASH const GUI_FONT GUI_FontSpaceKey;

/* Start of unicode area <CJK Unified Ideographs> */
GUI_FLASH const unsigned char acFontSpaceKey_7A7A[ 32] = 
{ /* code 7A7A */
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x01,0x20,0x01,0x3F,0xFF,0x00,0x00
};

GUI_FLASH const GUI_CHARINFO GUI_FontSpaceKey_CharInfo[1] = {
   {  16,  16,  2, (void GUI_FLASH *)&acFontSpaceKey_7A7A } /* code 7A7A */
};

GUI_FLASH const GUI_FONT_PROP GUI_FontSpaceKey_Prop1 = {
   0x7A7A /* first character */
  ,0x7A7A /* last character  */
  ,&GUI_FontSpaceKey_CharInfo[  0] /* address of first character */
  ,(void*)0 /* pointer to next GUI_FONT_PROP */
};

GUI_FLASH const GUI_FONT GUI_FontSpaceKey = {
   GUI_FONTTYPE_PROP /* type of font    */
  ,16 /* height of font  */
  ,16 /* space of font y */
  ,1 /* magnification x */
  ,1 /* magnification y */
  ,(void GUI_FLASH *)&GUI_FontSpaceKey_Prop1
};
