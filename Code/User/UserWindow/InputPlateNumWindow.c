#include "platform.h"
#include "Windows.h"
#include "rtc.h"
#include "time_calc.h"
#include "EDIT_Private.h"
#include "app.h"

#define INITIAL_XINGJI     0

#if INITIAL_XINGJI
#define Btn_Width		30
#define Btn_Height		28
#define Btn_H_Interval 	2
#define Btn_V_Interval 	2
#define X_START			(LCD_XSIZE-((Btn_Width*7)+Btn_H_Interval*6)>>1)
#define Y_START			73
#else
#define Btn_Width		30
#define Btn_Num_Width	42
#define Btn_Height		28
#define Btn_H_Interval 	2
#define Btn_V_Interval 	2
#define X_START			(LCD_XSIZE-((Btn_Width*7)+Btn_H_Interval*6)>>1)
#define Y_START			73
#endif

typedef struct
{
	const char *Key;
	const char **Key_To_Chinese;
} sKeyConvChinese;

extern int EDIT_Inpur_MaxNumBytes;

/*候选字第一个字的索引号*/
static unsigned char s_nCurDisplayFirstIndex = 1;
/*总共可用的候选字个数*/
static unsigned char s_nTotalOptionalChinese = 0;
/*按功能键是向后增加候选字,还是减小候选字序号*/
static unsigned char s_bIncreaseOptionalChinese = 1;
/*指向当前候选字的首地址的指针*/
static const sKeyConvChinese *s_pKeyConvChinese;

static const char *Btn_EnglishTab[] =
{
	"0", "1", "2", "3", "4", "5", "6",
	"7", "8", "9", "A", "B", "C", "D",
	"E", "F", "G", "H", "I", "J", "K",
	"L", "M", "N", "O", "P", "Q", "R",
	"S", "T", "U", "V", "W", "X", "Y",
	"Z", "", "", "", ".", "", "",
	"", "", "", "", "", "", "",
};

static const unsigned short KeyIDTab[] =
{
	10000, 10001, 10002, 10003, 10004, 10005, 10006,
	10007, 10008, 10009, 10010, 10011, 10012, 10013,
	10014, 10015, 10016, 10017, 10018, 10019, 10020,
	10021, 10022, 10023, 10024, 10025, 10026, 10027,
	10028, 10029, 10030, 10031, 10032, 10033, 10034,
	10035, 10036, 10037, 10038, 10039, 10040, 10041,
	10042, 10043, 10044, 10045, 10046, 10047, 10048,
};

const char aA1[] = {0xE6, 0xBE, 0xB3, 0x00}; /*澳*/

const char aB1[] = {0xE8, 0xBE, 0xB9, 0x00}; /*边*/
const char aB2[] = {0xE5, 0x8C, 0x97, 0x00}; /*北*/

const char aC1[] = {0xE5, 0xB7, 0x9D, 0x00}; /*川*/
const char aC2[] = {0xE6, 0x88, 0x90, 0x00}; /*成*/

const char aD1[] = {0xE7, 0x94, 0xB5, 0x00}; /*电*/

const char aE1[] = {0xE9, 0x84, 0x82, 0x00}; /*鄂*/

const char aG1[] = {0xE7, 0x94, 0x98, 0x00}; /*甘*/
const char aG2[] = {0xE8, 0xB5, 0xA3, 0x00}; /*赣*/
const char aG3[] = {0xE6, 0xA1, 0x82, 0x00}; /*桂*/
const char aG4[] = {0xE8, 0xB4, 0xB5, 0x00}; /*贵*/
const char aG5[] = {0xE6, 0xB8, 0xAF, 0x00}; /*港*/
const char aG6[] = {0xE6, 0x8C, 0x82, 0x00}; /*挂*/
const char aG7[] = {0xE5, 0xB9, 0xBF, 0x00}; /*广*/

const char aH1[] = {0xE6, 0xB2, 0xAA, 0x00}; /*沪*/
const char aH2[] = {0xE9, 0xBB, 0x91, 0x00}; /*黑*/
const char aH3[] = {0xE6, 0xB5, 0xB7, 0x00}; /*海*/

