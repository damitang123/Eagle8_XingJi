#include "platform.h"
#include "spi_flash.h"
#include "delay.h"
#include "Windows.h"
#include "app.h"
#include "spi_flash.h"
#include "spi_hw_driver.h"
#include "system_misc.h"


#define FLASHCSLow()    			GPIOA->BRR	= GPIO_Pin_8
#define FLASHCSHigh()   			GPIOA ->BSRR= GPIO_Pin_8


static void Spi_Flash_CS_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	FLASHCSHigh();
}

void Spi_Flash_Init(void)
{
	static bool bIsInit = false;

	if(bIsInit)
	{
		return;
	}

	bIsInit = true;

	Spi_Flash_CS_Init();
	
	Spi_Hw_Init();

	Flash_Check_Type();
}

void Spi_Flash_DeInit(void)
{

}

static void Flash_Send_Byte(unsigned char nByte)
{
	Spi_Hw_Send_Single_Byte(nByte);
}

static unsigned char Flash_Get_Byte(void)
{
	return Spi_Hw_Get_Single_Byte();
}


/***************************************************************************************/
static sFLashTypeTable s_sCurrentFlashProperty;

static const sFLashTypeTable g_sFLashTypeTable[] =
{
	'W', "W25Q64F", 0xEF, 0x16, 2047,			/*Winbond 64Mb*/
	'W', "W25Q128F", 0xEF, 0x17, 4095,			/*Winbond 128Mb*/
	'S', "SST25VF032B", 0xBF, 0x4A, 1023,		/*SST25VF032B*/
	'S', "SST25VF016B", 0xBF, 0x41, 511,			/*SST25VF016B*/
	'G', "GD25Q64C", 0xC8, 0x16, 2047,			/*GD25Q64C*/
	'G', "GD25Q128C", 0xC8, 0x17, 4095,			/*GD25Q128C*/
};

void Flash_Check_Type(void)
{
	static bool bAreadyCheckFlashType = false;
	
	if(bAreadyCheckFlashType == false)
	{
		unsigned char nMFID, nDeviceID, nSize, i;
		nSize = sizeof(g_sFLashTypeTable) / sizeof(g_sFLashTypeTable[0]);
		FLASH_Get_ID(0,&nMFID, &nDeviceID);
		for(i = 0; i < nSize; i ++)
		{
			if(nMFID == g_sFLashTypeTable[i].nFlashMFID && nDeviceID == g_sFLashTypeTable[i].nFlashDeviceID)
			{
				bAreadyCheckFlashType = true;
				s_sCurrentFlashProperty.nFlashMFID = nMFID;
				s_sCurrentFlashProperty.nFlashDeviceID = nDeviceID;
				s_sCurrentFlashProperty.nFlashType = g_sFLashTypeTable[i].nFlashType;
				s_sCurrentFlashProperty.nFlashMaxSector = g_sFLashTypeTable[i].nFlashMaxSector;
				if(s_sCurrentFlashProperty.nFlashType == 'S')
				{
					Flash_Unprotect();
				}
				TRACE_PRINTF("Spi Flash Type: %s\r\n", g_sFLashTypeTable[i].strFlashName);
				return;
			}
		}

		TRACE_PRINTF("Spi Flash Type: %s!\r\n", "Unknow");
		s_sCurrentFlashProperty.nFlashMaxSector = 511;
	}
}

unsigned short Flash_Get_Max_Sector(void)
{
	return s_sCurrentFlashProperty.nFlashMaxSector;
}

static bool Flash_WaitTimeOut(unsigned long nWaitTime)
{
	unsigned char nTemp;
	unsigned long  nStartWaitTime = Get_System_Time();
	do
	{
		FLASHCSLow();
		Flash_Send_Byte(FLASH_CMD_RDSR);

		nTemp = Flash_Get_Byte();
		FLASHCSHigh();

		if((nTemp & 0x01) == 0)
		{
			return false;
		}
	}
	while(Get_System_Time() < (nWaitTime + nStartWaitTime));

	return true;
}



static void Flash_Write_Enable(void)
{
	FLASHCSLow();
	Flash_Send_Byte(FLASH_CMD_WREN);
	FLASHCSHigh();
}

bool Flash_Write_Byte(unsigned long nAddr, unsigned char data)
{
	unsigned char *pAddr;
	pAddr = (unsigned char *)&nAddr;

	Flash_Write_Enable();

	FLASHCSLow();
	Flash_Send_Byte(FLASH_CMD_WRITE);
	Flash_Send_Byte(*(pAddr + 2));
	Flash_Send_Byte(*(pAddr + 1));
	Flash_Send_Byte(*(pAddr + 0));
	Flash_Send_Byte(data);
	FLASHCSHigh();

	return Flash_WaitTimeOut(500) ? false : true;
}

