#include "platform.h"
#include "Windows.h"
#include "rtc.h"
#include "time_calc.h"
#include "EDIT_Private.h"
#include "data.h"
#include "test.h"
#include "app.h"

#define INITIAL_XINGJI     0

#if SYSTEM_LANGUAGE == LAN_ENGLISH

#define USE_UTF8_CODE	

#define Btn_Width		22
#define Btn_Height		34
#define Btn_H_Interval 	2
#define Btn_V_Interval 	5
#define X_START			(LCD_XSIZE-((Btn_Width*10)+Btn_H_Interval*9)>>1)
#define Y_START			80

typedef enum
{
	eKeyTypeEnglish = 0,		//英文字母
	eKeyTypeSymbo,		//符号
	eKeyTypeChinese,		//中文拼音
} eKeyType;

//中文拼音汉字选择表
static const char *Btn_PyCnTab[] =
{
	"<<", "", "", "", "", "", "", "", "", ">>",
};

//英文字母表
static const char *Btn_EnglishTab[] =
{
	"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P",
	"A", "S", "D", "F", "G", "H", "J", "K", "L",
	"Z", "X", "C", "V", "B", "N", "M",
	"q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
	"a", "s", "d", "f", "g", "h", "j", "k", "l",
	"z", "x", "c", "v", "b", "n", "m",

};

//中文拼音字母表
static const char *Btn_PyTab[] =
{
	"q", "w", "e", "r", "t", "y", "u", "i", "o", "p",
	"a", "s", "d", "f", "g", "h", "j", "k", "l",
	"z", "x", "c", "v", "b", "n", "m",
};

//数字表
static const char *Btn_DigitTab[] =
{
	"1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
};

//符号表
static const char *Btn_SymboTab[] =
{
	"!", "@", "#", "$", "%", "^", "&", "*", "(", ")",
	"<", ">", "[", "]", "\\", "/", "+", "-", "=", "_",
	"\"", "?", ":", ";", ",", ".",
};

static const unsigned short KeyIDTab[] =
{
	/*10个拼音选择键******************************************/
	10000, 10001, 10002, 10003, 10004, 10005, 10006, 10007, 10008, 10009,
	
	/*10个数字键************************************************/
	10010, 10011, 10012, 10013, 10014, 10015, 10016, 10017, 10018, 10019,

	//QWERTYUIOP
	10020, 10021, 10022, 10023, 10024, 10025, 10026, 10027, 10028, 10029,
	
	//ASDFGHJKL
	10030, 10031, 10032, 10033, 10034, 10035, 10036, 10037, 10038,
	
	//ZXCVBNM
	10039, 10040, 10041, 10042, 10043, 10044, 10045,
	
	/*英语字母输入选择键*****************************************/
	10046,
	
	/*Shift键***************************************************/
	10047,

	/*数字标点输入选择键*/
	10048,

	/*空格键*/
	10049,

	/*拼音输入选择键*/
	10050,
};


#if 0

#else

static void _ConfirmButton_Function(WM_MESSAGE *pMsg)
{	
	sInputWindowProperty s;
	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);
	
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
	
	switch(pThisWin->sProperty.nWinType)
	{
		case ePlateNumber:
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aPlateNumber, sizeof(g_sRecord.aPlateNumber));
			s.nWinType = eInputTestee;
			s.aCaptionText = _TesteeNameInput;
			CW_InputWindow_Create(pMsg->hWin,(void *)&s);
			break;
			
		case eInputTestee:
			memset(g_sRecord.aTestee,0,sizeof(g_sRecord.aTestee));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aTestee, sizeof(g_sRecord.aTestee));
			s.nWinType = eInputDriverLicence;
			s.aCaptionText = _PleaseInputDriveLicence;
			CW_InputWindow_Create(pMsg->hWin,(void *)&s);
			break;

		case eInputDriverLicence:
			memset(g_sRecord.aDriverLicense,0,sizeof(g_sRecord.aDriverLicense));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aDriverLicense, sizeof(g_sRecord.aDriverLicense));
			s.nWinType = eTestLocation;
			s.aCaptionText = _TestLocationInput;
			CW_InputWindow_Create(pMsg->hWin, (void *)&s);
			break;

		case eTestLocation:
			memset(g_sRecord.aLocation,0,sizeof(g_sRecord.aLocation));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aLocation, sizeof(g_sRecord.aLocation));
			s.nWinType = eInputPoliceName;
			s.aCaptionText = _PoliceNameInput;
			CW_InputWindow_Create(pMsg->hWin,(void *)&s);
			break;

		case eInputPoliceName:
			memset(g_sRecord.aPoliceName,0,sizeof(g_sRecord.aPoliceName));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aPoliceName, sizeof(g_sRecord.aPoliceName));
			s.nWinType = eInputPoliceID;
			s.aCaptionText = _PleaseInputPoliceID;
			CW_InputWindow_Create(pMsg->hWin,(void *)&s);
			break;

		case eInputPoliceID:
			memset(g_sRecord.aPoliceID,0,sizeof(g_sRecord.aPoliceID));
			EDIT_GetText(pThisWin->hEditBox,(char *)g_sRecord.aPoliceID, sizeof(g_sRecord.aPoliceID));
			s.nWinType = eInputDepartment;
			s.aCaptionText = _DepartmentInput;
			CW_InputWindow_Create(pMsg->hWin, (void *)&s);
			break;

		case eInputDepartment:
			memset(g_sRecord.aDepartment,0,sizeof(g_sRecord.aDepartment));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aDepartment, sizeof(g_sRecord.aDepartment));
			if(g_sRecord.nTestMode == eTestRefuseMode)
			{
				/*如果是拒测,则在此需要标志有新数据要写.*/
				pApp->bNewDataNeedSave = true;
			}
			Data_Record_Full_Save();
			{
				sWinManageList * pWM;
				pWM = FindWMBByWinType(eUserMain);
				WM_DeleteWindow(pWM->pNext->hWin);
			}
			CW_DisplayRecordWindow_Create(GetLastWinFromList(),(void *)1);
			break;
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;

	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);

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

	/*创建输入编辑框*/
	pThisWin->hEditBox = EDIT_CreateEx(X_START, 34, LCD_XSIZE - X_START * 2, 33,
				                     pMsg->hWin, WM_CF_SHOW, 0, 0, 100);                     
	EDIT_SetFont(pThisWin->hEditBox, &GUI_Font16_UTF8);
	EDIT_SetTextAlign(pThisWin->hEditBox, GUI_TA_VCENTER);
	EDIT_SetTextColor(pThisWin->hEditBox, 0, GUI_RED);

	switch(pThisWin->sProperty.nWinType)
	{
		case ePlateNumber:
			Voc_Cmd(eVOC_INPUT_PLATE);
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aPlateNumber) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;
			
		case eInputTestee:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aTestee) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;

		case eInputDriverLicence:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aDriverLicense) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			break;
		
		case eTestLocation:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aLocation) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aLocation);
			break;
			
		case eInputPoliceName:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aPoliceName) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aPoliceName);
			break;

		case eInputPoliceID:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aPoliceID) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aPoliceID);
			break;

		case eInputDepartment:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aDepartment) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aDepartment);
			break;
	}

	WM_SetFocus(pThisWin->hEditBox);
}


