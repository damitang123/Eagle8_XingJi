#ifndef _SPI_FLASH_EFFICIENT_RW_H_
#define _SPI_FLASH_EFFICIENT_RW_H_

bool Spi_FLash_Load_User_Struct(unsigned char *pUserStruct, unsigned short nSizeOfStruct , unsigned short nStartSector);
void Spi_FLash_Save_User_Struct(unsigned char *pUserStruct, unsigned short nSizeOfStruct, unsigned short nStartSector);

#endif


