#ifndef _BUZZER_H_
#define _BUZZER_H_

typedef enum
{
	eBuzzerOn,
	eBuzzerOff,
}eBuzzerState;

typedef struct
{
	bool bIsBusy;
	eBuzzerState eCurrentState;
	unsigned short nTimeCnt;
	unsigned short nOnDuration;
	unsigned short nOffDuration;
	unsigned char nRepeatCnt;
}sBuzzerConfig;

typedef enum
{
	eBUZ_NORMAL = 0,
	eBUZ_START_TEST,
	eBUZ_DEVICE_OK,
	eBUZ_PROCESS,
	eBUZ_REPORT,
	eBUZ_REPORT_PASS,
	eBUZ_REPORT_FAIL,
	eBUZ_BLOW_BREAK,
	eBUZ_BLOW_OK,
	eBUZ_ALRAM,
	eBUZ_ERROR,
	eBUZ_CRITICAL,
	eBUZ_OK,
	eBUZ_START,
	eBUZ_BREAK,
	eBUZ_TIMEOUT,
	eBUZ_PRESSOK,
	eBUZ_LINK,
	eBUZ_FP_IDENTIFY_FAIL,
	eOP
} EBuzzerType;

typedef struct
{
	bool bBuzzerDefaultState;
	bool bNeedRepeat;
	const unsigned char aCtrlTable[15];
}__attribute__ ((packed))sBuzzerActionStruct;


extern sBuzzerConfig g_sBuzzerConfig;

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Beep(EBuzzerType eBeep);
void Buzzer_Ctrl(void);


#endif