#endif

static void _Edit_Focus_Message(WM_MESSAGE *pMsg)
{
	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);
	
	if(pMsg->Data.v == WM_NOTIFICATION_GOT_FOCUS)
	{
		if(pMsg->hWinSrc == pThisWin->hEditBox)
		{
			char x;
			WM_HWIN hWin;
			pThisWin->nInputType = eKeyTypeEnglish;

			//中文拼音汉字选择 ２方向键,8个选择键  总共10个键----------------------------------------
			for(x = 0; x < 10; x++)
			{
				hWin = BUTTON_CreateEx(1 + x * (22 + Btn_H_Interval), Y_START, 22, Btn_Height,
				                       pMsg->hWin,
				                       WM_CF_SHOW, 1, KeyIDTab[x]);
				BUTTON_SetFont(hWin, &GUI_Font16_UTF8);
				BUTTON_SetText(hWin, Btn_PyCnTab[x]);
				BUTTON_SetFocussable(hWin, 0);
				BUTTON_SetBkColor(hWin, 0, 0xB59179);
				BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
				BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
				BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
				BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
				WM_HideWindow(hWin);
			}

			//	1 2 3 4 5 6 7 8 9 0 键	10个键
			for(x = 0; x < 10; x++)
			{
				hWin = BUTTON_CreateEx(X_START + x * (Btn_Width + Btn_H_Interval), Y_START + 0 * (Btn_Height + Btn_V_Interval), Btn_Width, Btn_Height,
				                       pMsg->hWin,
				                       WM_CF_SHOW, 1, KeyIDTab[10 + x]);           
				BUTTON_SetFont(hWin, &GUI_Font16_UTF8);
			
				BUTTON_SetText(hWin, Btn_DigitTab[x]);
				BUTTON_SetFocussable(hWin, 0);

				BUTTON_SetBkColor(hWin, 0, 0xB59179);
				BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
				BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
				BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
				BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
			}

			//	Q W E R T Y U I O P 键  10个键
			for(x = 0; x < 10; x++)
			{
				hWin = BUTTON_CreateEx(X_START + x * (Btn_Width + Btn_H_Interval), Y_START + 1 * (Btn_Height + Btn_V_Interval), Btn_Width, Btn_Height,
				                       pMsg->hWin,
				                       WM_CF_SHOW, 1, KeyIDTab[20 + x]);
				BUTTON_SetFont(hWin, &GUI_Font16_UTF8);

				if(pThisWin->nInputType == eKeyTypeChinese)
				{
					BUTTON_SetText(hWin, Btn_PyTab[x]);
				}
				else if(pThisWin->nInputType == eKeyTypeSymbo)
				{
					BUTTON_SetText(hWin, Btn_SymboTab[x]);
				}
				else
				{
				#if defined(CUST_CHINA_JXHK)
					BUTTON_SetText(hWin, Btn_EnglishTab[x + 26]);
				#else
					BUTTON_SetText(hWin, Btn_EnglishTab[x]);
				#endif
				}
				BUTTON_SetFocussable(hWin, 0);
				BUTTON_SetBkColor(hWin, 0, 0xFFB5B5);
				BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
				BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
				BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
				BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
			}

			// A S D F G H J K L 键 9个键
			for(x = 0; x < 9; x++)
			{
				hWin = BUTTON_CreateEx((X_START + (Btn_Width + Btn_H_Interval) / 2) + x * (Btn_Width + Btn_H_Interval), Y_START + 2 * (Btn_Height + Btn_V_Interval), Btn_Width, Btn_Height,
				                       pMsg->hWin,
				                       WM_CF_SHOW, 1, KeyIDTab[30 + x]);

				BUTTON_SetFont(hWin, &GUI_Font16_UTF8);

				if(pThisWin->nInputType == eKeyTypeChinese)
				{
					BUTTON_SetText(hWin, Btn_PyTab[10 + x]);
				}
				else if(pThisWin->nInputType == eKeyTypeSymbo)
				{
					BUTTON_SetText(hWin, Btn_SymboTab[10 + x]);
				}
				else
				{
				#if defined(CUST_CHINA_JXHK)
					BUTTON_SetText(hWin, Btn_EnglishTab[10 + x + 26]);
				#else
					BUTTON_SetText(hWin, Btn_EnglishTab[10 + x]);
				#endif
				}

				BUTTON_SetFocussable(hWin, 0);
				BUTTON_SetBkColor(hWin, 0, 0xFFB5B5);
				BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
				BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
				BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
				BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
			}

			// Z X C V B N M  键 7个键
			for(x = 0; x < 7; x++)
			{
				hWin = BUTTON_CreateEx((X_START + (Btn_Width + Btn_H_Interval) + (Btn_Width + Btn_H_Interval) / 2) + x * (Btn_Width + Btn_H_Interval), Y_START + 3 * (Btn_Height + Btn_V_Interval), Btn_Width, Btn_Height,
				                       pMsg->hWin,
				                       WM_CF_SHOW, 1, KeyIDTab[39 + x]);
				BUTTON_SetFont(hWin, &GUI_Font16_UTF8);
	
				if(pThisWin->nInputType == eKeyTypeChinese)
				{
					BUTTON_SetText(hWin, Btn_PyTab[19 + x]);
				}
				else if(pThisWin->nInputType == eKeyTypeSymbo)
				{
					BUTTON_SetText(hWin, Btn_SymboTab[19 + x]);
				}
				else
				{
				#if defined(CUST_CHINA_JXHK)
					BUTTON_SetText(hWin, Btn_EnglishTab[19 + x + 26]);
				#else
					BUTTON_SetText(hWin, Btn_EnglishTab[19 + x]);
				#endif
				}


				BUTTON_SetFocussable(hWin, 0);
				BUTTON_SetBkColor(hWin, 0, 0xFFB5B5);
				BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
				BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
				BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
				BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
			}

			//标点符号------------------------------------------------------------
			hWin = BUTTON_CreateEx(X_START, Y_START + 3 * (Btn_Height + Btn_V_Interval) , Btn_Width + (Btn_Width + Btn_H_Interval) / 2, Btn_Height,
			                       pMsg->hWin,
			                       WM_CF_SHOW, 1, KeyIDTab[46]);                   
			BUTTON_SetFont(hWin, &GUI_Font16_UTF8);


			BUTTON_SetText(hWin, ".?");
			BUTTON_SetFocussable(hWin, 0);
			BUTTON_SetBkColor(hWin, 0, 0xB59179);
			BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
			if(pThisWin->nInputType == eKeyTypeSymbo)
			{
				BUTTON_SetTextColor(hWin, 0, GUI_RED);
			}
			else
			{
				BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
			}
			BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
			BUTTON_SetTextColor(hWin, 2, GUI_GRAY);

			//英文字母键------------------------------------------------------------------
			hWin = BUTTON_CreateEx(X_START + (Btn_Width + (Btn_Width + Btn_H_Interval) / 2) + Btn_H_Interval + 7 * (Btn_Width + Btn_H_Interval), Y_START + 3 * (Btn_Height + Btn_V_Interval), LCD_XSIZE - (X_START + (Btn_Width + (Btn_Width + Btn_H_Interval) / 2) + Btn_H_Interval + 7 * (Btn_Width + Btn_H_Interval)) - X_START, Btn_Height,
			                       pMsg->hWin,
			                       WM_CF_SHOW, 1, KeyIDTab[47]);
			BUTTON_SetFont(hWin, &GUI_Font16_UTF8);

			BUTTON_SetText(hWin, "En");
			BUTTON_SetFocussable(hWin, 0);
			BUTTON_SetBkColor(hWin, 0, 0xB59179);
			BUTTON_SetBkColor(hWin, 1, 0x5D4E41);

			if(pThisWin->nInputType == eKeyTypeEnglish)
			{
				BUTTON_SetTextColor(hWin, 0, GUI_RED);
			}
			else
			{
				BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
			}
			BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
			BUTTON_SetTextColor(hWin, 2, GUI_GRAY);



			//Shift键-------------------------------------------------------------------------
			hWin = BUTTON_CreateEx(X_START, Y_START + 4 * (Btn_Height + Btn_V_Interval), Btn_Width + (Btn_Width + Btn_H_Interval) / 2 + Btn_H_Interval + Btn_Width, Btn_Height,
			                       pMsg->hWin,
			                       WM_CF_SHOW, 1, KeyIDTab[48]);
			BUTTON_SetFont(hWin, &GUI_Font16_UTF8);

			BUTTON_SetText(hWin, "Shift");
			BUTTON_SetFocussable(hWin, 0);
			BUTTON_SetBkColor(hWin, 0, 0xB59179);
			BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
			BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
			BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
			BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
			//Shift键使能
			if((pThisWin->nInputType == eKeyTypeEnglish && ARR_SIZE(Btn_EnglishTab) <= 26) || (pThisWin->nInputType == eKeyTypeSymbo && ARR_SIZE(Btn_SymboTab) <= 26) || (pThisWin->nInputType == eKeyTypeChinese && ARR_SIZE(Btn_PyTab) <= 26))
			{
				WM_DisableWindow(WM_GetDialogItem(pMsg->hWin, KeyIDTab[48]));
			}
			else
			{
				WM_EnableWindow(WM_GetDialogItem(pMsg->hWin, KeyIDTab[48]));
			}
			

			//空格键--------------------------------------------------------------------------
			hWin = BUTTON_CreateEx(X_START + (Btn_Width + (Btn_Width + Btn_H_Interval) / 2 + Btn_H_Interval + Btn_Width) + Btn_H_Interval, Y_START + (Btn_Height + Btn_V_Interval) * 4 , (Btn_Width + Btn_H_Interval) * 5 - Btn_H_Interval, Btn_Height,
			                       pMsg->hWin,
			                       WM_CF_SHOW, 1, KeyIDTab[49]);
			BUTTON_SetFont(hWin, &GUI_Font16_UTF8);

			BUTTON_SetText(hWin, "Space");
			BUTTON_SetFocussable(hWin, 0);
			//BUTTON_SetBkColor(hWin, 0, 0xB59179);
			BUTTON_SetBkColor(hWin, 0, 0xFFB5B5);
			BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
			BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
			BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
			BUTTON_SetTextColor(hWin, 2, GUI_GRAY);

			//拼音汉字选择键---------------------------------------------------------------------
			hWin = BUTTON_CreateEx(X_START + (Btn_Width + (Btn_Width + Btn_H_Interval) / 2 + Btn_H_Interval + Btn_Width) + Btn_H_Interval + ((Btn_Width + Btn_H_Interval) * 5 - Btn_H_Interval) + Btn_H_Interval, Y_START + (Btn_Height + Btn_V_Interval) * 4 , Btn_Width + (Btn_Width + Btn_H_Interval) / 2 + Btn_H_Interval + Btn_Width, Btn_Height,
			                       pMsg->hWin,
			                       WM_CF_SHOW, 1, KeyIDTab[50]);
			BUTTON_SetFont(hWin, &GUI_Font16_UTF8);

			BUTTON_SetText(hWin, "");
			BUTTON_SetFocussable(hWin, 0);
			BUTTON_SetBkColor(hWin, 0, 0xB59179);
			BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
			WM_DisableWindow(hWin);
		}
	}
	else if(pMsg->Data.v == WM_NOTIFICATION_LOST_FOCUS)
	{
		if(pMsg->hWinSrc == pThisWin->hEditBox)
		{
			char i;
			WM_HWIN hWin;
			for(i = 0; i < (sizeof(KeyIDTab) / sizeof(unsigned short)); i++)
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

static void _Callback(WM_MESSAGE *pMsg)
{
	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);
	
	Enter_Callback_Hook(pMsg);
	switch(pMsg->MsgId)
	{
		case WM_CREATE:
			_Create(pMsg);
			break;

		case WM_PAINT:
			break;
			
		case WM_KEY:
			if(WM_Key_Status == 0)
			{
				switch (WM_Key_Code)
				{
					case GUI_KEY_LEFT_FUN:
						WM_DeleteWindow(pMsg->hWin);
						break;
						
					case GUI_KEY_RIGHT_FUN:
						EDIT_AddKey(pThisWin->hEditBox, GUI_KEY_BACKSPACE);
						break;
						
					case GUI_KEY_MIDDLE_FUN:
						_ConfirmButton_Function(pMsg);
						break;
				}
			}
			return;

			
		case WM_NOTIFY_PARENT:
			if(WM_GetId(pMsg->hWinSrc) >= KeyIDTab[0])
			{
				if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
				{
					char aButtonText[20] = {0};
					/*获取按钮对应的文字信息*/
					BUTTON_GetText(pMsg->hWinSrc, aButtonText, sizeof(aButtonText));
					
					if(WM_GetId(pMsg->hWinSrc) >= KeyIDTab[10] && WM_GetId(pMsg->hWinSrc) <= KeyIDTab[19])
					{
						char aEDITBuffer[60];
						EDIT_GetText(pThisWin->hEditBox, (char *)aEDITBuffer, sizeof(aEDITBuffer));
						if(strlen(aEDITBuffer) + strlen(aButtonText) <= pThisWin->hEditMaxLen)
						{
							if(strlen(aButtonText))
							{
								EDIT_AddKey(pThisWin->hEditBox, GUI_UC_GetCharCode(aButtonText));
							}
						}
					}
					//英文字母 拼音字母 标点*********************************************************************
					else if(WM_GetId(pMsg->hWinSrc) >= KeyIDTab[20] && WM_GetId(pMsg->hWinSrc) <= KeyIDTab[45])
					{
						char aEDITBuffer[60];
						EDIT_GetText(pThisWin->hEditBox, (char *)aEDITBuffer, sizeof(aEDITBuffer));
						if(strlen(aEDITBuffer) + strlen(aButtonText) <= pThisWin->hEditMaxLen)
						{
							if(strlen(aButtonText))
							{
								EDIT_AddKey(pThisWin->hEditBox, GUI_UC_GetCharCode(aButtonText));
							}
						}
					}
					//*选择标点输入******************************************************************************************
					else if(WM_GetId(pMsg->hWinSrc) == KeyIDTab[46])
					{
						if(pThisWin->nInputType != eKeyTypeSymbo)
						{
							char w = 0;
							unsigned short nNumber = ARR_SIZE(Btn_SymboTab);
							pThisWin->nInputType = eKeyTypeSymbo;

							//拼音字母按键
							for(w = 0; w < 26; w++)
							{
								if(w < nNumber)
								{
									BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[20 + w]), Btn_SymboTab[w]);
								}
								else
								{
									BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[20 + w]), "");
								}
							}

							//键盘类型选择键
							BUTTON_SetTextColor(WM_GetDialogItem(pMsg->hWin, KeyIDTab[50]), 0, GUI_BLACK); //拼音
							BUTTON_SetTextColor(WM_GetDialogItem(pMsg->hWin, KeyIDTab[46]), 0, GUI_RED); 	//标点
							BUTTON_SetTextColor(WM_GetDialogItem(pMsg->hWin, KeyIDTab[47]), 0, GUI_BLACK); //英文字母

							//Shift键使能
							if(nNumber <= 26)
							{
								WM_DisableWindow(WM_GetDialogItem(pMsg->hWin, KeyIDTab[48]));
							}
							else
							{
								WM_EnableWindow(WM_GetDialogItem(pMsg->hWin, KeyIDTab[48]));
							}
						}
					}
					//*选择英文字母输入*************************************************************************************
					else if(WM_GetId(pMsg->hWinSrc) == KeyIDTab[47])
					{
						if(pThisWin->nInputType != eKeyTypeEnglish)
						{
							char w = 0;
							unsigned short nNumber = ARR_SIZE(Btn_EnglishTab);
							pThisWin->nInputType = eKeyTypeEnglish;

							//拼音字母按键
							for(w = 0; w < 26; w++)
							{
								if(w < nNumber)
								{
									BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[20 + w]), Btn_EnglishTab[w]);
								}
								else
								{
									BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[20 + w]), "");
								}
							}

							//键盘类型选择键
							BUTTON_SetTextColor(WM_GetDialogItem(pMsg->hWin, KeyIDTab[50]), 0, GUI_BLACK); //拼音
							BUTTON_SetTextColor(WM_GetDialogItem(pMsg->hWin, KeyIDTab[46]), 0, GUI_BLACK); //标点
							BUTTON_SetTextColor(WM_GetDialogItem(pMsg->hWin, KeyIDTab[47]), 0, GUI_RED); //英文字母


							//Shift键使能
							if(nNumber <= 26)
							{
								WM_DisableWindow(WM_GetDialogItem(pMsg->hWin, KeyIDTab[48]));
							}
							else
							{
								WM_EnableWindow(WM_GetDialogItem(pMsg->hWin, KeyIDTab[48]));
							}
						}
					}
					//*Shift键***********************************************************************************************
					else if(WM_GetId(pMsg->hWinSrc) == KeyIDTab[48])
					{	
						char w;
						unsigned short nOffset;
						char aButtonTextBuffer[20] = {0};

						/*获取第1字母按钮对应的文字信息*/
						BUTTON_GetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[20]), aButtonTextBuffer, sizeof(aButtonTextBuffer));

						if(pThisWin->nInputType == eKeyTypeEnglish && ARR_SIZE(Btn_EnglishTab) > 26) //英文字母输入Shift切换
						{
							if(strcmp(aButtonTextBuffer,Btn_EnglishTab[0]) == 0)
							{
								nOffset = 26;
							}
							else
							{
								nOffset = 0;
							}

							for(w = 0; w < 26; w++)
							{
								BUTTON_SetText(WM_GetDialogItem(pMsg->hWin, KeyIDTab[20 + w]), Btn_EnglishTab[w + nOffset]);
							}
						}
					}
					//*发送ADDKEY消息 空格键***************************************************************************
					else if(WM_GetId(pMsg->hWinSrc) == KeyIDTab[49])
					{
						char aEDITBuffer[60];
						EDIT_GetText(pThisWin->hEditBox, (char *)aEDITBuffer, sizeof(aEDITBuffer));
						if(strlen(aEDITBuffer) + 1 <= pThisWin->hEditMaxLen)
						{
							if(strlen(aButtonText))
							{
								EDIT_AddKey(pThisWin->hEditBox, ' ');
							}
						}
					}
					//*选择拼音输入*************************************************************************************
					else if(WM_GetId(pMsg->hWinSrc) == KeyIDTab[50])
					{
					
					}
				}
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
			TRACE_PRINTF("eInputWindow window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}

CW_InputWindow *CW_InputWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	CW_InputWindow *pThisWin = NULL;
	eUserWindow WinType = eInputWindow;

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_InputWindow));
	if(hWin)
	{
		TRACE_PRINTF("eInputWindow windows is created!hWin = 0x%04X\r\n",hWin);

		pThisWin = (CW_InputWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

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
		TRACE_PRINTF("eInputWindow window can't be created!\r\n");
	}

	return pThisWin;
}

