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

    Source file: BATTERY4
    Dimensions:  24 * 12
    NumColors:   209

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

static GUI_CONST_STORAGE GUI_COLOR ColorsBATTERY4[] =
{
	0xFF0000, 0x000000, 0x011800, 0x000900
	, 0x222222, 0x010101, 0x010201, 0x2A5838
	, 0x45B86A, 0x39C86A, 0x3AC96B, 0x3AC569
	, 0x3AC76A, 0x38C467, 0x43D176, 0x33C161
	, 0x007501, 0x5C8D54, 0xDACDD5, 0xC9C3C3
	, 0xBEBDB8, 0xCCC8C5, 0xC4C1BE, 0x888785
	, 0x10100F, 0x202020, 0x39985C, 0x60FF9E
	, 0x52F28C, 0x53F48E, 0x53F38E, 0x51F08A
	, 0x5FFF9F, 0x4BEB81, 0x008601, 0x77B071
	, 0xFFFFFF, 0xFAFAFB, 0xFDFCFF, 0xF7F7F6
	, 0x707072, 0x191919, 0x2F2F2F, 0x333333
	, 0x218E49, 0x38EA7A, 0x31C768, 0x31CB69
	, 0x31CA69, 0x30C866, 0x3AD476, 0x2CC55F
	, 0x007C00, 0x669D62, 0xE7E1EB, 0xDAD9DF
	, 0xD2D4D6, 0xD2D3D7, 0xD9D9DD, 0xECECEF
	, 0x8C8C8F, 0x444444, 0x838383, 0x393938
	, 0x010301, 0x0D7B34, 0x15D256, 0x13B44B
	, 0x13B74B, 0x13B64B, 0x12B549, 0x17BE55
	, 0x0EB344, 0x5D9357, 0xD1C6D5, 0xC1BFC3
	, 0xBABCBC, 0xBCBBBD, 0xBEBEC0, 0xCBCBCD
	, 0x7C7C7D, 0x6F6F70, 0x787879, 0x040404
	, 0x06772C, 0x0AC849, 0x08AC3F, 0x08AF40
	, 0x08AE40, 0x08AD3F, 0x0CB449, 0x05AB3A
	, 0x588E52, 0xC2B9C4, 0xB6B3B6, 0xAFB1AF
	, 0xB0B0B0, 0xB3B3B3, 0xBEBEBF, 0x797979
	, 0x5F5F5F, 0xB3B3B4, 0x575758, 0x0C782F
	, 0x14C950, 0x11AD45, 0x10B046, 0x10AE43
	, 0x15B54F, 0x0DAC3F, 0x007D00, 0x538B4E
	, 0xBBB1BC, 0xAFACAE, 0xA8AAA8, 0xAAAAAA
	, 0xACABAC, 0xB6B6B6, 0x767675, 0x60605F
	, 0xB0B0B1, 0x555656, 0x238142, 0x3ADB70
	, 0x32BD61, 0x32C062, 0x32BF62, 0x31BE5F
	, 0x3AC76E, 0x2DBA59, 0x007E00, 0x599354
	, 0xD2C6D3, 0xC2C0C2, 0xBABDBA, 0xBCBDBC
	, 0xBFBFBF, 0xCBCCCB, 0x7D7D7C, 0x6F6E6E
	, 0xEBE9EB, 0x777677, 0x020202, 0x40935C
	, 0x6CF39B, 0x5ED085, 0x60D387, 0x60D487
	, 0x5DD183, 0x6CDE97, 0x57CD7C, 0x007D02
	, 0x67A262, 0xF6EFF7, 0xE7E5E7, 0xDEE1DE
	, 0xDFE0DF, 0xE4E5E5, 0xF8F9F8, 0x919292
	, 0x4F4E4E, 0xA5A29F, 0x4D4B49, 0x649D77
	, 0xA8FFCB, 0x8FF4B0, 0x91F6B3, 0x91F7B3
	, 0x8DF2AE, 0xA1FFC7, 0x84EEA5, 0x028508
	, 0x6DAC69, 0xFFFCFE, 0xF5F5F4, 0xF7F6F6
	, 0xFCFBFB, 0xF9F9F9, 0x777777, 0x0A0A0B
	, 0x1C1D1D, 0x6E636A, 0xB3D2BD, 0x90E8AE
	, 0x93E8AF, 0x94E3AE, 0x94E6AF, 0x8FE1AA
	, 0xA3EBC2, 0x88DEA2, 0x067D0C, 0x579153
	, 0xE6D8E5, 0xD3CED1, 0xC8C8C6, 0xD4D2D2
	, 0xCDCBCB, 0x939191, 0x161615, 0x1E1E1E
	, 0x010401, 0x010501, 0x010701, 0x010601
	, 0x010B01, 0x011F01, 0x011301, 0x010001
	, 0x090909
};

static GUI_CONST_STORAGE GUI_LOGPALETTE PalBATTERY4 =
{
	209,	/* number of entries */
	1, 	/* Has transparency */
	&ColorsBATTERY4[0]
};

static GUI_CONST_STORAGE unsigned char acBATTERY4[] =
{
	0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x04, 0x00, 0x00, 0x00,
	0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x00, 0x00, 0x00,
	0x05, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1D, 0x1D, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x24, 0x25, 0x26, 0x24, 0x27, 0x28, 0x01, 0x29, 0x2A, 0x2B,
	0x06, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x2F, 0x2F, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x01,
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x44, 0x44, 0x46, 0x47, 0x48, 0x34, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, 0x50, 0x51, 0x3B, 0x52, 0x53,
	0x06, 0x54, 0x55, 0x56, 0x57, 0x58, 0x57, 0x57, 0x59, 0x5A, 0x5B, 0x34, 0x5C, 0x5D, 0x5E, 0x5F, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x01,
	0x06, 0x67, 0x68, 0x69, 0x6A, 0x6A, 0x6A, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x01,
	0x40, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7D, 0x7D, 0x7F, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E,
	0x06, 0x8F, 0x90, 0x91, 0x92, 0x92, 0x93, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, 0xA0, 0xA1, 0xA2, 0x01,
	0x05, 0xA3, 0xA4, 0xA5, 0xA6, 0xA6, 0xA7, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0x24, 0xAD, 0xAE, 0xAF, 0xB0, 0xB1, 0xB2, 0x01, 0xB3, 0xB4, 0x19,
	0x06, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0x00, 0x00, 0x00,
	0x8E, 0x05, 0x06, 0x40, 0x40, 0xC8, 0xC9, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0x05, 0x06, 0x05, 0x8E, 0xD0, 0x01, 0x04, 0x00, 0x00, 0x00
};

GUI_CONST_STORAGE GUI_BITMAP bmBATTERY4 =
{
	24, /* XSize */
	12, /* YSize */
	24, /* BytesPerLine */
	8, /* BitsPerPixel */
	acBATTERY4,  /* Pointer to picture data (indices) */
	&PalBATTERY4  /* Pointer to palette */
};

/* *** End of file *** */
