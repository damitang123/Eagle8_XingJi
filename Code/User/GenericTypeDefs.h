/*******************************************************************

                  Generic Type Definitions

********************************************************************
    FileName:        GenericTypeDefs.h
    Dependencies:    None
    Processor:       PIC10, PIC12, PIC16, PIC18, PIC24, dsPIC, PIC32
    Compiler:        MPLAB C Compilers for PIC18, PIC24, dsPIC, & PIC32
                  Hi-Tech PICC PRO, Hi-Tech PICC18 PRO
    Company:         Microchip Technology Inc.

    Software License Agreement

    The software supplied herewith by Microchip Technology Incorporated
    (the "Company") is intended and supplied to you, the Company's
    customer, for use solely and exclusively with products manufactured
    by the Company.

    The software is owned by the Company and/or its supplier, and is
    protected under applicable copyright laws. All rights are reserved.
    Any use in violation of the foregoing restrictions may subject the
    user to criminal sanctions under applicable laws, as well as to
    civil liability for the breach of the terms and conditions of this
    license.

    THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
    WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
    TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
    IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

********************************************************************
    File Description:

    Change History:
    Rev   Date         Description
    1.1   09/11/06     Add base signed types
    1.2   02/28/07     Add QWORD, LONGLONG, QWORD_VAL
    1.3   02/06/08     Add def's for PIC32
    1.4   08/08/08     Remove LSB/MSB Macros, adopted by Peripheral lib
    1.5   08/14/08     Simplify file header
    2.0   07/13/09     Updated for new release of coding standards
    3.1   05/28/10     MiWi DE 3.1
*******************************************************************/

#ifndef __GENERIC_TYPE_DEFS_H_
#define __GENERIC_TYPE_DEFS_H_


/* get compiler defined type definitions (NULL, size_t, etc) */
#include <stddef.h>
#include "stm32f10x.h"

#define	__PIC32MX__

#define PUBLIC                                  /* Function attributes */
#define PROTECTED
#define PRIVATE   static

/* INT is processor specific in length may vary in size */
typedef signed int          INT;
typedef signed char         INT8;
typedef signed short int    INT16;
typedef signed long int     INT32;

/* MPLAB C Compiler for PIC18 does not support 64-bit integers */
#if !defined(__18CXX)
	typedef signed long long    INT64;
#endif

/* UINT is processor specific in length may vary in size */
typedef unsigned int        UINT;
typedef unsigned char       UINT8;
typedef unsigned short int  UINT16;
/* 24-bit type only available on C18 */

typedef unsigned long int   UINT32;     /* other name for 32-bit integer */
/* MPLAB C Compiler for PIC18 does not support 64-bit integers */
#if !defined(__18CXX)
	typedef unsigned long long  UINT64;
#endif

typedef union
{
	UINT8 Val;
	struct
	{
		UINT8 b0: 1;
		UINT8 b1: 1;
		UINT8 b2: 1;
		UINT8 b3: 1;
		UINT8 b4: 1;
		UINT8 b5: 1;
		UINT8 b6: 1;
		UINT8 b7: 1;
	} bits;
} UINT8_VAL, UINT8_BITS;

typedef union
{
	UINT16 Val;
	UINT8 v[2] ;
	struct
	{
		UINT8 LB;
		UINT8 HB;
	} byte;
	struct
	{
		UINT8 b0: 1;
		UINT8 b1: 1;
		UINT8 b2: 1;
		UINT8 b3: 1;
		UINT8 b4: 1;
		UINT8 b5: 1;
		UINT8 b6: 1;
		UINT8 b7: 1;
		UINT8 b8: 1;
		UINT8 b9: 1;
		UINT8 b10: 1;
		UINT8 b11: 1;
		UINT8 b12: 1;
		UINT8 b13: 1;
		UINT8 b14: 1;
		UINT8 b15: 1;
	} bits;
} UINT16_VAL, UINT16_BITS;

