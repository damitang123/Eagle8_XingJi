#include "platform.h"
#include "tjpgd.h"
#include "GUI.h"

void LCD_L0_FillRect_WithData(int x0, int y0, int x1, int y1,void *pData);

static unsigned char aJpeg_Buffer[3100];

typedef struct 
{
	int xOff,yOff;
	unsigned char *pJpgFile;
	unsigned long nFileOffset;
} IODEV;


UINT _in_func( JDEC* jdec,   BYTE* buff,  UINT nByte)
{	
	IODEV *pDev = (IODEV *)jdec->device;
	if(buff)
	{
		memcpy(buff,(void *)(pDev->pJpgFile + pDev->nFileOffset),nByte);
		pDev->nFileOffset += nByte;
		return nByte;
	}
	else
	{
		(pDev->nFileOffset) += nByte;
		return 0;
	}
}  

UINT _out_func(JDEC* jdec, void* bitmap, JRECT* rect)
{
	GUI_RECT r;
	IODEV *pDev = (IODEV *)jdec->device;
	
	r.x0 = rect->left;
	r.x1 = rect->right;
	r.y0 = rect->top;
	r.y1 = rect->bottom;

	r.x0 += pDev->xOff;
	r.x1 += pDev->xOff;
	r.y0 += pDev->yOff;
	r.y1 += pDev->yOff;

	LCD_L0_FillRect_WithData(r.x0,r.y0,r.x1, r.y1, bitmap);
	
	return 1;
}  

void Jpeg_Display(void* pJpgFile,int x, int y ,int nScale)
{
	JDEC jdec; 
	JRESULT  res;
	IODEV dev;
	dev.nFileOffset = 0;
	dev.xOff = x;
	dev.yOff = y;
	dev.pJpgFile = pJpgFile;
	res = jd_prepare(&jdec,_in_func,aJpeg_Buffer,sizeof(aJpeg_Buffer),&dev);
	if (res == JDR_OK) 
	{
		res = jd_decomp(&jdec, _out_func, nScale);
		if (res != JDR_OK)
		{
			TRACE_PRINTF("Failed to decompress: rc = %d\r\n", res);
		}
	} 
	else 
	{
		TRACE_PRINTF("Failed to prepare: rc = %d\r\n", res);
	}
}