const char aJ1[] = {0xE4, 0xBA, 0xAC, 0x00}; /*京*/
const char aJ2[] = {0xE6, 0xB4, 0xA5, 0x00}; /*津*/
const char aJ3[] = {0xE5, 0x90, 0x89, 0x00}; /*吉*/
const char aJ4[] = {0xE5, 0x86, 0x80, 0x00}; /*冀*/
const char aJ5[] = {0xE6, 0x99, 0x8B, 0x00}; /*晋*/
const char aJ6[] = {0xE5, 0xA2, 0x83, 0x00}; /*境*/
const char aJ7[] = {0xE8, 0xAD, 0xA6, 0x00}; /*警*/
const char aJ8[] = {0xE5, 0x86, 0x9B, 0x00}; /*军*/
const char aJ9[] = {0xE6, 0xB5, 0x8E, 0x00}; /*济*/
const char aJ10[] = {0xE6, 0xB1, 0x9F, 0x00}; /*江*/

const char aK0[] = {0xE7, 0xA9, 0xBA, 0x00}; /*空*/

const char aL1[] = {0xE8, 0xBE, 0xBD, 0x00}; /*辽*/
const char aL2[] = {0xE9, 0xB2, 0x81, 0x00}; /*鲁*/
const char aL3[] = {0xE4, 0xB8, 0xB4, 0x00}; /*临*/
const char aL4[] = {0xE9, 0xA2, 0x86, 0x00}; /*领*/
const char aL5[] = {0xE6, 0x9E, 0x97, 0x00}; /*林*/
const char aL6[] = {0xE5, 0x85, 0xB0, 0x00}; /*兰*/

const char aM1[] = {0xE8, 0x92, 0x99, 0x00}; /*蒙*/
const char aM2[] = {0xE9, 0x97, 0xBD, 0x00}; /*闽*/

const char aN1[] = {0xE5, 0xAE, 0x81, 0x00}; /*宁*/
const char aN2[] = {0xE5, 0x86, 0x9C, 0x00}; /*农*/
const char aN3[] = {0xE5, 0x8D, 0x97, 0x00}; /*南*/

const char aQ1[] = {0xE9, 0x9D, 0x92, 0x00}; /*青*/
const char aQ2[] = {0xE7, 0x90, 0xBC, 0x00}; /*琼*/

const char aS1[] = {0xE9, 0x99, 0x95, 0x00}; /*陕*/
const char aS2[] = {0xE8, 0x8B, 0x8F, 0x00}; /*苏*/
const char aS3[] = {0xE4, 0xBD, 0xBF, 0x00}; /*使*/
const char aS4[] = {0xE8, 0xAF, 0x95, 0x00}; /*试*/
const char aS5[] = {0xE6, 0xB0, 0xB4, 0x00}; /*水*/
const char aS6[] = {0xE6, 0xB2, 0x88, 0x00}; /*沈*/

const char aT1[] = {0xE5, 0x8F, 0xB0, 0x00}; /*台*/
const char aT2[] = {0xE6, 0x8B, 0x96, 0x00}; /*拖*/
const char aT3[] = {0xE9, 0x80, 0x9A, 0x00}; /*通*/

const char aW1[] = {0xE7, 0x9A, 0x96, 0x00}; /*皖*/

const char aX1[] = {0xE6, 0xB9, 0x98, 0x00}; /*湘*/
const char aX2[] = {0xE6, 0x96, 0xB0, 0x00}; /*新*/
const char aX3[] = {0xE5, 0xAD, 0xA6, 0x00}; /*学*/
const char aX4[] = {0xE6, 0xB6, 0x88, 0x00}; /*消*/

const char aY1[] = {0xE6, 0xB8, 0x9D, 0x00}; /*渝*/
const char aY2[] = {0xE8, 0xB1, 0xAB, 0x00}; /*豫*/
const char aY3[] = {0xE7, 0xB2, 0xA4, 0x00}; /*粤*/
const char aY4[] = {0xE4, 0xBA, 0x91, 0x00}; /*云*/

