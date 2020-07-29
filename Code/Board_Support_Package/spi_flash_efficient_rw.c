#include "platform.h"
#include "spi_flash.h"
/*
	Spi_FLash_Load_User_Struct  ����˵��:
	�������pUserStruct �Ĵ�С������Ա�һ����������.(�����Ա�4096����)
	���������pUserStruct ���һ��Ԫ�ر����������Ƿ���Ч�ı�־.
	������Ч���ݷ��ص�ʱ�򷵻�FALSE(����ʱӦע���������),������Ч��ʱ�򷵻�TRUE.
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

	/*��������������..����Ч����,�������������*/
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
		/*����ȡFLAG��־.*/
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

	/*��������������..����Ч����,�������������*/
	Flash_Sector_Erase(nStartAddr);

	pUserStruct[nSizeOfStruct - 1] = (unsigned char)eFlashFlagValid;
	Flash_Write_Array(nStartAddr , (unsigned char * )pUserStruct, nSizeOfStruct);

	TRACE_PRINTF("Write valid data, Offset(Dirty Sector,need to be erased) : 0x%08X\r\n",nOffset);
}




