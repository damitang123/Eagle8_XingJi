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

    Source file: Test_Released
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

static GUI_CONST_STORAGE GUI_COLOR ColorsTest_Released[] =
{
	0x00FF00, 0xE5E5E5, 0xFFFFFF, 0xFCFCFC
	, 0xF9F9F9, 0xCBCCCB, 0xEEEEEE, 0xEFEFEF
	, 0xE3E3E3, 0xFEFEFE, 0xD2D3D2, 0xDDDDDD
	, 0xECECEC, 0xDBDBDB, 0xF3F3F3, 0xD3D4D3
	, 0xEDEDED, 0xD9D9D9, 0xC5C6C5, 0xC3C4C3
	, 0xF8F8F8, 0xEBEBEB, 0xBBB9B8, 0xA5A7A5
	, 0xD7D7D7, 0xBCBAB8, 0xA9ABA9, 0xBDBEBD
	, 0xAFB1AF, 0xE6E6E6, 0xFBFBFB, 0xDBD9D9
	, 0xDAD8D8, 0xD9D7D7, 0xD8D6D6, 0xB9BAB9
	, 0xBEBCBA, 0xBFC0BF, 0xC4C5C4, 0xC0C1C0
	, 0xACAEAC, 0xBEBFBE, 0xE7E7E7, 0xCFD0CF
	, 0xE2E2E2, 0xD6D6D6, 0xDFDFDF, 0xF4F4F4
	, 0xA6A8A6, 0xFAFAFA, 0xC9CAC9, 0xF6F6F6
	, 0xD4D4D4, 0xF1F1F1, 0xB3B5B3, 0x9A9D9A
	, 0x919491, 0xEAEAEA, 0xD8D8D8, 0xDADADA
	, 0x969996, 0x949794, 0xFDFFFD
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalTest_Released =
{
	63,	/* number of entries */
	1, 	/* Has transparency */
	&ColorsTest_Released[0]
};

static GUI_CONST_STORAGE unsigned char acTest_Released[] =
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x04, 0x0B, 0x0C, 0x02, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x0E, 0x0B, 0x00, 0x02, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x11, 0x00, 0x02, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x02, 0x13, 0x00, 0x14, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x0E, 0x08, 0x16, 0x00, 0x17, 0x09, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x15, 0x15, 0x19, 0x00, 0x00, 0x00, 0x1A, 0x02, 0x1B, 0x00, 0x00, 0x02, 0x04, 0x04, 0x04, 0x04, 0x04, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1D, 0x1E, 0x00, 0x00, 0x02, 0x1F, 0x1F, 0x20, 0x21, 0x22, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x09, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, 0x23, 0x09, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x09, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x1E, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x08, 0x26, 0x27, 0x23, 0x28, 0x29, 0x10, 0x04, 0x00, 0x00, 0x02, 0x2A, 0x0A, 0x2B, 0x0F, 0x2C, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x1D, 0x2D, 0x18, 0x2E, 0x02, 0x02, 0x02, 0x04, 0x00, 0x00, 0x2F, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x30, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x31, 0x00, 0x00, 0x32, 0x33, 0x02, 0x02, 0x02, 0x1E, 0x34, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x2C, 0x32, 0x05, 0x18, 0x02, 0x02, 0x02, 0x31, 0x00, 0x00, 0x00, 0x1A, 0x08, 0x02, 0x0E, 0x1B, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x35, 0x1D, 0x2A, 0x0C, 0x02, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x36, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x37, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x38, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x39, 0x3A, 0x3B, 0x08, 0x02, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1E, 0x3C, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x09, 0x1E, 0x1E, 0x03, 0x02, 0x02, 0x02, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3D, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x02, 0x02, 0x3E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x31, 0x00, 0x00, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x03, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

GUI_CONST_STORAGE GUI_BITMAP bmTest_Released =
{
	22, /* XSize */
	22, /* YSize */
	22, /* BytesPerLine */
	8, /* BitsPerPixel */
	acTest_Released,  /* Pointer to picture data (indices) */
	&PalTest_Released  /* Pointer to palette */
};

/* *** End of file *** */