const char aZ1[] = {0xE8, 0x97, 0x8F, 0x00}; /*藏*/
const char aZ2[] = {0xE6, 0xB5, 0x99, 0x00}; /*浙*/

static const char *Btn_A[] = {(char * )1, aA1};
static const char *Btn_B[] = {(char * )2, aB1, aB2};
static const char *Btn_C[] = {(char * )2, aC1, aC2};
static const char *Btn_D[] = {(char * )1, aD1};
static const char *Btn_E[] = {(char * )1, aE1};
static const char *Btn_F[] = {(char * )0};
static const char *Btn_G[] = {(char * )7, aG1, aG2, aG3, aG4, aG5, aG6, aG7};
static const char *Btn_H[] = {(char * )3, aH1, aH2, aH3};
static const char *Btn_I[] = {(char * )0};
static const char *Btn_J[] = {(char * )10, aJ1, aJ2, aJ3, aJ4, aJ5, aJ6, aJ7, aJ8, aJ9, aJ10};
static const char *Btn_K[] = {(char * )1, aK0};
static const char *Btn_L[] = {(char * )6, aL1, aL2, aL3, aL4, aL5, aL6};
static const char *Btn_M[] = {(char * )2, aM1, aM2};
static const char *Btn_N[] = {(char * )3, aN1, aN2, aN3};
static const char *Btn_O[] = {(char * )0};
static const char *Btn_P[] = {(char * )0};
static const char *Btn_Q[] = {(char * )2, aQ1, aQ2};
static const char *Btn_R[] = {(char * )0};
static const char *Btn_S[] = {(char * )6, aS1, aS2, aS3, aS4, aS5, aS6};
static const char *Btn_T[] = {(char * )3, aT1, aT2, aT3};
static const char *Btn_U[] = {(char * )0};
static const char *Btn_V[] = {(char * )0};
static const char *Btn_W[] = {(char * )1, aW1};
static const char *Btn_X[] = {(char * )4, aX1, aX2, aX3, aX4};
static const char *Btn_Y[] = {(char * )4, aY1, aY2, aY3, aY4};
static const char *Btn_Z[] = {(char * )2, aZ1, aZ2};


const sKeyConvChinese KeyConvChineseTab[] =
{
	{"A", Btn_A},
	{"B", Btn_B},
	{"C", Btn_C},
	{"D", Btn_D},
	{"E", Btn_E},
	{"F", Btn_F},
	{"G", Btn_G},
	{"H", Btn_H},
	{"I", Btn_I},
	{"J", Btn_J},
	{"K", Btn_K},
	{"L", Btn_L},
	{"M", Btn_M},
	{"N", Btn_N},
	{"O", Btn_O},
	{"P", Btn_P},
	{"Q", Btn_Q},
	{"R", Btn_R},
	{"S", Btn_S},
	{"T", Btn_T},
	{"U", Btn_U},
	{"V", Btn_V},
	{"W", Btn_W},
	{"X", Btn_X},
	{"Y", Btn_Y},
	{"Z", Btn_Z}
};

#if INITIAL_XINGJI