#else

typedef enum
{
	eKeyTypeEnglish = 0,		//英文字母
	eKeyTypeSymbo,		//符号
	eKeyTypeChinese,		//中文拼音
	eKeyTypeNumber,		//数字
} eKeyType;

static void _ConfirmButton_Function(WM_MESSAGE *pMsg)
{	
	sInputWindowProperty s;
	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);
	
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
	switch(pThisWin->sProperty.nWinType)
	{
		case eInputTestee:
			memset(g_sRecord.aTestee,0,sizeof(g_sRecord.aTestee));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aTestee, sizeof(g_sRecord.aTestee));
			s.nWinType = eInputDriverLicence;
		#if defined(CUST_CHINA_SZJAZN)
			s.aCaptionText = "请输入证件号";
		#else
			s.aCaptionText = _PleaseInputDriveLicence;
		#endif
			CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
			break;

		case eTestLocation:
			memset(g_sRecord.aLocation,0,sizeof(g_sRecord.aLocation));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aLocation, sizeof(g_sRecord.aLocation));
		#if USE_GB_T21254_2017
			if(pApp->bLoginAsAdministrator)
			{
				s.nWinType = eInputPoliceName;
				s.aCaptionText = _PoliceNameInput;
				CW_InputWindow_Create(pMsg->hWin,(void *)&s);
			}
			else
			{
				if(g_sRecord.nTestMode == eTestRefuseMode)
				{
					/*如果是拒测,则在此需要标志有新数据要写.*/
					pApp->bNewDataNeedSave = true;
				}
				Data_Record_Full_Save();
				{
					sWinManageList * pWM;
					pWM = FindWMBByWinType(eUserMain);
					WM_DeleteWindow(pWM->pNext->hWin);
				}
				#if SERVER_USE_XING_JIKONG_GU
				/*如果警员ID和部门不需要输入时，当测试值大于20时，必须要清标志位，否则关不了机了*/
				g_sXingJiKongGU.bAlcValue_Excessive_Flag=false;//超标后所有信息录入完成后重新置0  added by lxl 20180821
				#endif
				CW_DisplayRecordWindow_Create(GetLastWinFromList(),(void *)1);
			}
		#else
			s.nWinType = eInputPoliceName;
			s.aCaptionText = _PoliceNameInput;
			CW_InputWindow_Create(pMsg->hWin,(void *)&s);
		#endif
			break;

		case eInputPoliceName:
			memset(g_sRecord.aPoliceName,0,sizeof(g_sRecord.aPoliceName));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aPoliceName, sizeof(g_sRecord.aPoliceName));
			s.nWinType = eInputPoliceID;
			s.aCaptionText = _PleaseInputPoliceID;
			CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
			break;

		case eInputDepartment:
		#if defined(CUST_CHINA_HBBJYXGS)
			memset(g_sRecord.aDepartment,0,sizeof(g_sRecord.aDepartment));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aDepartment, sizeof(g_sRecord.aDepartment));
			if(GetWinHandleByWinType(eShowReport))
			{
				sWinManageList * pWM;
				pWM = FindWMBByWinType(eUserMain);
				WM_DeleteWindow(pWM->pNext->hWin);
				Data_Record_Modify(&g_sRecord);
				CW_DisplayRecordWindow_Create(GetLastWinFromList(),(void *)1);
			}
			else
			{
				sWinManageList * pWM;
				pWM = FindWMBByWinType(eInputPlateNumWindow);
				WM_DeleteWindow(pWM->hWin);
				Data_Record_Modify(&g_sRecord);
			}
		#else
			memset(g_sRecord.aDepartment,0,sizeof(g_sRecord.aDepartment));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aDepartment, sizeof(g_sRecord.aDepartment));
			if(g_sRecord.nTestMode == eTestRefuseMode)
			{
				/*如果是拒测,则在此需要标志有新数据要写.*/
				pApp->bNewDataNeedSave = true;
			}
			Data_Record_Full_Save();
			{
				sWinManageList * pWM;
				pWM = FindWMBByWinType(eUserMain);
				WM_DeleteWindow(pWM->pNext->hWin);
			}
			#if SERVER_USE_XING_JIKONG_GU
			g_sXingJiKongGU.bAlcValue_Excessive_Flag=false;//g_nAlcValue_Excessive_Flag=false;//超标后所有信息录入完成后重新置0  added by lxl 20180821
			#endif
			CW_DisplayRecordWindow_Create(GetLastWinFromList(),(void *)1);
		#endif
			break;

		case eInputLoginName:
			memset(g_sCurrPoliceUserInfor.aPoliceName,0,sizeof(g_sCurrPoliceUserInfor.aPoliceName));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sCurrPoliceUserInfor.aPoliceName, sizeof(g_sCurrPoliceUserInfor.aPoliceName));
			if(strlen((char *)g_sCurrPoliceUserInfor.aPoliceName) == 0)
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);
				s.pContent = _AlertInputValidName;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 3;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
				return;
			}
			s.nWinType = eInputLoginID;
			s.aCaptionText = _PleaseInputID;
			CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
			break;

		case eInputLoginDepartment:
			pThisWin->hEditMaxLen = sizeof(g_sCurrPoliceUserInfor.aDepartment) - 1;
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sCurrPoliceUserInfor.aDepartment, sizeof(g_sCurrPoliceUserInfor.aDepartment));
			s.nWinType = eSetLoginPassword;
			s.aCaptionText = _PleaseSetLoginPassword;
			CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
			break;
	}
}

