#ifndef _ADC_H_
#define _ADC_H_

#define ADC_DMA_MODE			0
#define ADC_TRIGER_BY_TIMER	1

#define NumberOfChannel			5
#define NumberOfSampleTimes	5
extern unsigned short ADCConvertedValue[NumberOfSampleTimes][NumberOfChannel];

#define	Regular_Index_PumpVoltage		1
#define	Regular_Index_TC1047			2
#define	Regular_Index_NTC				3
#define	Regular_Index_Battery			4
#define	Regular_Index_PressSensorValue	5

unsigned short ADC_Software_Filter(unsigned char  Index);

#define	Adc_Get_PumpVoltage()			ADC_Software_Filter(Regular_Index_PumpVoltage-1)
#define	Adc_Get_TC1047_Value()			ADC_Software_Filter(Regular_Index_TC1047-1)
#define	Adc_Get_Alc_Sensor_Temprature_Value()		ADC_Software_Filter(Regular_Index_NTC-1)
#define	Adc_Get_Battery_Value()			ADC_Software_Filter(Regular_Index_Battery-1)
#define 	Adc_Get_PressSensorValue()		ADC_Software_Filter(Regular_Index_PressSensorValue-1)

typedef struct
{
	signed char temperature;
	unsigned short adcValue;
} __attribute__ ((packed)) sTemperature_VS_Adc;

void ADC1_Configuration(void);

BOOL Adc_Is_Pump_Ready(void);
BOOL Adc_Is_Blowing(unsigned short nOriginalAdcValue, signed char nEnvironmentTemperature);
BOOL Adc_Is_Inhaling(unsigned short nOriginalAdcValue, signed char nEnvironmentTemperature);
float Adc_Get_Sensor_Temperature(void);
float Adc_Get_Environment_Temperature(void);

float Adc_Get_Battery_Voltage(void);
unsigned char Adc_Get_Battery_Volume(void);

void ADC_Test(void);

void ADC2_Configuration(void);
unsigned short ADC_Get_Quick_ALC_Sensor(void);
bool Adc_Is_Quick_Test_Alcohol_Sensor_Ready(void);


#endif