static void _Edit_Focus_Message(WM_MESSAGE *pMsg)
{
	CW_InputPlateNumberWindow *pThisWin;
	pThisWin = (CW_InputPlateNumberWindow *)FindWMBFromListByWin(pMsg->hWin);
	
	if(pMsg->Data.v == WM_NOTIFICATION_GOT_FOCUS)
	{
		if(pMsg->hWinSrc == pThisWin->hEditBox)
		{
			char x, y;
			WM_HWIN hWin;
			/*创建矩阵键盘*/
			for(y = 0; y < 7; y++)
			{
				for(x = 0; x < 7; x++)
				{
					hWin = BUTTON_CreateEx(X_START + x * (Btn_Width + Btn_H_Interval), Y_START + y * (Btn_Height + Btn_V_Interval) , Btn_Width, Btn_Height,
					                       pMsg->hWin,
					                       WM_CF_SHOW, 1, KeyIDTab[y * 7 + x]);
				
					BUTTON_SetFont(hWin, &GUI_Font16_UTF8);
					BUTTON_SetText(hWin, Btn_EnglishTab[y * 7 + x]);
					BUTTON_SetFocussable(hWin, 0);
					BUTTON_SetBkColor(hWin, 0, 0xB59179);
					BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
					BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
					BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
					BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
				}
			}

		}
	}
	else if(pMsg->Data.v == WM_NOTIFICATION_LOST_FOCUS)
	{
		if(pMsg->hWinSrc == pThisWin->hEditBox)
		{
			char i;
			WM_HWIN hWin;
			for(i = 0; i < 49; i++)
			{
				hWin = WM_GetDialogItem(pMsg->hWin, KeyIDTab[i]);
				if(hWin)
				{
					WM_DeleteWindow(hWin);
				}
			}
		}
	}
}

#else
static void _Edit_Focus_Message(WM_MESSAGE *pMsg)
{
	CW_InputPlateNumberWindow *pThisWin;
	pThisWin = (CW_InputPlateNumberWindow *)FindWMBFromListByWin(pMsg->hWin);
	
	if(pMsg->Data.v == WM_NOTIFICATION_GOT_FOCUS)
	{
		if(pMsg->hWinSrc == pThisWin->hEditBox)
		{
			char x, y;
			WM_HWIN hWin;
			for(y = 0; y < 2; y++)
			{
				for(x = 0; x < 5; x++)
				{
					hWin = BUTTON_CreateEx(X_START + x * (Btn_Num_Width + Btn_H_Interval + 1), Y_START + y * (Btn_Height + Btn_V_Interval) - 1 , Btn_Num_Width, Btn_Height,
					                       pMsg->hWin,
					                       WM_CF_SHOW, 1, KeyIDTab[y * 5 + x]);
					BUTTON_SetFont(hWin, &GUI_FontTahoma23);
					BUTTON_SetText(hWin, Btn_EnglishTab[y * 5 + x]);
					BUTTON_SetFocussable(hWin, 0);
					BUTTON_SetBkColor(hWin, 0, _RGB(153, 217, 234));
					BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
					BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
					BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
				}
			}
			/*创建矩阵键盘*/
			for(y = 2; y < 7; y++)
			{
				for(x = 0; x < 7; x++)
				{
					hWin = BUTTON_CreateEx(X_START + x * (Btn_Width + Btn_H_Interval), Y_START + y * (Btn_Height + Btn_V_Interval) + 2 , Btn_Width, Btn_Height,
					                       pMsg->hWin,
					                       WM_CF_SHOW, 1, KeyIDTab[10 +  (y - 2) * 7 + x]);

					if(y == 6)
					{
						BUTTON_SetFont(hWin, &GUI_Font16_UTF8);
					}
					else
					{
						BUTTON_SetFont(hWin, &GUI_FontTahoma23);
					}
					BUTTON_SetText(hWin, Btn_EnglishTab[10 +  (y - 2) * 7 + x]);
					BUTTON_SetFocussable(hWin, 0);
					if(y == 6)
					{
						BUTTON_SetBkColor(hWin, 0, 0xB59179);
						BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
						BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
						BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
					}
					else
					{
						BUTTON_SetBkColor(hWin, 0, GUI_WHITE);
						BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
						BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
						BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
					}
				}
			}
		}
	}
	else if(pMsg->Data.v == WM_NOTIFICATION_LOST_FOCUS)
	{
		if(pMsg->hWinSrc == pThisWin->hEditBox)
		{
			char i;
			WM_HWIN hWin;
			for(i = 0; i < 49; i++)
			{
				hWin = WM_GetDialogItem(pMsg->hWin, KeyIDTab[i]);
				if(hWin)
				{
					WM_DeleteWindow(hWin);
				}
			}
		}
	}
}

#endif

