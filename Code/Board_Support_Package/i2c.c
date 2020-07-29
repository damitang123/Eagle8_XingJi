#include "stm32f10x.h"
#include "delay.h"

#define I2C_GPIO_USE_OD_CONFIG	0

#define SCL1 				GPIOB->BSRR=GPIO_Pin_6
#define SCL0 				GPIOB->BRR=GPIO_Pin_6
#define SDA1 			GPIOB->BSRR=GPIO_Pin_7
#define SDA0 			GPIOB->BRR=GPIO_Pin_7
#define SDADATA 		(GPIOB->IDR & GPIO_Pin_7)

bool g_bI2CValidAck = false;

#if I2C_GPIO_USE_OD_CONFIG

void I2C_Hal_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIOB->BSRR=GPIO_Pin_6|GPIO_Pin_7;
}

/************************************************************/

static void I2C_Delay(void)
{
	delay_us(5);
}

static void I2C_Start(void)
{
	SDA1;
	SCL1;
	I2C_Delay();
	SDA0;
	I2C_Delay();
	SCL0;
	I2C_Delay();
}

static void I2C_Stop(void)
{
	SDA0;
	I2C_Delay();
	SCL1;
	I2C_Delay();
	SDA1;
	I2C_Delay();
}

static void I2C_SendACK(unsigned char nAckValue)
{
	if(nAckValue)
	{
		SDA1;
	}
	else
	{
		SDA0;
	}
	I2C_Delay();
	SCL1;
	I2C_Delay();
	SCL0;
	I2C_Delay();
}

static unsigned char I2C_RecvACK(void)
{
	unsigned char cy;
	SDA1;
	I2C_Delay();
	SCL1;
	I2C_Delay();
	if(SDADATA)
	{
		cy = 1;
	}
	else
	{
		cy = 0;
	}
	SCL0;
	I2C_Delay();
	return cy;
}

static bool I2C_SendByte(unsigned char dat)
{
	unsigned char i;
	for (i = 0; i < 8; i++)
	{
		if((dat << i) & 0x80)
		{
			SDA1;
		}
		else
		{
			SDA0;
		}
		I2C_Delay();
		SCL1;
		I2C_Delay();
		SCL0;
		I2C_Delay();
	}
	return  (I2C_RecvACK() == 0 ?  true : false);
}

static unsigned char I2C_RecvByte(void)
{
	unsigned char i;
	unsigned char dat = 0;
	unsigned char cy;
	SDA1;
	for (i = 0; i < 8; i++)
	{
		dat <<= 1;
		SCL1;
		I2C_Delay();;
		if(SDADATA)
		{
			cy = 1;
		}
		else
		{
			cy = 0;
		}
		dat |= cy;
		SCL0;
		I2C_Delay();
	}
	return dat;
}

#else

void I2C_Hal_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIOB->BSRR=GPIO_Pin_6|GPIO_Pin_7;
}

/************************************************************/

static void I2C_Delay(void)
{
	delay_us(5);
}

static void I2C_Start(void)
{
	SDA1;
	SCL1;
	I2C_Delay();
	SDA0;
	I2C_Delay();
	SCL0;
	I2C_Delay();
}

static void I2C_Stop(void)
{
	SDA0;
	I2C_Delay();
	SCL1;
	I2C_Delay();
	SDA1;
	I2C_Delay();
}

