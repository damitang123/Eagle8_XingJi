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

    Source file: GPS_Location
    Dimensions:  13 * 19
    NumColors:   17

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

static GUI_CONST_STORAGE GUI_COLOR ColorsGPS_Location[] =
{
	0x0000FF, 0xAD8C77, 0xFDF7ED, 0xD09679
	, 0xEDD4BF, 0xFDF5E9, 0xCDAB91, 0xF0D0B2
	, 0xCFAB8D, 0xDDC2AB, 0xDEAC89, 0xFFE8C5
	, 0xCE9B7C, 0xDDC2AC, 0xD8A787, 0xA2826E
	, 0xC2917A
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalGPS_Location =
{
	17,	/* number of entries */
	1, 	/* Has transparency */
	&ColorsGPS_Location[0]
};

static GUI_CONST_STORAGE unsigned char acGPS_Location[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x04, 0x05, 0x04, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x06, 0x02, 0x02, 0x02, 0x06, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x01, 0x07, 0x02, 0x02, 0x02, 0x07, 0x01, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x02, 0x02, 0x02, 0x02, 0x02, 0x08, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x03, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x00, 0x00,
	0x00, 0x00, 0x09, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x09, 0x00, 0x00,
	0x00, 0x01, 0x0A, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0A, 0x01, 0x00,
	0x00, 0x03, 0x02, 0x02, 0x02, 0x0B, 0x01, 0x0B, 0x02, 0x02, 0x02, 0x03, 0x00,
	0x00, 0x0C, 0x02, 0x02, 0x0B, 0x01, 0x00, 0x01, 0x0B, 0x02, 0x02, 0x0C, 0x00,
	0x01, 0x0B, 0x0B, 0x0D, 0x01, 0x00, 0x00, 0x00, 0x01, 0x0D, 0x0B, 0x0B, 0x01,
	0x03, 0x0E, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0E, 0x03,
	0x10, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x10,
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10
};

GUI_CONST_STORAGE GUI_BITMAP bmGPS_Location =
{
	13, /* XSize */
	19, /* YSize */
	13, /* BytesPerLine */
	8, /* BitsPerPixel */
	acGPS_Location,  /* Pointer to picture data (indices) */
	&PalGPS_Location  /* Pointer to palette */
};

/* *** End of file *** */