#ifndef _SPI_FLASH_H_
#define _SPI_FLASH_H_

#define FLASH_SECTOR_SIZE		0x1000

typedef struct
{
	char nFlashType;
	const char *strFlashName;
	unsigned char nFlashMFID;
	unsigned char nFlashDeviceID;
	unsigned short nFlashMaxSector;
}sFLashTypeTable;


#define FLASH_CMD_READ  		(unsigned)0x03
#define FLASH_CMD_WRITE 		(unsigned)0x02
#define FLASH_CMD_WREN  		(unsigned)0x06
#define FLASH_CMD_RDSR  		(unsigned)0x05
#define FLASH_CMD_ERASE 		(unsigned)0x60
#define FLASH_CMD_EWSR  		(unsigned)0x50
#define FLASH_CMD_WRSR  		(unsigned)0x01
#define FLASH_CMD_SER   		(unsigned)0x20
#define FLASH_CMD_RMFID   		(unsigned)0x90


void Spi_Flash_Init(void);
void Spi_Flash_DeInit(void);
void Flash_Check_Type(void);
unsigned short Flash_Get_Max_Sector(void);
bool Flash_Write_Byte(unsigned long nAddr, unsigned char data);
unsigned char Flash_Read_Byte(unsigned long nAddr);
bool Flash_Write_Array(unsigned long nAddr, unsigned char  *pData, unsigned short nCount);
void Flash_Read_Array(unsigned long nAddr, unsigned char *pData, unsigned short nCount);
bool Flash_Content_Copy(unsigned long nAddrSrc,unsigned long nAddrDest,unsigned long nSize);
bool Flash_Modify_Content_Any_Addr_Any_Size(unsigned long nAddr, unsigned char  *pData, unsigned short nSize,unsigned long nSwapAddr);
void FLASH_Get_ID(unsigned long nAddr,unsigned char *nMFID, unsigned char *nDeviceID);
void Flash_Unprotect(void);
bool Flash_Sector_Erase(unsigned long nAddr);
bool Flash_Chip_Erase(void);

void Flash_Read_Array_For_Ex_Font(unsigned long nAddr, unsigned char *pData, unsigned short nCount);


#endif