static void _Edit_Focus_Message(WM_MESSAGE *pMsg)
{
	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);
		
	if(g_Language == eLanguage_Chinese)
	{
		switch(pThisWin->sProperty.nWinType)
		{
			case eInputTestee:
			case eTestLocation:
			case eInputPoliceName:
			case eInputDepartment:
			case eInputLoginName:
			case eInputLoginDepartment:
				pThisWin->nInputType = eKeyTypeChinese;
				break;
		}
	}
	else
	{
		switch(pThisWin->sProperty.nWinType)
		{
			case eInputTestee:
			case eTestLocation:
			case eInputPoliceName:
			case eInputDepartment:
				pThisWin->nInputType = eKeyTypeEnglish;
				break;
		}
	}
}

void _Create_Function(WM_MESSAGE *pMsg)
{
	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);

	switch(pThisWin->sProperty.nWinType)
	{
		case eInputTestee:
			Voc_Cmd(eVOC_INPUT_TESTER_NAME);
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aTestee) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
		#if defined(CUST_CHINA_HBBJYXGS)
			EDIT_SetText(pThisWin->hEditBox, (char *)g_sRecord.aTestee);
		#endif
			break;
			
		case eTestLocation:
			Voc_Cmd(eVOC_INPUT_TEST_LOCATION);
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aLocation) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
		#if defined(CUST_CHINA_HBBJYXGS)
			EDIT_SetText(pThisWin->hEditBox, (char *)g_sRecord.aLocation);
		#else
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aLocation);
		#endif
			break;
			
		case eInputPoliceName:
			Voc_Cmd(eVOC_INPUT_POLICE_NAME);
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aPoliceName) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
		#if defined(CUST_CHINA_HBBJYXGS)
			EDIT_SetText(pThisWin->hEditBox, (char *)g_sRecord.aPoliceName);
		#else
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aPoliceName);
		#endif
			break;

		case eInputDepartment:
			Voc_Cmd(eVOC_INPUT_DEPARTMENT);
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aDepartment) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
		#if defined(CUST_CHINA_HBBJYXGS)
			EDIT_SetText(pThisWin->hEditBox, (char *)g_sRecord.aDepartment);
		#else
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aDepartment);
		#endif
			break;

		case eInputLoginName:
			pThisWin->hEditMaxLen = sizeof(g_sCurrPoliceUserInfor.aPoliceName) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;

		case eInputLoginDepartment:
			pThisWin->hEditMaxLen = sizeof(g_sCurrPoliceUserInfor.aDepartment) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	WM_HWIN hWin;

	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);

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

	/*创建输入编辑框*/
	pThisWin->hEditBox = EDIT_CreateEx(3, 34, LCD_XSIZE - 3 * 2, 33,
				                     pMsg->hWin, WM_CF_SHOW, 0, 0, 100);                     
	EDIT_SetFont(pThisWin->hEditBox, &GUI_Font16_UTF8);
	EDIT_SetTextAlign(pThisWin->hEditBox, GUI_TA_VCENTER);
	EDIT_SetTextColor(pThisWin->hEditBox, 0, GUI_RED);

	/*输入拼音预览*/
	pThisWin->hText = TEXT_CreateEx(3 + 3, 71 - 1 - 16, LCD_XSIZE - 3 * 2, 17, pMsg->hWin, WM_CF_SHOW, 0, 0, "");
	TEXT_SetFont(pThisWin->hText, &GUI_FontTahoma16);
	TEXT_SetTextColor(pThisWin->hText, GUI_BLUE);
	TEXT_SetBkColor(pThisWin->hText, GUI_INVALID_COLOR);
	
	_Create_Function(pMsg);
	
	WM_SetFocus(pThisWin->hEditBox);
}

