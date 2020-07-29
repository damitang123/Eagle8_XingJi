#ifndef	_ADS8320_H_
#define _ADS8320_H_

#define	USE_ADS8320			1
#define  	USE_ADS3201			0


void Ad8320_CS_Enable(void);
void Ad8320_CS_Disable(void);

void Ad8320_Gpio_Init(void);
unsigned short Adc_Get_Alcohol_Sensor_Value(void);
unsigned short Adc_Get_Alcohol_Sensor_Value_Fast(void);
BOOL Adc_Is_Alcohol_Sensor_Ready(void);

#endif
