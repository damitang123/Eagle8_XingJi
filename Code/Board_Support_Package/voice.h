#ifndef _VOICE_H_
#define _VOICE_H_

#define VOICE_MIN_VOLUME		0xE0
#define VOICE_MAX_VOLUME		0xEF

typedef enum
{
	eVOC_BLOW = 0,
	eVOC_BLOW_INT,
	eVOC_RESULT_DRINK,
	eVOC_RESULT_DRUNK,
	eVOC_LOW_POWER,
	eVOC_PRINT_NOPAPER,
	eVOC_INPUT_PLATE,
	eVOC_INPUT_LISENCE,
	eVOC_INPUT_POLICEID,
	eVOC_PRESS_FINGER,
	eVOC_FINGER_VERI_OK,
	eVOC_FINGER_VERI_FAIL,
	eVOC_RESULT_PASS,
	eVOC_INPUT_TESTER_NAME,
	eVOC_INPUT_TEST_LOCATION,
	eVOC_INPUT_POLICE_NAME,
	eVOC_INPUT_DEPARTMENT
} EVoiceType;

void Voc_Init(void);

void Voc_Cmd(unsigned char eVoc);

void Voc_Close(void);

void Voice_Alc_Value(float fAlc, unsigned char nTestUnit);

void Voice_Test(void);

#endif