unsigned char Flash_Read_Byte(unsigned long nAddr)
{
	unsigned char *pAddr, nTemp;
	pAddr = (unsigned char *)&nAddr;

	FLASHCSLow();

	Flash_Send_Byte(FLASH_CMD_READ);
	Flash_Send_Byte(*(pAddr + 2));
	Flash_Send_Byte(*(pAddr + 1));
	Flash_Send_Byte(*(pAddr + 0));
	nTemp = Flash_Get_Byte();

	FLASHCSHigh();

	return nTemp;
}

bool Flash_Write_Array(unsigned long nAddr, unsigned char  *pData, unsigned short nCount)
{
	unsigned short i;

	if(nCount == 0) return true;
	
	for(i = 0; i < nCount; i++)
	{
		if(Flash_Write_Byte(nAddr++, *pData++) == FALSE)
		{
			return FALSE;
		}
	}

	return true;
}

void Flash_Read_Array(unsigned long nAddr, unsigned char *pData, unsigned short nCount)
{
	unsigned char *pAddr;
	pAddr = (unsigned char *)&nAddr;

	FLASHCSLow();

	Flash_Send_Byte(FLASH_CMD_READ);
	Flash_Send_Byte(*(pAddr + 2));
	Flash_Send_Byte(*(pAddr + 1));
	Flash_Send_Byte(*(pAddr + 0));
	while(nCount--)
	{
		*pData++ = Flash_Get_Byte();
	}

	FLASHCSHigh();
}

bool Flash_Content_Copy(unsigned long nAddrSrc,unsigned long nAddrDest,unsigned long nSize)
{
	unsigned long i;
	unsigned char nTemp;

	if(nSize == 0) return true;
	
	for(i = 0; i< nSize; i++)
	{
		nTemp = Flash_Read_Byte(nAddrSrc++);
		if(Flash_Write_Byte(nAddrDest++, nTemp) == false)
		{
			return false;
		}
	}
	return true;
}