typedef union
{
	UINT32 Val;
	UINT16 w[2] ;
	UINT8  v[4] ;
	struct
	{
		UINT16 LW;
		UINT16 HW;
	} word;
	struct
	{
		UINT8 LB;
		UINT8 HB;
		UINT8 UB;
		UINT8 MB;
	} byte;
	struct
	{
		UINT16_VAL low;
		UINT16_VAL high;
	} wordUnion;
	struct
	{
		UINT8 b0: 1;
		UINT8 b1: 1;
		UINT8 b2: 1;
		UINT8 b3: 1;
		UINT8 b4: 1;
		UINT8 b5: 1;
		UINT8 b6: 1;
		UINT8 b7: 1;
		UINT8 b8: 1;
		UINT8 b9: 1;
		UINT8 b10: 1;
		UINT8 b11: 1;
		UINT8 b12: 1;
		UINT8 b13: 1;
		UINT8 b14: 1;
		UINT8 b15: 1;
		UINT8 b16: 1;
		UINT8 b17: 1;
		UINT8 b18: 1;
		UINT8 b19: 1;
		UINT8 b20: 1;
		UINT8 b21: 1;
		UINT8 b22: 1;
		UINT8 b23: 1;
		UINT8 b24: 1;
		UINT8 b25: 1;
		UINT8 b26: 1;
		UINT8 b27: 1;
		UINT8 b28: 1;
		UINT8 b29: 1;
		UINT8 b30: 1;
		UINT8 b31: 1;
	} bits;
} UINT32_VAL;

/* MPLAB C Compiler for PIC18 does not support 64-bit integers */
#if !defined(__18CXX)
typedef union
{
	UINT64 Val;
	UINT32 d[2] ;
	UINT16 w[4] ;
	UINT8 v[8]  ;
	struct
	{
		UINT32 LD;
		UINT32 HD;
	} dword;
	struct
	{
		UINT16 LW;
		UINT16 HW;
		UINT16 UW;
		UINT16 MW;
	} word;
	struct
	{
		UINT8 b0: 1;
		UINT8 b1: 1;
		UINT8 b2: 1;
		UINT8 b3: 1;
		UINT8 b4: 1;
		UINT8 b5: 1;
		UINT8 b6: 1;
		UINT8 b7: 1;
		UINT8 b8: 1;
		UINT8 b9: 1;
		UINT8 b10: 1;
		UINT8 b11: 1;
		UINT8 b12: 1;
		UINT8 b13: 1;
		UINT8 b14: 1;
		UINT8 b15: 1;
		UINT8 b16: 1;
		UINT8 b17: 1;
		UINT8 b18: 1;
		UINT8 b19: 1;
		UINT8 b20: 1;
		UINT8 b21: 1;
		UINT8 b22: 1;
		UINT8 b23: 1;
		UINT8 b24: 1;
		UINT8 b25: 1;
		UINT8 b26: 1;
		UINT8 b27: 1;
		UINT8 b28: 1;
		UINT8 b29: 1;
		UINT8 b30: 1;
		UINT8 b31: 1;
		UINT8 b32: 1;
		UINT8 b33: 1;
		UINT8 b34: 1;
		UINT8 b35: 1;
		UINT8 b36: 1;
		UINT8 b37: 1;
		UINT8 b38: 1;
		UINT8 b39: 1;
		UINT8 b40: 1;
		UINT8 b41: 1;
		UINT8 b42: 1;
		UINT8 b43: 1;
		UINT8 b44: 1;
		UINT8 b45: 1;
		UINT8 b46: 1;
		UINT8 b47: 1;
		UINT8 b48: 1;
		UINT8 b49: 1;
		UINT8 b50: 1;
		UINT8 b51: 1;
		UINT8 b52: 1;
		UINT8 b53: 1;
		UINT8 b54: 1;
		UINT8 b55: 1;
		UINT8 b56: 1;
		UINT8 b57: 1;
		UINT8 b58: 1;
		UINT8 b59: 1;
		UINT8 b60: 1;
		UINT8 b61: 1;
		UINT8 b62: 1;
		UINT8 b63: 1;
	} bits;
} UINT64_VAL;
#endif /* __18CXX */

/***********************************************************************************/

/* Alternate definitions */
typedef void                    		VOID;

typedef char                    		CHAR8;
typedef unsigned char           	UCHAR8;

typedef unsigned char           	BYTE;                           	/* 8-bit unsigned  */
typedef unsigned short int      	WORD;                           	/* 16-bit unsigned */
typedef unsigned long           	DWORD;                         	 /* 32-bit unsigned */
/* MPLAB C Compiler for PIC18 does not support 64-bit integers */

typedef unsigned long long      	QWORD;                          	/* 64-bit unsigned */
typedef signed char             	CHAR;                          	 /* 8-bit signed    */
typedef signed short int        	SHORT;                         	 /* 16-bit signed   */
typedef signed long             	LONG;                           	/* 32-bit signed   */

/* MPLAB C Compiler for PIC18 does not support 64-bit integers */
typedef signed long long        LONGLONG;                       /* 64-bit signed   */

typedef union
{
	BYTE Val;
	struct
	{
		BYTE b0: 1;
		BYTE b1: 1;
		BYTE b2: 1;
		BYTE b3: 1;
		BYTE b4: 1;
		BYTE b5: 1;
		BYTE b6: 1;
		BYTE b7: 1;
	} bits;
} BYTE_VAL, BYTE_BITS;