#if SERVER_USE_XING_JIKONG_GU
static char User_Police_Information[50];//added by lxl 
static char _PleaseInput_Tester_Name[]={0xE8,0xAF,0xB7,0xE8,0xBE,0x93,0xE5,0x85,0xA5,0xE8,0xA2,0xAB,0xE6,0xB5,0x8B,0xE8,0xAF,0x95,0xE4,0xBA,0xBA,0xE5,0xA7,0x93,0xE5,0x90,0x8D,0xEF,0xBC,0x81,0x00};   /*请输入被测试人姓名！*/
static char _PleaseInput_Test_Location[]={0xE8,0xAF,0xB7,0xE8,0xBE,0x93,0xE5,0x85,0xA5,0xE6,0xB5,0x8B,0xE8,0xAF,0x95,0xE5,0x9C,0xB0,0xE7,0x82,0xB9,0xEF,0xBC,0x81,0x00};   /*请输入测试地点！*/
static char _PleaseInput_Police_Name[]= {0xE8,0xAF,0xB7,0xE8,0xBE,0x93,0xE5,0x85,0xA5,0xE6,0x89,0xA7,0xE5,0x8B,0xA4,0xE6,0xB0,0x91,0xE8,0xAD,0xA6,0xE5,0xA7,0x93,0xE5,0x90,0x8D,0xEF,0xBC,0x81,0x00};   /*请输入执勤民警姓名！*/
static char _PleaseInput_Police_Department[]={0xE8,0xAF,0xB7,0xE8,0xBE,0x93,0xE5,0x85,0xA5,0xE6,0x89,0xA7,0xE6,0xB3,0x95,0xE9,0x83,0xA8,0xE9,0x97,0xA8,0xEF,0xBC,0x81,0x00};   /*请输入执法部门！*/
static void Alcohol_Value_Analyze_Left(CW_InputWindow *pThisWin,WM_MESSAGE *pMsg)
{
	if((g_sRecord.fAlcValue>=20)&&(g_sXingJiKongGU.bAdministration==false))
	{
		memset(User_Police_Information,0,sizeof(User_Police_Information));
		EDIT_GetText(pThisWin->hEditBox, (char *)User_Police_Information, sizeof(User_Police_Information));
		TRACE_PRINTF("User_Police_Information:%s\r\n",User_Police_Information);
		if(strlen(User_Police_Information)>0)
		{
			if(pMsg->hWin != GetFirstWinFromList())
			{
				WM_DeleteWindow(pMsg->hWin);
			}
		}
		else
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_WarningContent);
			switch(pThisWin->sProperty.nWinType)
			{
				case eInputTestee:
					s.pContent = _PleaseInput_Tester_Name; 
					break;
					
				case eTestLocation:
					s.pContent = _PleaseInput_Test_Location; 
					break;

				case eInputPoliceName:
					s.pContent = _PleaseInput_Police_Name; 
					break;

				case eInputDepartment:
					s.pContent = _PleaseInput_Police_Department; 
					break;
			}
			s.nContentColor = GUI_RED;
			s.nBackGroundColor = GUI_LIGHTGRAY;
			s.nAutoCloseTime = 3;
			s.pFunMiddleOK = NULL;
			CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
		}
	}
	else
	{
		if(pMsg->hWin != GetFirstWinFromList())
		{
			WM_DeleteWindow(pMsg->hWin);
		}
	}
}
static void Alcohol_Value_Analyze_Middle(CW_InputWindow *pThisWin,WM_MESSAGE *pMsg)
{
	if((g_sRecord.fAlcValue>=20)&&(g_sXingJiKongGU.bAdministration==false))
	{
		memset(User_Police_Information,0,sizeof(User_Police_Information));
		EDIT_GetText(pThisWin->hEditBox, (char *)User_Police_Information, sizeof(User_Police_Information));
		TRACE_PRINTF("User_Police_Information1:%s\r\n",User_Police_Information);
		if(strlen(User_Police_Information)>0)
		{
			_ConfirmButton_Function(pMsg);
		}
		else
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_WarningContent);
			switch(pThisWin->sProperty.nWinType)
			{
				case eInputTestee:
					s.pContent = _PleaseInput_Tester_Name; 
					break;

				case eTestLocation:
					s.pContent = _PleaseInput_Test_Location; 
					break;

				case eInputPoliceName:
					s.pContent = _PleaseInput_Police_Name; 
					break;

				case eInputDepartment:
					s.pContent = _PleaseInput_Police_Department; 
					break;
			}
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
	CW_InputWindow *pThisWin;
	pThisWin = (CW_InputWindow *)FindWMBFromListByWin(pMsg->hWin);
	
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
							if(pMsg->hWin != GetFirstWinFromList())
							{
								WM_DeleteWindow(pMsg->hWin);
							}
						#endif
						break;
						
					case GUI_KEY_RIGHT_FUN:
						WM_SendMessage(pThisWin->hKeyBoard, pMsg);
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
			switch (pMsg->Data.v)
			{
				case WM_NOTIFICATION_GOT_FOCUS:
					_Edit_Focus_Message(pMsg);
					pThisWin->hKeyBoard = CW_InputKeyBoard_Create(0, 68, LCD_XSIZE, 220, pMsg->hWin, NULL);
					WM_BringToTop(pThisWin->hText);
					break;
					
				case WM_NOTIFICATION_LOST_FOCUS:
					WM_DeleteWindow(pThisWin->hKeyBoard);
					break;
					
				case WM_NOTIFICATION_WIN_DELETE:
					WM_SetFocus(pThisWin->hEditBox);
					break;
			}
			return;
			
		case WM_DELETE:
			Voc_Close();
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eInputWindow window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}

CW_InputWindow *CW_InputWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	CW_InputWindow *pThisWin = NULL;
	eUserWindow WinType = eInputWindow;

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_InputWindow));
	if(hWin)
	{
		TRACE_PRINTF("eInputWindow windows is created!hWin = 0x%04X\r\n",hWin);

		pThisWin = (CW_InputWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

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
		TRACE_PRINTF("eInputWindow window can't be created!\r\n");
	}

	return pThisWin;
}

#endif


