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

    Source file: Signal3
    Dimensions:  29 * 17
    NumColors:   38

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

static GUI_CONST_STORAGE GUI_COLOR ColorsSignal3[] =
{
	0x0000FF, 0x272727, 0x5F5F5F, 0xC2C2C2
	, 0xF7F7F7, 0x919191, 0x8F8F8F, 0xF6F6F6
	, 0x8D8D8D, 0x808080, 0xF4F4F4, 0x8C8C8C
	, 0x8E8E8E, 0xF2F2F2, 0x8B8B8B, 0xF0F0F0
	, 0xEEEEEE, 0xEBEBEB, 0xE8E8E8, 0xE6E6E6
	, 0xB8B8B8, 0xE3E3E3, 0xE0E0E0, 0xF3F3F3
	, 0xDEDEDE, 0x1C1C1C, 0xDCDCDC, 0x0F0F0F
	, 0xE7E7E7, 0xDADADA, 0x070707, 0x646464
	, 0xA6A6A6, 0x282828, 0xACACAC, 0xA8A8A8
	, 0xDBDBDB, 0xBDBDBD
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalSignal3 =
{
	38,	/* number of entries */
	1, 	/* Has transparency */
	&ColorsSignal3[0]
};

static GUI_CONST_STORAGE unsigned char acSignal3[] =
{
	0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x02, 0x03, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x05, 0x06, 0x07, 0x08, 0x01, 0x01, 0x07, 0x07, 0x01, 0x01, 0x08, 0x07, 0x06, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x09, 0x06, 0x0A, 0x0B, 0x01, 0x0A, 0x0A, 0x01, 0x0B, 0x0A, 0x06, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x09, 0x0C, 0x0D, 0x0E, 0x0D, 0x0D, 0x0E, 0x0D, 0x0C, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x09, 0x08, 0x0F, 0x0F, 0x0F, 0x0F, 0x08, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x09, 0x0B, 0x10, 0x10, 0x0B, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x11, 0x11, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x12, 0x12, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x13, 0x13, 0x01, 0x00, 0x00, 0x00, 0x14, 0x02, 0x01, 0x02, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x15, 0x15, 0x01, 0x00, 0x00, 0x00, 0x02, 0x03, 0x04, 0x03, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x16, 0x16, 0x01, 0x00, 0x00, 0x00, 0x01, 0x17, 0x17, 0x17, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x18, 0x18, 0x19, 0x02, 0x01, 0x02, 0x19, 0x10, 0x10, 0x10, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1A, 0x1A, 0x1B, 0x03, 0x04, 0x03, 0x1B, 0x1C, 0x1C, 0x1C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x1D, 0x1D, 0x1E, 0x10, 0x10, 0x10, 0x1E, 0x16, 0x16, 0x16, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x20, 0x20, 0x21, 0x22, 0x16, 0x22, 0x21, 0x23, 0x24, 0x23, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x1F, 0x1F, 0x0B, 0x1F, 0x01, 0x1F, 0x0B, 0x1F, 0x01, 0x1F, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

GUI_CONST_STORAGE GUI_BITMAP bmSignal3 =
{
	29, /* XSize */
	17, /* YSize */
	29, /* BytesPerLine */
	8, /* BitsPerPixel */
	acSignal3,  /* Pointer to picture data (indices) */
	&PalSignal3  /* Pointer to palette */
};

/* *** End of file *** */
