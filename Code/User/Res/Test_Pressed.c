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

    Source file: Test_Pressed
    Dimensions:  22 * 22
    NumColors:   63

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

static GUI_CONST_STORAGE GUI_COLOR ColorsTest_Pressed[] =
{
	0x00FF00, 0x56FFFF, 0x56FEFE, 0x02FFFF
	, 0x0AFCFC, 0x04FFFF, 0x0CFDFD, 0x00FFFF
	, 0x0AFDFD, 0x08FEFE, 0x19FDFD, 0x09FEFE
	, 0x03FFFF, 0x00FFE4, 0x05FFFF, 0x0CFCFC
	, 0x0CFBFB, 0x07FEFE, 0x1EEEEE, 0x0FF7F7
	, 0x08FDFD, 0x21EBEB, 0x16F4F4, 0x0FFAFA
	, 0x0BFFFF, 0x0AFFFF, 0x0EF9F8, 0x05FEFE
	, 0x01FFFF, 0x16FFFF, 0x13FCFC, 0x15FBFB
	, 0x17FBFB, 0x19FAFA, 0x1FF6F6, 0x18FFFF
	, 0x20FFFF, 0x18F6F6, 0x0EFAFA, 0x12FFFF
	, 0x15F8F7, 0x0DFBFB, 0x13FDFD, 0x0FF8F8
	, 0x06FFFF, 0x06FEFE, 0x14FAFA, 0x09FDFD
	, 0x14F4F4, 0x0BFCFC, 0x27F0F0, 0x17F0EF
	, 0x12FDFD, 0x2BF9F9, 0x10F4F4, 0x25FBFB
	, 0x10F4F3, 0x17FFFF, 0x12FEFE, 0x13FEFE
	, 0x2DFEFE, 0x45FCFC, 0x29FCFC
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalTest_Pressed =
{
	63,	/* number of entries */
	1, 	/* Has transparency */
	&ColorsTest_Pressed[0]
};

static GUI_CONST_STORAGE unsigned char acTest_Pressed[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x05, 0x05, 0x06, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x03, 0x09, 0x0A, 0x07, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0C, 0x09, 0x0D, 0x07, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0E, 0x0B, 0x00, 0x07, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x07, 0x10, 0x00, 0x03, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0C, 0x11, 0x12, 0x00, 0x13, 0x07, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x0E, 0x0E, 0x15, 0x00, 0x00, 0x00, 0x16, 0x07, 0x17, 0x00, 0x00, 0x18, 0x19, 0x19, 0x18, 0x18, 0x18, 0x18, 0x00, 0x00,
	0x00, 0x00, 0x0C, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x1C, 0x00, 0x00, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x00, 0x00,
	0x00, 0x00, 0x24, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x07, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00,
	0x00, 0x00, 0x24, 0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x29, 0x1C, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A, 0x00, 0x00,
	0x00, 0x00, 0x0C, 0x11, 0x10, 0x29, 0x26, 0x2B, 0x29, 0x0E, 0x1C, 0x00, 0x00, 0x2C, 0x2D, 0x2E, 0x21, 0x1F, 0x11, 0x07, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x11, 0x01, 0x2F, 0x11, 0x07, 0x07, 0x07, 0x1C, 0x00, 0x00, 0x0C, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x30, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x1C, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x07, 0x1C, 0x1F, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x11, 0x31, 0x31, 0x14, 0x07, 0x07, 0x07, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x11, 0x1C, 0x0C, 0x32, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x05, 0x2D, 0x2D, 0x0E, 0x07, 0x07, 0x07, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x33, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x34, 0x01, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x2C, 0x14, 0x0B, 0x11, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x35, 0x36, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x37, 0x38, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x07, 0x07, 0x2C, 0x39, 0x3A, 0x07, 0x07, 0x07, 0x1C, 0x00, 0x00, 0x19, 0x3B, 0x0E, 0x3C, 0x3D, 0x3E, 0x23, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

GUI_CONST_STORAGE GUI_BITMAP bmTest_Pressed =
{
	22, /* XSize */
	22, /* YSize */
	22, /* BytesPerLine */
	8, /* BitsPerPixel */
	acTest_Pressed,  /* Pointer to picture data (indices) */
	&PalTest_Pressed  /* Pointer to palette */
};

/* *** End of file *** */