typedef union
{
	WORD Val;
	BYTE v[2] ;
	struct
	{
		BYTE LB;
		BYTE HB;
	} byte;
	struct
	{
		BYTE b0: 1;
		BYTE b1: 1;
		BYTE b2: 1;
		BYTE b3: 1;
		BYTE b4: 1;
		BYTE b5: 1;
		BYTE b6: 1;
		BYTE b7: 1;
		BYTE b8: 1;
		BYTE b9: 1;
		BYTE b10: 1;
		BYTE b11: 1;
		BYTE b12: 1;
		BYTE b13: 1;
		BYTE b14: 1;
		BYTE b15: 1;
	} bits;
} WORD_VAL, WORD_BITS;

typedef union
{
	DWORD Val;
	WORD w[2] ;
	BYTE v[4] ;
	struct
	{
		WORD LW;
		WORD HW;
	} word;
	struct
	{
		BYTE LB;
		BYTE HB;
		BYTE UB;
		BYTE MB;
	} byte;
	struct
	{
		WORD_VAL low;
		WORD_VAL high;
	} wordUnion;
	struct
	{
		BYTE b0: 1;
		BYTE b1: 1;
		BYTE b2: 1;
		BYTE b3: 1;
		BYTE b4: 1;
		BYTE b5: 1;
		BYTE b6: 1;
		BYTE b7: 1;
		BYTE b8: 1;
		BYTE b9: 1;
		BYTE b10: 1;
		BYTE b11: 1;
		BYTE b12: 1;
		BYTE b13: 1;
		BYTE b14: 1;
		BYTE b15: 1;
		BYTE b16: 1;
		BYTE b17: 1;
		BYTE b18: 1;
		BYTE b19: 1;
		BYTE b20: 1;
		BYTE b21: 1;
		BYTE b22: 1;
		BYTE b23: 1;
		BYTE b24: 1;
		BYTE b25: 1;
		BYTE b26: 1;
		BYTE b27: 1;
		BYTE b28: 1;
		BYTE b29: 1;
		BYTE b30: 1;
		BYTE b31: 1;
	} bits;
} DWORD_VAL;

/* MPLAB C Compiler for PIC18 does not support 64-bit integers */
typedef union
{
	QWORD Val;
	DWORD d[2] ;
	WORD w[4] ;
	BYTE v[8] ;
	struct
	{
		DWORD LD;
		DWORD HD;
	} dword;
	struct
	{
		WORD LW;
		WORD HW;
		WORD UW;
		WORD MW;
	} word;
	struct
	{
		BYTE b0: 1;
		BYTE b1: 1;
		BYTE b2: 1;
		BYTE b3: 1;
		BYTE b4: 1;
		BYTE b5: 1;
		BYTE b6: 1;
		BYTE b7: 1;
		BYTE b8: 1;
		BYTE b9: 1;
		BYTE b10: 1;
		BYTE b11: 1;
		BYTE b12: 1;
		BYTE b13: 1;
		BYTE b14: 1;
		BYTE b15: 1;
		BYTE b16: 1;
		BYTE b17: 1;
		BYTE b18: 1;
		BYTE b19: 1;
		BYTE b20: 1;
		BYTE b21: 1;
		BYTE b22: 1;
		BYTE b23: 1;
		BYTE b24: 1;
		BYTE b25: 1;
		BYTE b26: 1;
		BYTE b27: 1;
		BYTE b28: 1;
		BYTE b29: 1;
		BYTE b30: 1;
		BYTE b31: 1;
		BYTE b32: 1;
		BYTE b33: 1;
		BYTE b34: 1;
		BYTE b35: 1;
		BYTE b36: 1;
		BYTE b37: 1;
		BYTE b38: 1;
		BYTE b39: 1;
		BYTE b40: 1;
		BYTE b41: 1;
		BYTE b42: 1;
		BYTE b43: 1;
		BYTE b44: 1;
		BYTE b45: 1;
		BYTE b46: 1;
		BYTE b47: 1;
		BYTE b48: 1;
		BYTE b49: 1;
		BYTE b50: 1;
		BYTE b51: 1;
		BYTE b52: 1;
		BYTE b53: 1;
		BYTE b54: 1;
		BYTE b55: 1;
		BYTE b56: 1;
		BYTE b57: 1;
		BYTE b58: 1;
		BYTE b59: 1;
		BYTE b60: 1;
		BYTE b61: 1;
		BYTE b62: 1;
		BYTE b63: 1;
	} bits;
} QWORD_VAL;

#endif /* __GENERIC_TYPE_DEFS_H_ */
