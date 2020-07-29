#include "GUI_FontIntern.h"
#include <stddef.h>           /* needed for definition of NULL */
#include "GUI_Protected.h"
#include "GenericTypeDefs.h"
#include "spi_flash.h"


static int _GetLineDistX(const char GUI_UNI_PTR *s, int Len)
{
	int Dist = 0;
	unsigned short *ps = (unsigned short *)s;
	if (s)
	{
		while((*ps != 0) && Len >= 0)
		{
			Dist += GUI_GetCharDistX(*ps);	/*按小端存储的UNICODE码*/
			ps++;
			Len -= 2;
		}
	}
	return Dist;
}

static int _GetLineLen(const char GUI_UNI_PTR *s, int MaxLen)
{
	int Len = 0;
	unsigned short *ps = (unsigned short *)s;
	while ((*ps != 0) && Len < MaxLen)
	{
		if(*ps == '\n') return Len;
		ps++;
		Len += 2;
	}
	return Len;
}

static void _DispLine(const char GUI_UNI_PTR *s, int Len)
{
	unsigned short *ps = (unsigned short *)s;
	while (--Len >= 0)
	{
		GL_DispChar(*ps);				/*按小端存储的UNICODE码*/
		ps++;
		Len -= 2;
	}
}

static const tGUI_ENC_APIList GUI_ENC_APIList =
{
	_GetLineDistX,
	_GetLineLen,
	_DispLine
};

/*************************** End of file ****************************/



unsigned char acUnicode_En[16];/*从FLASH中取得的字体矩阵*/
unsigned char acUnicode_Cn[32];/*从FLASH中取得的字体矩阵*/
unsigned char aUnicode_Width = 0;

void Extern_FLASH_Read_Unicode_FontImage(U16 ch)
{
	char i;
	unsigned short sum = 0;
	Spi_Flash_Init();
	Flash_Read_Array(ch * 32, acUnicode_Cn, 32);
	Spi_Flash_DeInit();

	/*对奇数列进行判断*/
	for(i = 1; i < 32; i += 2)
		sum += acUnicode_Cn[i];

	/*对单行的数据采用半宽的方法显示*/
	if(sum == 0)
	{
		for(i = 0; i < 32; i += 2)
			acUnicode_En[i / 2] = acUnicode_Cn[i];
		aUnicode_Width = 0;
		return;
	}
	aUnicode_Width = 1;
}




GUI_CONST_STORAGE GUI_CHARINFO Font16_Unicode_CharInfo[2] =
{
	{8,     8,    1, acUnicode_En},
	{16,   16,  2, acUnicode_Cn}
};

GUI_CONST_STORAGE GUI_FONT_PROP GUI_Font16_Unicode_Prop =
{
	0x0000,                                         			/* first character               */
	0xFFFF,                                         			/* last character                */
	Font16_Unicode_CharInfo,                           /* address of first character    */
	0
};


const GUI_FONT GUI_Font16_Unicode =
{
	GUIPROP_DispChar,
	GUIPROP_GetCharDistX,
	GUIPROP_GetFontInfo,
	GUIPROP_IsInFont,
	&GUI_ENC_APIList /* type of font    */
	, 16 	/* height of font  */
	, 17 	/* space of font y */
	, 1 	/* magnification x */
	, 1 	/* magnification y */
	, {&GUI_Font16_Unicode_Prop}
};

const GUI_FONT GUI_Font16_UTF8 =
{
	GUI_FONTTYPE_PROP  /* type of font    */
	, 16                /* height of font  */
	, 18                /* space of font y */
	, 1                 /* magnification x */
	, 1                 /* magnification y */
	, {(const GUI_FONT_PROP GUI_UNI_PTR *)&GUI_Font16_Unicode_Prop}
};

const GUI_FONT GUI_Font16_UTF8X2 =
{
	GUI_FONTTYPE_PROP  /* type of font    */
	, 16                /* height of font  */
	, 19                /* space of font y */
	, 2                 /* magnification x */
	, 2                 /* magnification y */
	, {(const GUI_FONT_PROP GUI_UNI_PTR *)&GUI_Font16_Unicode_Prop}
};

const GUI_FONT GUI_Font16_UTF8X3 =
{
	GUI_FONTTYPE_PROP  /* type of font    */
	, 16                /* height of font  */
	, 20                /* space of font y */
	, 3                 /* magnification x */
	, 3                 /* magnification y */
	, {(const GUI_FONT_PROP GUI_UNI_PTR *)&GUI_Font16_Unicode_Prop}
};