bool Flash_Modify_Content_Any_Addr_Any_Size(unsigned long nAddr, unsigned char  *pData, unsigned short nSize,unsigned long nSwapAddr)
{
	unsigned long nAddrTempSrc,nAddrTempSwap;
	if(nSize > FLASH_SECTOR_SIZE)
	{
		return false;
	}

	nAddrTempSrc = (nAddr / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE;
	nAddrTempSwap = (nSwapAddr / FLASH_SECTOR_SIZE) * FLASH_SECTOR_SIZE;

	/*1.擦除交换扇区内容*/
	if(Flash_Sector_Erase(nAddrTempSwap) == false)
	{
		return false;
	}
	/*2.复制内容到交换扇区.*/
	if(Flash_Content_Copy(nAddrTempSrc,nSwapAddr,FLASH_SECTOR_SIZE) == false)
	{
		return false;
	}
	/*3.擦除源扇区内容.*/
	if(Flash_Sector_Erase(nAddrTempSrc) == false)
	{
		return false;
	}
	/*4.复制回源数据内容步骤1(扇区内容的前一部分)*/
	if(Flash_Content_Copy(nAddrTempSwap, nAddrTempSrc,nAddr - nAddrTempSrc) == false)
	{
		return false;
	}
	/*5.复制回源数据内容步骤2(扇区内容的后一部分)*/
	if(Flash_Content_Copy(nAddrTempSwap + (nAddr - nAddrTempSrc) + nSize, nAddrTempSrc + (nAddr - nAddrTempSrc) + nSize , FLASH_SECTOR_SIZE - (nAddr - nAddrTempSrc) - nSize) == false)
	{
		return false;
	}
	/*6.写入真正要修改的数据*/
	return Flash_Write_Array(nAddr, pData , nSize);
}

void FLASH_Get_ID(unsigned long nAddr,unsigned char *nMFID, unsigned char *nDeviceID)
{
	unsigned char *pAddr;
	pAddr = (unsigned char *)&nAddr;
	
	FLASHCSLow();

	Flash_Send_Byte(FLASH_CMD_RMFID);
	Flash_Send_Byte(*(pAddr + 2));
	Flash_Send_Byte(*(pAddr + 1));
	Flash_Send_Byte(*(pAddr + 0));

	*nMFID = Flash_Get_Byte();
	*nDeviceID = Flash_Get_Byte();

	FLASHCSHigh();
}

void Flash_Unprotect(void)
{
	FLASHCSLow();
	Flash_Send_Byte(FLASH_CMD_EWSR);
	FLASHCSHigh();

	FLASHCSLow();
	Flash_Send_Byte(FLASH_CMD_WRSR);
	Flash_Send_Byte(0);
	FLASHCSHigh();
}

bool Flash_Sector_Erase(unsigned long nAddr)
{
	unsigned char *pAddr;
	pAddr = (unsigned char *)&nAddr;

	Flash_Write_Enable();

	FLASHCSLow();
	Flash_Send_Byte(FLASH_CMD_SER);
	Flash_Send_Byte(*(pAddr + 2));
	Flash_Send_Byte(*(pAddr + 1));
	Flash_Send_Byte(*(pAddr + 0));
	FLASHCSHigh();

	return Flash_WaitTimeOut(500) ? false : true;
}

bool Flash_Chip_Erase(void)
{
	Flash_Write_Enable();

	FLASHCSLow();
	Flash_Send_Byte(FLASH_CMD_ERASE);
	FLASHCSHigh();

	return Flash_WaitTimeOut(500) ? false : true;
}

/*********************************************************************************************/

static const unsigned char EnFontLibDecode[256] =
{
	0x00, 0x08, 0x03, 0x02, 0x0C, 0x0B, 0x0E, 0x09, 0x01, 0x07, 0x0D, 0x05, 0x04, 0x0A, 0x06, 0x0F,
	0x80, 0x88, 0x83, 0x82, 0x8C, 0x8B, 0x8E, 0x89, 0x81, 0x87, 0x8D, 0x85, 0x84, 0x8A, 0x86, 0x8F,
	0x30, 0x38, 0x33, 0x32, 0x3C, 0x3B, 0x3E, 0x39, 0x31, 0x37, 0x3D, 0x35, 0x34, 0x3A, 0x36, 0x3F,
	0x20, 0x28, 0x23, 0x22, 0x2C, 0x2B, 0x2E, 0x29, 0x21, 0x27, 0x2D, 0x25, 0x24, 0x2A, 0x26, 0x2F,
	0xC0, 0xC8, 0xC3, 0xC2, 0xCC, 0xCB, 0xCE, 0xC9, 0xC1, 0xC7, 0xCD, 0xC5, 0xC4, 0xCA, 0xC6, 0xCF,
	0xB0, 0xB8, 0xB3, 0xB2, 0xBC, 0xBB, 0xBE, 0xB9, 0xB1, 0xB7, 0xBD, 0xB5, 0xB4, 0xBA, 0xB6, 0xBF,
	0xE0, 0xE8, 0xE3, 0xE2, 0xEC, 0xEB, 0xEE, 0xE9, 0xE1, 0xE7, 0xED, 0xE5, 0xE4, 0xEA, 0xE6, 0xEF,
	0x90, 0x98, 0x93, 0x92, 0x9C, 0x9B, 0x9E, 0x99, 0x91, 0x97, 0x9D, 0x95, 0x94, 0x9A, 0x96, 0x9F,
	0x10, 0x18, 0x13, 0x12, 0x1C, 0x1B, 0x1E, 0x19, 0x11, 0x17, 0x1D, 0x15, 0x14, 0x1A, 0x16, 0x1F,
	0x70, 0x78, 0x73, 0x72, 0x7C, 0x7B, 0x7E, 0x79, 0x71, 0x77, 0x7D, 0x75, 0x74, 0x7A, 0x76, 0x7F,
	0xD0, 0xD8, 0xD3, 0xD2, 0xDC, 0xDB, 0xDE, 0xD9, 0xD1, 0xD7, 0xDD, 0xD5, 0xD4, 0xDA, 0xD6, 0xDF,
	0x50, 0x58, 0x53, 0x52, 0x5C, 0x5B, 0x5E, 0x59, 0x51, 0x57, 0x5D, 0x55, 0x54, 0x5A, 0x56, 0x5F,
	0x40, 0x48, 0x43, 0x42, 0x4C, 0x4B, 0x4E, 0x49, 0x41, 0x47, 0x4D, 0x45, 0x44, 0x4A, 0x46, 0x4F,
	0xA0, 0xA8, 0xA3, 0xA2, 0xAC, 0xAB, 0xAE, 0xA9, 0xA1, 0xA7, 0xAD, 0xA5, 0xA4, 0xAA, 0xA6, 0xAF,
	0x60, 0x68, 0x63, 0x62, 0x6C, 0x6B, 0x6E, 0x69, 0x61, 0x67, 0x6D, 0x65, 0x64, 0x6A, 0x66, 0x6F,
	0xF0, 0xF8, 0xF3, 0xF2, 0xFC, 0xFB, 0xFE, 0xF9, 0xF1, 0xF7, 0xFD, 0xF5, 0xF4, 0xFA, 0xF6, 0xFF
};

#define FONT_DECODE(ADDR,DATA)	EnFontLibDecode[DATA]^ADDR

void Flash_Read_Array_For_Ex_Font(unsigned long nAddr, unsigned char *pData, unsigned short nCount)
{
	unsigned char *pAddr;

	pAddr = (unsigned char *)&nAddr;

	FLASHCSLow();

	Flash_Send_Byte(FLASH_CMD_READ);
	Flash_Send_Byte(*(pAddr + 2));
	Flash_Send_Byte(*(pAddr + 1));
	Flash_Send_Byte(*(pAddr + 0));

	while(nCount--)
	{
		*pData++ = FONT_DECODE(nAddr, Flash_Get_Byte());
		nAddr++;
	}

	FLASHCSHigh();
}




