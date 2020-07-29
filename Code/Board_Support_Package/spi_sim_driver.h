#ifndef _SPI_SIM_DRIVER_H_
#define _SPI_SIM_DRIVER_H_

#define SIM_SPI

void Spi_Sim_Init(void);
unsigned char Spi_Sim_Get_Single_Byte(void);
void Spi_Sim_Send_Single_Byte(unsigned char nData);


#endif
