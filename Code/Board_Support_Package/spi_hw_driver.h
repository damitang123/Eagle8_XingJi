#ifndef _SPI_HW_DRIVER_H_
#define _SPI_HW_DRIVER_H_

#define HW_SPI

void Spi_Hw_Init(void);
unsigned char Spi_Hw_Get_Single_Byte(void);
void Spi_Hw_Send_Single_Byte(unsigned char nData);


#endif
