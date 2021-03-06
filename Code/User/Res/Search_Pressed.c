/*********************************************************************
                 SEGGER MICROCONTROLLER SYSTEME GmbH
         Solutions for real time microcontroller applications
                            www.segger.com
**********************************************************************

    C-file generated by

         �C/GUI-BitmapConvert V3.90.
         Compiled Aug 19 2004, 09:07:56
           (c) 2002  Micrium, Inc.
    www.micrium.com

    (c) 1998-2002  Segger
    Microcontroller Systeme GmbH
    www.segger.com

**********************************************************************

    Source file: Search_Pressed
    Dimensions:  22 * 22
    NumColors:   10

**********************************************************************
*/

#include "stdlib.h"

#include "GUI.h"

#ifndef GUI_CONST_STORAGE
	#define GUI_CONST_STORAGE const
#endif

/*   Palette
    The following are the entries of the palette table.
    Every entry is a 32-bit value (of which 24 bits are actually used)
    the lower   8 bits represent the Red component,
    the middle  8 bits represent the Green component,
    the highest 8 bits (of the 24 bits used) represent the Blue component
    as follows:   0xBBGGRR
*/

static GUI_CONST_STORAGE GUI_COLOR ColorsSearch_Pressed[] =
{
	0x00FF00, 0x00FFFF, 0x0AFFFF, 0x00F9F9
	, 0x08FFFF, 0x11FFFF, 0x18FFFF, 0x1AFFFF
	, 0x12FFFF, 0x0EFFFF
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalSearch_Pressed =
{
	10,	/* number of entries */
	1, 	/* Has transparency */
	&ColorsSearch_Pressed[0]
};

static GUI_CONST_STORAGE unsigned char acSearch_Pressed[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x11, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x11, 0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x23, 0x00, 0x00, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x14, 0x10, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00, 0x00,
	0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
	0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
	0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
	0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x03, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x11, 0x30, 0x00, 0x00, 0x00, 0x01, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x13, 0x00, 0x00, 0x00, 0x11, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x11, 0x30, 0x00, 0x03, 0x11, 0x50, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x11, 0x11, 0x11, 0x16, 0x17, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x11, 0x10, 0x00, 0x11, 0x80, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x91, 0x11, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x10,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

GUI_CONST_STORAGE GUI_BITMAP bmSearch_Pressed =
{
	22, /* XSize */
	22, /* YSize */
	11, /* BytesPerLine */
	4, /* BitsPerPixel */
	acSearch_Pressed,  /* Pointer to picture data (indices) */
	&PalSearch_Pressed  /* Pointer to palette */
};

/* *** End of file *** */
