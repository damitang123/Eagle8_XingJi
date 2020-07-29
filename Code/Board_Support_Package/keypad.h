#ifndef _KEYPAD_H_
#define _KEYPAD_H_

#include "GUI.h"

/*°´¼ü¼ì²âºê¶¨Òå*/
typedef enum
{
	eKeyReleased = 0,
	eKeyPressed = 1,
	eKeyPressedContinue = 2,
	eKeyIgnore = 3,
} eKeyPressState;

typedef enum
{
	eKey_Idle = 0,
	eKey_Invalid = 1,
	eKey_Left = GUI_KEY_LEFT,
	eKey_Right = GUI_KEY_RIGHT,
	eKey_Up = GUI_KEY_UP,
	eKey_Down = GUI_KEY_DOWN,
	eKey_OK = GUI_KEY_ENTER,
	ekey_Left_Fun = GUI_KEY_LEFT_FUN,
	ekey_Right_Fun = GUI_KEY_RIGHT_FUN,
	eKey_Num0 = GUI_KEY_NUM0,
	eKey_Num1 = GUI_KEY_NUM1,
	eKey_Num2 = GUI_KEY_NUM2,
	eKey_Num3 = GUI_KEY_NUM3,
	eKey_Num4 = GUI_KEY_NUM4,
	eKey_Num5 = GUI_KEY_NUM5,
	eKey_Num6 = GUI_KEY_NUM6,
	eKey_Num7 = GUI_KEY_NUM7,
	eKey_Num8 = GUI_KEY_NUM8,
	eKey_Num9 = GUI_KEY_NUM9,
	eKey_Star = GUI_KEY_STAR,
	eKey_Well = GUI_KEY_NUMBER_KEY,
} eKey;

#define KEY_TOTAL_COUNTER	3

extern bool g_bShortKeyEvent[KEY_TOTAL_COUNTER];
extern bool g_bLongKeyEvent[KEY_TOTAL_COUNTER];

void Key_Init(void);
void Key_Scan(void);
int KeyValueToKeyIndex(char nKeyValue);
bool KeyLeftIsPressed(void);
bool KeyRightIsPressed(void);
bool KeyMiddleIsPressed(void);
bool Get_Key_Pressed_State(eKey nKey);
unsigned long Get_Key_PressDurationTime(eKey nKey);

#endif