/*按下中间键后的消息.*/
/*
	输入顺序
	1.车牌号
	2.被测人姓名
	3.驾驶证号
	4.地点
	5.民警名字
	6.民警警号
	7.所属部门.
*/
static void _ConfirmButton_Function(WM_MESSAGE *pMsg)
{	
	sInputWindowProperty s;
	CW_InputPlateNumberWindow *pThisWin;
	pThisWin = (CW_InputPlateNumberWindow *)FindWMBFromListByWin(pMsg->hWin);

	s = s;
	pThisWin = pThisWin;

	switch(pThisWin->sProperty.nWinType)
	{
		case ePlateNumber:
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aPlateNumber, sizeof(g_sRecord.aPlateNumber));
			s.nWinType = eInputTestee;
			s.aCaptionText = _TesteeNameInput;
			CW_InputWindow_Create(pMsg->hWin,(void *)&s);
			break;
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;

	CW_InputPlateNumberWindow *pThisWin;
	pThisWin = (CW_InputPlateNumberWindow *)FindWMBFromListByWin(pMsg->hWin);

	s_nCurDisplayFirstIndex = 1;
	s_nTotalOptionalChinese = 0;
	s_bIncreaseOptionalChinese = 1; 
	s_pKeyConvChinese = NULL;

	pMsg->Data.p = pThisWin->sProperty.aCaptionText;
	Create_Dialog_Caption(pMsg);

#if USE_TEXT_REPLACE_ICON
	/*创建左键*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _Back);

	/*创建右键*/
	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _Delete);

	/*创建中键*/
	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);
#else
	/*创建左键*/
	hWin = Create_Left_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmCancel_Released, hWin));
	BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmCancel_Pressed, hWin));

	/*创建右键*/
	hWin = Create_Right_Button(pMsg->hWin);
	BUTTON_SetBitmapEx(hWin, 0, BtnSetBmp(bmLeft_Arrows_Released, hWin));
	BUTTON_SetBitmapEx(hWin, 1, BtnSetBmp(bmLeft_Arrows_Pressed, hWin));

	/*创建中键*/
	hWin = Create_Middle_Button(pMsg->hWin);
	BUTTON_SetText(hWin, _OK);
#endif

	switch(pThisWin->sProperty.nWinType)
	{
		case ePlateNumber:
			Voc_Cmd(eVOC_INPUT_PLATE);
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aPlateNumber) - 1;
			break;
	}
	pThisWin->hEditBox = EDIT_CreateEx(X_START, 34, LCD_XSIZE - X_START * 2, 33,
	                     pMsg->hWin, WM_CF_SHOW, 0, 0, pThisWin->hEditMaxLen);

	EDIT_SetFont(pThisWin->hEditBox, &GUI_Font16_UTF8);
	EDIT_SetTextAlign(pThisWin->hEditBox, GUI_TA_VCENTER);
	EDIT_SetTextColor(pThisWin->hEditBox, 0, GUI_RED);
	
	WM_SetFocus(pThisWin->hEditBox);
}
#if SERVER_USE_XING_JIKONG_GU
static char platnumber_Information[26];//added by lxl 
static const  char _PleaseInput_PlateNumber[]={0xE8,0xAF,0xB7,0xE8,0xBE,0x93,0xE5,0x85,0xA5,0xE8,0xBD,0xA6,0xE7,0x89,0x8C,0xE5,0x8F,0xB7,0xEF,0xBC,0x81,0x00};   /*请输入车牌号！*/

