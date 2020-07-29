#include "platform.h"
#include "GUI.h"
#include "app.h"
#include "keypad.h"
#include "buzzer.h"
#include "system_misc.h"

bool g_bIgnoreReleaseKeyEvent = false;

static bool s_bInvalidComblinationKey;
static unsigned short s_aKeyCnt[KEY_TOTAL_COUNTER];
bool g_bShortKeyEvent[KEY_TOTAL_COUNTER];
bool g_bLongKeyEvent[KEY_TOTAL_COUNTER];

bool KeyLeftIsPressed(void)
{
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) ? false : true;
}

bool KeyRightIsPressed(void)
{
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) ? false : true;
}

bool KeyMiddleIsPressed(void)
{
	return GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13) ? true : false;
}

void Key_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static unsigned char Get_Key_Value(unsigned char  *pMutiKeyCnt)
{
	unsigned char nKeyValue = 0, nMutiKeyCnt = 0;

	/*Power Key*/
	if(GPIOB->IDR & GPIO_Pin_13)
	{
		nKeyValue |= 0x01;
		nMutiKeyCnt++;
	}

	/*Left Key*/
	if((GPIOB->IDR & GPIO_Pin_12) == 0)
	{
		nKeyValue |= 0x02;
		nMutiKeyCnt++;
	}

	/*Right Key*/
	if((GPIOB->IDR & GPIO_Pin_14) == 0)
	{
		nKeyValue |= 0x04;
		nMutiKeyCnt++;
	}

	*pMutiKeyCnt = nMutiKeyCnt;

	return nKeyValue;
}

int KeyIndexToKeyValue(char nKeyIndex)
{
	switch(nKeyIndex)
	{
		case 0:
			return eKey_OK;
		case 1:
			return ekey_Left_Fun;
		case 2:
			return ekey_Right_Fun;
	}
	return eKey_OK;
}

int KeyValueToKeyIndex(char nKeyValue)
{
	switch(nKeyValue)
	{
		case eKey_OK:
			return 0;
		case ekey_Left_Fun:
			return 1;
		case ekey_Right_Fun:
			return 2;
	}
	return 0;
}

static void Key_Press_Hook(char nKeyIndex, unsigned short nKeyPressDurationCnt)
{
	pApp->nStartIdleTime = Get_System_Time();
	if(pApp->eWorkState == eWorkNormal)
	{
		GUI_StoreKeyMsg(KeyIndexToKeyValue(nKeyIndex), eKeyPressed);
	}
	else
	{
		g_bIgnoreReleaseKeyEvent = true;
	}
}

static void Key_Release_Hook(char nKeyIndex)
{
	Buzzer_Beep(eBUZ_PRESSOK);
	if(pApp->eWorkState == eWorkNormal)
	{
		if(g_bIgnoreReleaseKeyEvent)
		{
			g_bIgnoreReleaseKeyEvent = false;
		}
		else
		{	
			GUI_StoreKeyMsg(KeyIndexToKeyValue(nKeyIndex), eKeyReleased);
		}
	}
}

bool Get_Key_Pressed_State(eKey nKey)
{
	switch(nKey)
	{
		case eKey_OK:
			return (GPIOB->IDR & GPIO_Pin_13) ? true : false;
		case eKey_Left:
			return (GPIOB->IDR & GPIO_Pin_12) ? false : true;
		case eKey_Right:
			return (GPIOB->IDR & GPIO_Pin_14) ? false : true;
		default:
			return false;
	}
}

/*返回值单位为ms*/
unsigned long Get_Key_PressDurationTime(eKey nKey)
{
	return (unsigned long)s_aKeyCnt[KeyValueToKeyIndex(nKey)] * 10;
}


/*此函数的调用周期10ms*/
void Key_Scan(void)
{
	unsigned char i, nMutiKeyCnt;
	static unsigned short nKey;

	nKey = Get_Key_Value(&nMutiKeyCnt);

	if(nMutiKeyCnt && nMutiKeyCnt != 1)
	{
		for(i = 0; i < KEY_TOTAL_COUNTER; i++)
		{
			s_aKeyCnt[i] = 0;
		}
		s_bInvalidComblinationKey = true;
	}
	else
	{
		if(s_bInvalidComblinationKey)
		{
			if(nKey == 0)
			{
				s_bInvalidComblinationKey = false;
				GUI_StoreKeyMsg(eKey_OK, eKeyIgnore);
			}
			else
			{
				return;
			}
		}

		for(i = 0; i < KEY_TOTAL_COUNTER; i++)
		{
			if(nKey & (1 << i))
			{
				s_aKeyCnt[i]++;
				
				if(s_aKeyCnt[i] == 3)	
				{
					Key_Press_Hook(i,s_aKeyCnt[i]);
				}
				else if(s_aKeyCnt[i] > 100 )
				{
					if(s_aKeyCnt[i] == 150)
					{
						g_bLongKeyEvent[i] = true;
					}
				}
			}
			else if(s_aKeyCnt[i] > 3)
			{
				s_aKeyCnt[i] = 0;
				g_bShortKeyEvent[i] = true;
				Key_Release_Hook(i);
			}
			else
			{
				s_aKeyCnt[i] = 0;
			}
		}
	}
}

