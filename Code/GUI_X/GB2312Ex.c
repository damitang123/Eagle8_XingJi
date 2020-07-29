#include "GUI_FontIntern.h"
#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"
#include "GenericTypeDefs.h"
#include "spi_flash.h"


static U16 DB2S_GB2312(U8 Byte0, U8 Byte1)
{
	return Byte1 | (((U16)Byte0) << 8);
}


static int _GetLineDistX_GB2312(const char GUI_UNI_PTR *s, int Len)
{
	int Dist = 0;
	if (s)
	{
		U8 c0;
		while (((c0 = *(const U8 *)s) != 0) && Len >= 0)
		{
			s++;
			Len--;
			if (c0 > 127)
			{
				U8  c1 = *(const U8 *)s++;
				Len--;
				Dist += GUI_GetCharDistX(DB2S_GB2312(c0, c1));
			}
			else
			{
				Dist += GUI_GetCharDistX(c0);
			}
		}
	}
	return Dist;
}

static int _GetLineLen_GB2312(const char GUI_UNI_PTR *s, int MaxLen)
{
	int Len = 0;
	U8 c0;
	while (((c0 = *(const U8 *)s) != 0) && Len < MaxLen)
	{
		s++;
		if (c0 > 127)
		{
			Len++;
			s++;
		}
		else
		{
			switch (c0)
			{
				case '\n':
					return Len;
			}
		}
		Len++;
	}
	return Len;
}

static void _DispLine_GB2312(const char GUI_UNI_PTR *s, int Len)
{
	U8 c0;
	while (--Len >= 0)
	{
		c0 = *(const U8 *)s++;
		if (c0 > 127)
		{
			U8  c1 = *(const U8 *)s++;
			Len--;
			GL_DispChar (DB2S_GB2312(c0, c1));
		}
		else
		{
			GL_DispChar(c0);
		}
	}
}

const tGUI_ENC_APIList GUI_ENC_APIList_GB2312 =
{
	_GetLineDistX_GB2312,
	_GetLineLen_GB2312,
	_DispLine_GB2312
};

/*************************** End of file ****************************/



unsigned char acGB2132_En[16];/*从FLASH中取得的字体矩阵*/
unsigned char acGB2132_Cn[32];/*从FLASH中取得的字体矩阵*/

void Extern_FLASH_Read_GBK_FontImage(U16 ch)
{
	DWORD_VAL     glyphOffset;
	if(ch < 0x1000)
	{
		//	imageSize = 16;	/*字体矩阵大小*/
		glyphOffset.Val = ch * 16L + 1048576L;	/*字体矩阵相对基地址的偏移量*/
		//	pParam->chGlyphWidth = 0x08;/*字体宽度---注意此时是定宽*/
		Spi_Flash_Init();
		Flash_Read_Array_For_Ex_Font(glyphOffset.Val, acGB2132_En, 16);
		Spi_Flash_DeInit();
	}
	else if((ch >> 8) >= 0x81)
	{
		//	imageSize = 32;
		glyphOffset.Val = (((ch >> 8) - 129) * 190 + (ch & 0xFF) - 64 - ((ch & 0xFF) >> 7)) * 32L + 65536L + 1048576L;
		//	pParam->chGlyphWidth = 0x10;
		Spi_Flash_Init();
		Flash_Read_Array_For_Ex_Font(glyphOffset.Val, acGB2132_Cn, 32);
		Spi_Flash_DeInit();
	}



}




GUI_CONST_STORAGE GUI_CHARINFO Font16_GB2312_CharInfo[2] =
{
	{8,     8,    1, acGB2132_En},
	{16,   16,  2, acGB2132_Cn}
};

GUI_CONST_STORAGE GUI_FONT_PROP GUI_Font16_GB2312_Prop =
{
	0xA1A1,                                         			/* first character               */
	0xFEFE,                                         			/* last character                */
	Font16_GB2312_CharInfo,                           /* address of first character    */
	0
};


const GUI_FONT GUI_Font16_GB2312 =
{
	GUIPROP_DispChar,
	GUIPROP_GetCharDistX,
	GUIPROP_GetFontInfo,
	GUIPROP_IsInFont,
	&GUI_ENC_APIList_GB2312 /* type of font    */
	, 16 	/* height of font  */
	, 16 	/* space of font y */
	, 1 	/* magnification x */
	, 1 	/* magnification y */
	, {&GUI_Font16_GB2312_Prop}
};

const GUI_FONT GUI_Font16_GB2312_2SIZE =
{
	GUIPROP_DispChar,
	GUIPROP_GetCharDistX,
	GUIPROP_GetFontInfo,
	GUIPROP_IsInFont,
	&GUI_ENC_APIList_GB2312 /* type of font    */
	, 16 	/* height of font  */
	, 16 	/* space of font y */
	, 2 	/* magnification x */
	, 2 	/* magnification y */
	, {&GUI_Font16_GB2312_Prop}
};


GUI_FONT GUI_Font_FLASH =
{
	GUIPROP_DispChar,
	GUIPROP_GetCharDistX,
	GUIPROP_GetFontInfo,
	GUIPROP_IsInFont,
	&GUI_ENC_APIList_GB2312 /* type of font    */
	, 16 	/* height of font  */
	, 16 	/* space of font y */
	, 1 	/* magnification x */
	, 1 	/* magnification y */
	, {&GUI_Font16_GB2312_Prop}
};