static void Alcohol_Value_Analyze_Left(CW_InputPlateNumberWindow *pThisWin,WM_MESSAGE *pMsg)
{
	if((g_sRecord.fAlcValue>=20)&&(g_sXingJiKongGU.bAdministration==false))
	{
		memset(platnumber_Information,0,sizeof(platnumber_Information));
		EDIT_GetText(pThisWin->hEditBox, (char *)platnumber_Information, sizeof(platnumber_Information));
		TRACE_PRINTF("platnumber_Information:%s\r\n",platnumber_Information);
		if(strlen(platnumber_Information)>0)//added by lxl 20180817
		{
			WM_DeleteWindow(pMsg->hWin);
		}
		else //added by lxl 20180926
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_WarningContent);
			s.pContent = _PleaseInput_PlateNumber;  //_PasswordError;
			s.nContentColor = GUI_RED;
			s.nBackGroundColor = GUI_LIGHTGRAY;
			s.nAutoCloseTime = 3;
			s.pFunMiddleOK = NULL;
			CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
		}
	}
	else
	{
		WM_DeleteWindow(pMsg->hWin);
	}
}
static void Alcohol_Value_Analyze_Middle(CW_InputPlateNumberWindow *pThisWin,WM_MESSAGE *pMsg)
{
	if((g_sRecord.fAlcValue>=20)&&(g_sXingJiKongGU.bAdministration==false))
	{
		memset(platnumber_Information,0,sizeof(platnumber_Information));
		EDIT_GetText(pThisWin->hEditBox, (char *)platnumber_Information, sizeof(platnumber_Information));
		TRACE_PRINTF("platnumber_Information:%s\r\n",platnumber_Information);
		if(strlen(platnumber_Information)>0)//added by lxl 20180817
		{
			_ConfirmButton_Function(pMsg);
		}
		else //added by lxl 20180926
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_WarningContent);
			s.pContent =_PleaseInput_PlateNumber;
			s.nContentColor = GUI_RED;
			s.nBackGroundColor = GUI_LIGHTGRAY;
			s.nAutoCloseTime = 3;
			s.pFunMiddleOK = NULL;
			CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
		}
	}
	else
	{
		_ConfirmButton_Function(pMsg);
	}

}

