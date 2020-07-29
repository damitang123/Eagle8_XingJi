#ifndef _I2C_H_
#define _I2C_H_

extern bool g_bI2CValidAck;

void I2C_Hal_Init(void);
void I2C_Write_Command(unsigned char nDEV_Address, unsigned char nREG_Address);
void I2C_Single_Write(unsigned char nDEV_Address,unsigned char nREG_Address,unsigned char nREG_data);
unsigned char I2C_Single_Read(unsigned char nDEV_Address,unsigned char nREG_Address);
void I2C_Block_Write(unsigned char nDEV_Address,unsigned char *aBuffer,unsigned char nReg_Addr,unsigned char nCount);
void I2C_Block_Read(unsigned char nDEV_Address,unsigned char *aBuffer,unsigned char nReg_Addr,unsigned char nCount);

void I2C_Block_Write_2Reg(unsigned char nDEV_Address, unsigned char *aBuffer, unsigned short nReg_Addr, unsigned char nCount);
void I2C_Block_Read_2Reg(unsigned char nDEV_Address, unsigned char *aBuffer, unsigned short nReg_Addr, unsigned char nCount);

#endif