static void I2C_SDA_In(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void I2C_SDA_Out(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_7;
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void I2C_SendACK(unsigned char nAckValue)
{
	I2C_SDA_Out();
	if(nAckValue)
	{
		SDA1;
	}
	else
	{
		SDA0;
	}
	I2C_Delay();
	SCL1;
	I2C_Delay();
	SCL0;
	I2C_Delay();
}

static unsigned char I2C_RecvACK(void)
{
	unsigned char cy;
	SDA1;
	I2C_SDA_In();
	I2C_Delay();
	SCL1;
	I2C_Delay();
	if(SDADATA)
	{
		cy = 1;
	}
	else
	{
		cy = 0;
	}
	SCL0;
	I2C_Delay();
	I2C_SDA_Out();
	return cy;
}

static bool I2C_SendByte(unsigned char dat)
{
	unsigned char i;

	I2C_SDA_Out();
	
	for (i = 0; i < 8; i++)
	{
		if((dat << i) & 0x80)
		{
			SDA1;
		}
		else
		{
			SDA0;
		}
		I2C_Delay();
		SCL1;
		I2C_Delay();
		SCL0;
		I2C_Delay();
	}
	return  (I2C_RecvACK() == 0 ?  true : false);
}

static unsigned char I2C_RecvByte(void)
{
	unsigned char i;
	unsigned char dat = 0;
	unsigned char cy;
	
	SDA1;
	I2C_SDA_In();
	
	for (i = 0; i < 8; i++)
	{
		dat <<= 1;
		SCL1;
		I2C_Delay();;
		if(SDADATA)
		{
			cy = 1;
		}
		else
		{
			cy = 0;
		}
		dat |= cy;
		SCL0;
		I2C_Delay();
	}
	return dat;
}

#endif

void I2C_Write_Command(unsigned char nDEV_Address, unsigned char nREG_Address)
{
	I2C_Start();
	g_bI2CValidAck = I2C_SendByte(nDEV_Address);
	g_bI2CValidAck = I2C_SendByte(nREG_Address);
	I2C_Stop();
}

void I2C_Single_Write(unsigned char nDEV_Address, unsigned char nREG_Address, unsigned char nREG_data)
{
	I2C_Start();
	I2C_SendByte(nDEV_Address);
	I2C_SendByte(nREG_Address);
	I2C_SendByte(nREG_data);
	I2C_Stop();
}

unsigned char I2C_Single_Read(unsigned char nDEV_Address, unsigned char nREG_Address)
{
	unsigned char nREG_data;
	I2C_Start();
	I2C_SendByte(nDEV_Address);
	I2C_SendByte(nREG_Address);
	I2C_Start();
	I2C_SendByte(nDEV_Address + 1);
	nREG_data = I2C_RecvByte();
	I2C_SendACK(1);
	I2C_Stop();
	return nREG_data;
}

void I2C_Block_Write(unsigned char nDEV_Address, unsigned char *aBuffer, unsigned char nReg_Addr, unsigned char nCount)
{
	unsigned char i;
	I2C_Start();
	I2C_SendByte(nDEV_Address);
	I2C_SendByte(nReg_Addr);
	for(i = 0; i < nCount; i++)
	{
		I2C_SendByte(*aBuffer++);
	}
	I2C_Stop();
}

void I2C_Block_Read(unsigned char nDEV_Address, unsigned char *aBuffer, unsigned char nReg_Addr, unsigned char nCount)
{
	unsigned char i;
	I2C_Start();
	I2C_SendByte(nDEV_Address);
	I2C_SendByte(nReg_Addr);
	I2C_Start();
	I2C_SendByte(nDEV_Address + 1);
	if(nCount > 1)
	{
		for(i = 0; i < nCount - 1; i++)
		{
			*aBuffer++ = I2C_RecvByte();
			I2C_SendACK(0);
		}
	}
	*aBuffer = I2C_RecvByte();
	I2C_SendACK(1);
	I2C_Stop();
}

/*************************************************************************************************************************/
/*扩展的I2C读写,专供给摄像头使用.*/
void I2C_Block_Write_2Reg(unsigned char nDEV_Address, unsigned char *aBuffer, unsigned short nReg_Addr, unsigned char nCount)
{
	unsigned char i;
	I2C_Start();
	I2C_SendByte(nDEV_Address);
	I2C_SendByte((unsigned char)(nReg_Addr >> 8));
	I2C_SendByte((unsigned char)nReg_Addr);
	for(i = 0; i < nCount; i++)
	{
		I2C_SendByte(*aBuffer++);
	}
	I2C_Stop();
}

void I2C_Block_Read_2Reg(unsigned char nDEV_Address, unsigned char *aBuffer, unsigned short nReg_Addr, unsigned char nCount)
{
	unsigned char i;
	I2C_Start();
	I2C_SendByte(nDEV_Address);
	I2C_SendByte((unsigned char)(nReg_Addr >> 8));
	I2C_SendByte((unsigned char)nReg_Addr);
	I2C_Start();
	I2C_SendByte(nDEV_Address + 1);
	if(nCount > 1)
	{
		for(i = 0; i < nCount - 1; i++)
		{
			*aBuffer++ = I2C_RecvByte();
			I2C_SendACK(0);
		}
	}
	*aBuffer = I2C_RecvByte();
	I2C_SendACK(1);
	I2C_Stop();
}

