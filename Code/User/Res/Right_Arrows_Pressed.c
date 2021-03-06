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

    Source file: Right_Arrows_Pressed
    Dimensions:  20 * 13
    NumColors:   6

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

static GUI_CONST_STORAGE GUI_COLOR ColorsRight_Arrows_Pressed[] =
{
	0x00FF00, 0x00FFFF, 0x00F9F9, 0x00FFFD
	, 0x00FFFC, 0x00FFFE
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalRight_Arrows_Pressed =
{
	6,	/* number of entries */
	1, 	/* Has transparency */
	&ColorsRight_Arrows_Pressed[0]
};

static GUI_CONST_STORAGE unsigned char acRight_Arrows_Pressed[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x12, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x41, 0x12, 0x00, 0x00,
	0x05, 0x55, 0x55, 0x55, 0x55, 0x55, 0x53, 0x51, 0x20, 0x00,
	0x05, 0x55, 0x55, 0x55, 0x53, 0x55, 0x55, 0x51, 0x11, 0x20,
	0x05, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x53, 0x15,
	0x05, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x11, 0x20,
	0x05, 0x55, 0x55, 0x55, 0x53, 0x55, 0x55, 0x51, 0x20, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x31, 0x20, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x12, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

GUI_CONST_STORAGE GUI_BITMAP bmRight_Arrows_Pressed =
{
	20, /* XSize */
	13, /* YSize */
	10, /* BytesPerLine */
	4, /* BitsPerPixel */
	acRight_Arrows_Pressed,  /* Pointer to picture data (indices) */
	&PalRight_Arrows_Pressed  /* Pointer to palette */
};

/* *** End of file *** */
