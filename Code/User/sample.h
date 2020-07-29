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
	bool bGetPeekValue;						/*�Ƿ������ֵ*/
	unsigned short nSampleMaxValue;			/*���ֹ��������ֵ*/
	unsigned short nIntergralLowerValue;		/*��������ֵ.*/
	unsigned short nCntGetPeekValue;			/*�ڴ�ֵ ֮ǰһ���ü�������ķ�ֵ,����ƾ�Ũ��Ϊ0ʱ,һֱ�����˳�.*/

	unsigned short nTotalIntegralCnt;			/*�ܵĻ��ִ���*/
	unsigned long nIntegralSumValue;			/*�ܵĻ���ֵ.*/
	
	unsigned short nSampleDataInCnt;			/*����������.*/
	unsigned short nSampleDataOutCnt;			/*����������ݼ�����*/
	unsigned short aAdcSampleDataBuffer[ADC_SAMPLE_BUFFER_SIZE];
	
	signed char nAlcSampleStartTemperature;	/*���ֿ�ʼʱ�¶�.*/
	signed char nAlcSampleEndTemperature;	/*���ֽ���ʱ�¶�.*/
}sAlcSample;

extern sAlcSample g_sAlcSample;

void Start_Sample(void);
void Stop_Sample(void);
bool Alcohol_Sensor_AD_Integral(void);

#endif
