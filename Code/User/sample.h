#ifndef _SAMPLE_H_
#define _SAMPLE_H_

#define SAMPLE_INTEGRAL_LOWER_RATIO		0.1
#define SAMPLE_RPOCESS_BUFFER_SIZE		16
#define ADC_SAMPLE_BUFFER_SIZE			500
#define MAX_AD_INTEGRAL_CNT				30000

typedef struct
{
	unsigned short nCnt;
	unsigned short nValue;
} sSamplePoint;

typedef struct
{
	bool bGetPeekValue;						/*是否获得最大值*/
	unsigned short nSampleMaxValue;			/*积分过程中最大值*/
	unsigned short nIntergralLowerValue;		/*积分下限值.*/
	unsigned short nCntGetPeekValue;			/*在此值 之前一定得计算出最大的峰值,否则酒精浓度为0时,一直不能退出.*/

	unsigned short nTotalIntegralCnt;			/*总的积分次数*/
	unsigned long nIntegralSumValue;			/*总的积分值.*/
	
	unsigned short nSampleDataInCnt;			/*采样计数器.*/
	unsigned short nSampleDataOutCnt;			/*处理采样数据计数器*/
	unsigned short aAdcSampleDataBuffer[ADC_SAMPLE_BUFFER_SIZE];
	
	signed char nAlcSampleStartTemperature;	/*积分开始时温度.*/
	signed char nAlcSampleEndTemperature;	/*积分结束时温度.*/
}sAlcSample;

extern sAlcSample g_sAlcSample;

void Start_Sample(void);
void Stop_Sample(void);
bool Alcohol_Sensor_AD_Integral(void);

#endif
