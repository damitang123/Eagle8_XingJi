#include "platform.h"
#include "spi_flash.h"
/*
	Spi_FLash_Load_User_Struct  函数说明:
	输入参数pUserStruct 的大小必须可以被一个扇区整除.(即可以被4096整除)
	输入参数的pUserStruct 最后一个元素必须是数据是否有效的标志.
	当无有效数据返回的时候返回FALSE(调用时应注意这种情况),数据有效的时候返回TRUE.
*/

#undef TRACE_PRINTF
#define TRACE_PRINTF(...)

typedef enum
{
	eFlashFlagFree = 0xFF,
	eFlashFlagValid = 0x9A,
	eFlaghFlagInvalid = 0x00,
} sFlashDataFlag;

static unsigned short Get_Suitable_BlockSize(unsigned short nSizeOfStruct)
{
	int nSize = 1;
	do
	{
		if(nSize >= nSizeOfStruct)
		{
			return nSize;
		}
		else
		{
			nSize <<= 1;
		}
	}
	while(nSize <= FLASH_SECTOR_SIZE);

	return FLASH_SECTOR_SIZE; 
}

bool Spi_FLash_Load_User_Struct(unsigned char *pUserStruct, unsigned short nSizeOfStruct , unsigned short nStartSector)
{
	unsigned short nOffset;
	unsigned long nStartAddr;
	sFlashDataFlag sFlag;
	unsigned short nBlockSize; 

	if(nSizeOfStruct > FLASH_SECTOR_SIZE)
	{
		TRACE_PRINTF("Size of data structures errors!\r\n");
		return false;
	}

	nBlockSize = Get_Suitable_BlockSize(nSizeOfStruct);

	nStartAddr = FLASH_SECTOR_SIZE * nStartSector;

	for(nOffset = 0 ; nOffset < FLASH_SECTOR_SIZE ; nOffset += nBlockSize)
	{
		Flash_Read_Array(nOffset + nStartAddr , (unsigned char * )pUserStruct, nSizeOfStruct);
		
		sFlag = (sFlashDataFlag)(pUserStruct[nSizeOfStruct - 1]);
		switch(sFlag)
		{
			case eFlashFlagFree:
				TRACE_PRINTF("There is no valid data(Blank Sector)!\r\n");
				return false;
			case eFlashFlagValid:
				TRACE_PRINTF("Load valid data, Offset : 0x%04X\r\n",(nOffset + nStartAddr) % FLASH_SECTOR_SIZE);
				return true;
		}
	}

	/*查找完整个扇区..无有效数据,则擦除整个扇区*/
	Flash_Sector_Erase(nStartAddr);

	TRACE_PRINTF("There is no valid data(Dirty Sector,need to be erased)!\r\n");
	
	return false;
}


void Spi_FLash_Save_User_Struct(unsigned char *pUserStruct, unsigned short nSizeOfStruct, unsigned short nStartSector)
{
	unsigned short nOffset;
	unsigned long nStartAddr;
	sFlashDataFlag sFlag;
	unsigned short nBlockSize; 

	if(nSizeOfStruct > FLASH_SECTOR_SIZE)
	{
		TRACE_PRINTF("Size of data structures errors!\r\n");
		return;
	}
	
	nBlockSize = Get_Suitable_BlockSize(nSizeOfStruct);

	nStartAddr = FLASH_SECTOR_SIZE * nStartSector;

	for(nOffset = 0 ; nOffset < FLASH_SECTOR_SIZE ; nOffset += nBlockSize)
	{
		/*仅读取FLAG标志.*/
		Flash_Read_Array(nOffset + nStartAddr +  nSizeOfStruct - 1, (unsigned char * )&sFlag, 1);
		switch(sFlag)
		{
			case eFlashFlagFree:
				pUserStruct[nSizeOfStruct - 1] = (unsigned char)eFlashFlagValid;
				Flash_Write_Array(nOffset + nStartAddr , (unsigned char * )pUserStruct, nSizeOfStruct);
				TRACE_PRINTF("Write valid data, Offset : 0x%04X\r\n",(nOffset + nStartAddr) % FLASH_SECTOR_SIZE);
				return;
			case eFlashFlagValid:
				pUserStruct[nSizeOfStruct - 1] = (unsigned char)eFlaghFlagInvalid;
				Flash_Write_Array(nOffset + nStartAddr , (unsigned char * )pUserStruct, nSizeOfStruct);
				TRACE_PRINTF("Mark invalid data, Offset : 0x%04X\r\n",(nOffset + nStartAddr) % FLASH_SECTOR_SIZE);
				break;
		}
	}

	/*查找完整个扇区..无有效数据,则擦除整个扇区*/
	Flash_Sector_Erase(nStartAddr);

	pUserStruct[nSizeOfStruct - 1] = (unsigned char)eFlashFlagValid;
	Flash_Write_Array(nStartAddr , (unsigned char * )pUserStruct, nSizeOfStruct);

	TRACE_PRINTF("Write valid data, Offset(Dirty Sector,need to be erased) : 0x%08X\r\n",nOffset);
}