#endif
static void _Callback(WM_MESSAGE *pMsg)
{
	CW_InputPlateNumberWindow *pThisWin;
	pThisWin = (CW_InputPlateNumberWindow *)FindWMBFromListByWin(pMsg->hWin);
	
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:  
						#if SERVER_USE_XING_JIKONG_GU
						Alcohol_Value_Analyze_Left(pThisWin,pMsg);
						#else
						WM_DeleteWindow(pMsg->hWin);
						#endif
					break;
						
					case GUI_KEY_RIGHT_FUN:
						EDIT_AddKey(pThisWin->hEditBox, GUI_KEY_BACKSPACE);
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						#if SERVER_USE_XING_JIKONG_GU
						Alcohol_Value_Analyze_Middle(pThisWin,pMsg);
						#else
						_ConfirmButton_Function(pMsg);
						#endif
						break;
				}
			}
			return;
			
		case WM_NOTIFY_PARENT:
			if(WM_GetId(pMsg->hWinSrc) >= KeyIDTab[0])
			{
				if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
				{
					unsigned short Char;
					char aButtonText[20];
					char aEDITBuffer[60];
					
					/*获取按钮对应的文字信息*/
					BUTTON_GetText(pMsg->hWinSrc, aButtonText, sizeof(aButtonText));
					Char = GUI_UC_GetCharCode(aButtonText);

					EDIT_GetText(pThisWin->hEditBox, (char *)aEDITBuffer, sizeof(aEDITBuffer));
					if(strlen(aEDITBuffer) + strlen(aButtonText) <= pThisWin->hEditMaxLen)
					{
						if(strlen(aButtonText))
						{
							if((WM_GetId(pMsg->hWinSrc) >= KeyIDTab[38]) && Char != 0 && Char != '>' && Char != '<')
							{
								/*先发送一个退格消息,删除已上屏的英语字母*/
								EDIT_AddKey(pThisWin->hEditBox, GUI_KEY_BACKSPACE);
							}
							EDIT_AddKey(pThisWin->hEditBox, Char);
						}
					}

					/*仅是中文语言才有汉字选择.*/
					if(g_Language == eLanguage_Chinese)
					{
						if(WM_GetId(pMsg->hWinSrc) <= KeyIDTab[35])	
						{
							char i;
							for(i = 0; i < ARR_SIZE(KeyConvChineseTab); i++)
							{
								if(Char >= 97) Char -= 32;
								if(Char == GUI_UC_GetCharCode(KeyConvChineseTab[i].Key))
								{
									/*已查找到匹配.*/
									s_nCurDisplayFirstIndex = 1;
									s_bIncreaseOptionalChinese = 1;
									s_nTotalOptionalChinese = (unsigned long)KeyConvChineseTab[i].Key_To_Chinese[0]; /*查找对应拼音下有多少个汉字*/
									s_pKeyConvChinese = &KeyConvChineseTab[i];
									if(s_nTotalOptionalChinese < 7)
									{
										char j;
										for(j = 0; j < s_nTotalOptionalChinese; j++)
										{
											BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[38 + j]), KeyConvChineseTab[i].Key_To_Chinese[j + 1]);
										}
										for(j = j; j < 7; j++)
										{
											BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[38 + j]), "");
										}
									}
									else
									{
										/*第一页显示不下,只显示5个字*/
										char j;
										for(j = 0; j < 6; j++)
										{
											BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[38 + j]), KeyConvChineseTab[i].Key_To_Chinese[j + s_nCurDisplayFirstIndex]);
										}
										BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[44]), ">>");
									}
									return;
								}
							}
						}

						if(pMsg->hWinSrc == WM_GetDialogItem(pMsg->hWin, KeyIDTab[44]))
						{
							if(((s_nCurDisplayFirstIndex + 6) <= s_nTotalOptionalChinese) && s_bIncreaseOptionalChinese)
							{
								/*显示后五个字.*/
								char j;
								s_nCurDisplayFirstIndex += 6;
								for(j = 0; j < 6; j++)
								{
									if(s_nCurDisplayFirstIndex + j <= s_nTotalOptionalChinese)
									{
										BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[38 + j]), s_pKeyConvChinese->Key_To_Chinese[j + s_nCurDisplayFirstIndex]);
									}
									else
									{
										BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[38 + j]), "");
									}
								}
								if(s_nCurDisplayFirstIndex + 6 >= s_nTotalOptionalChinese)
								{
									BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[44]), "<<");
									s_bIncreaseOptionalChinese = 0;
								}
							}
							else if(s_nCurDisplayFirstIndex > 6 && s_bIncreaseOptionalChinese == 0)
							{
								/*显示前五个字.*/
								char j;
								s_nCurDisplayFirstIndex -= 6;
								for(j = 0; j < 6; j++)
								{
									BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[38 + j]), s_pKeyConvChinese->Key_To_Chinese[j + s_nCurDisplayFirstIndex]);
								}
								if(s_nCurDisplayFirstIndex <= 1)
								{
									BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[44]), ">>");
									s_bIncreaseOptionalChinese = 1;
								}
							}
						}
					}
				}
				return;
			}
			
			_Edit_Focus_Message(pMsg);

			if(pMsg->Data.v ==  WM_NOTIFICATION_WIN_DELETE)
			{
				WM_SetFocus(pThisWin->hEditBox);
			}
			return;
			
		case WM_DELETE:
			Voc_Close();
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eInputPlateNumWindow window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}


CW_InputPlateNumberWindow *CW_InputPlateNumWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	CW_InputPlateNumberWindow *pThisWin = NULL;
	eUserWindow WinType = eInputPlateNumWindow;

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_InputPlateNumberWindow));
	if(hWin)
	{
		TRACE_PRINTF("eInputPlateNumWindow windows is created!hWin = 0x%04X\r\n",hWin);

		pThisWin = (CW_InputPlateNumberWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

		pThisWin->wObj.hWin = hWin;
		pThisWin->wObj.Property = (unsigned long)pData;
		pThisWin->wObj.eWinType = (eUserWindow)WinType;
		
		AddWindowToList(&pThisWin->wObj);

		pThisWin->sProperty = *(sInputWindowProperty*)pData;
		
		WM_SetCallback(hWin, _Callback);
		WM__SendMsgNoData(hWin, WM_CREATE);
	}
	else
	{
		TRACE_PRINTF("eInputPlateNumWindow window can't be created!\r\n");
	}

	return pThisWin;
}

