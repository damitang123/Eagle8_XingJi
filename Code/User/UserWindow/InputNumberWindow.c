#include "platform.h"
#include "Windows.h"
#include "rtc.h"
#include "time_calc.h"
#include "EDIT_Private.h"
#include "calibrate.h"

#if SERVER_USE_XING_JIKONG_GU
#include "app.h"
#endif

#define Btn_Width		54
#define Btn_Height		67
#define Btn_H_Interval 	3
#define Btn_V_Interval 	3
#define X_START			(LCD_XSIZE-((Btn_Width*4)+Btn_H_Interval*3)>>1)
#define Y_START			73

#define KEYBOARD_ID		GUI_ID_BUTTON9

static const char *Btn_NumberTab[] =
{
	"1", "2", "3", "X",
	"4", "5", "6", ".",
	"7", "8", "9", "0",
};

static unsigned char Get_Non_Numeric_Position(char *s)
{
	char i = 0;
	while(*s++)
	{
		if((*s >='0' && *s <= '9') || *s == '.')
		{
			i++;
		}
		else
		{
			return i + 1;
		}
	}

	return 0;
}

/*按下中间键后的消息.*/
static void _ConfirmButton_Function(WM_MESSAGE *pMsg)
{	
	float fTemp;
	char aTemp[12];
	void(*pFun)(void);
	CW_InputNumberWindow *pThisWin;
	pThisWin = (CW_InputNumberWindow *)FindWMBFromListByWin(pMsg->hWin);

	switch(pThisWin->sProperty.nWinType)
	{
		case eInputDriverLicence:
			memset(g_sRecord.aDriverLicense,0,sizeof(g_sRecord.aDriverLicense));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sRecord.aDriverLicense, sizeof(g_sRecord.aDriverLicense));
			if(1)
			{
				sInputWindowProperty s;
				s.nWinType = eTestLocation;
				s.aCaptionText = _TestLocationInput;
				CW_InputWindow_Create(pMsg->hWin, (void *)&s);
			}
			break;

		case eInputPoliceID:
			memset(g_sRecord.aPoliceID,0,sizeof(g_sRecord.aPoliceID));
			EDIT_GetText(pThisWin->hEditBox,(char *)g_sRecord.aPoliceID, sizeof(g_sRecord.aPoliceID));
			if(1)
			{
				sInputWindowProperty s;
				s.nWinType = eInputDepartment;
				s.aCaptionText = _DepartmentInput;
				CW_InputWindow_Create(pMsg->hWin, (void *)&s);
			}
			break;


		/*************************************************************************************************/
		/*注册使用者信息*/
		case eInputLoginID:
			memset(g_sCurrPoliceUserInfor.aPoliceID,0,sizeof(g_sCurrPoliceUserInfor.aPoliceID));
			EDIT_GetText(pThisWin->hEditBox,(char *)g_sCurrPoliceUserInfor.aPoliceID, sizeof(g_sCurrPoliceUserInfor.aPoliceID));
			if(1)
			{
				sInputWindowProperty s;
				s.nWinType = eInputLoginDepartment;
				s.aCaptionText = _PleaseInputDepartment;
				CW_InputWindow_Create(pMsg->hWin, (void *)&s);
			}
			break;
	
		case eSetLoginPassword:
			memset(g_sCurrPoliceUserInfor.aPassword,0,sizeof(g_sCurrPoliceUserInfor.aPassword));
			EDIT_GetText(pThisWin->hEditBox, (char *)g_sCurrPoliceUserInfor.aPassword, sizeof(g_sCurrPoliceUserInfor.aPassword));
			if(strlen((char *)g_sCurrPoliceUserInfor.aPassword) == 6)
			{
				sInputWindowProperty s;
				s.nWinType = eSetVerifyLoginPassword;
				s.aCaptionText = _PleaseSetVerifyLoginPassword;
				CW_InputNumberWindow_Create(pMsg->hWin,(void *)&s);
			}
			else
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);
				s.pContent = _AlertInputValidPassword;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 3;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			}
			break;

		case eSetVerifyLoginPassword:
			EDIT_GetText(pThisWin->hEditBox, (char *)aTemp, sizeof(aTemp));
			if(strcmp(aTemp, (char *)g_sCurrPoliceUserInfor.aPassword) == 0)
			{
				sWinManageList * pWM;
				pWM = FindWMBByWinType(eWinUserManage);
				Data_Police_User_Infor_Insert_Save(&g_sCurrPoliceUserInfor);
				WM_DeleteWindow(pWM->hWin);
				CW_UserManage_Create(GetLastWinFromList(),(void *)1);
			}
			else
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);
				s.pContent = _AlertInputValidVerifyPassword;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 3;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			}
			break;

		case eInputLoginPassword:
			EDIT_GetText(pThisWin->hEditBox, (char *)aTemp, sizeof(aTemp));
			if(strcmp(aTemp, pThisWin->sProperty.Data.p) == 0)
			{
				pFun = pThisWin->sProperty.pFunNext;
				WM_DeleteWindow(pMsg->hWin);
				if(pFun) (*pFun)();
			}
			else if(strlen(aTemp) == 0)
			{
				return;	
			}
			else
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);
				s.pContent = _PasswordError;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 3;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			}
			break;

		/************************************************************************************************/
			
		case eInputAdminPassword:
			EDIT_GetText(pThisWin->hEditBox, (char *)aTemp, sizeof(aTemp));
			if(strcmp(aTemp, pThisWin->sProperty.Data.p) == 0)
			{
				pFun = pThisWin->sProperty.pFunNext;
				WM_DeleteWindow(pMsg->hWin);
				if(pFun) (*pFun)();
			}
			else
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);
				s.pContent = _PasswordError;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 3;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(GetLastWinFromList(),(void *)&s);
			}
			break;

		case eInputStardardHighAlcValue:
			EDIT_GetText(pThisWin->hEditBox, (char *)aTemp, sizeof(aTemp));
			fTemp = (Get_Convert_Value_From_AnyUnit(atof(aTemp), g_nCalibrateUnit, g_sTestParam.nBrACBACK));
			if(fTemp > 60 && fTemp < 120)
			{
				pFun = pThisWin->sProperty.pFunNext;
				g_fHStandardAlcoholValue = fTemp;
				g_sAlcCalibrationParam.nCurrCalibrteState = eCaliHighPoint;
				WM_DeleteWindow(pMsg->hWin);
				if(pFun) (*pFun)();
			}
			else
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);
				s.pContent = _InvalidStandardHConcentration;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 0;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(pMsg->hWin,(void *)&s);
				return;
			}				
			break;

		case eInputStardardLowAlcValue:
			EDIT_GetText(pThisWin->hEditBox, (char *)aTemp, sizeof(aTemp));
			fTemp = (Get_Convert_Value_From_AnyUnit(atof(aTemp), g_nCalibrateUnit, g_sTestParam.nBrACBACK));
			if(fTemp > 10 && fTemp < 50)
			{
				pFun = pThisWin->sProperty.pFunNext;
				g_fLStandardAlcoholValue = fTemp;
				g_sAlcCalibrationParam.nCurrCalibrteState = eCaliLowPoint;
				WM_DeleteWindow(pMsg->hWin);
				if(pFun) (*pFun)();
			}
			else
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);;
				s.pContent = _InvalidStandardLConcentration;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 0;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(pMsg->hWin,(void *)&s);
				return;
			}
			break;

		case eInputQuickTestAlcValue:
			EDIT_GetText(pThisWin->hEditBox, (char *)aTemp, sizeof(aTemp));
			fTemp = (Get_Convert_Value_From_AnyUnit(atof(aTemp), g_nCalibrateUnit, g_sTestParam.nBrACBACK));
			if(fTemp > 10 && fTemp < 50)
			{
				pFun = pThisWin->sProperty.pFunNext;
				g_fQuickTestStandardAlcoholValue = fTemp;
				g_sAlcCalibrationParam.nCurrCalibrteState = eCaliQuickTest;
				WM_DeleteWindow(pMsg->hWin);
				if(pFun) (*pFun)();
			}
			else
			{
				sDialogWindowProperty s;
				s.nWinType = eDialogOK;
				s.pFont = _GetFont(Font_Content);;
				s.pContent = _InvalidStandardLConcentration;
				s.nContentColor = GUI_RED;
				s.nBackGroundColor = GUI_LIGHTGRAY;
				s.nAutoCloseTime = 0;
				s.pFunMiddleOK = NULL;
				CW_ShowDialog_Create(pMsg->hWin,(void *)&s);
				return;
			}
			break;
	}
}

static void _Create(WM_MESSAGE *pMsg)
{
	char aTemp[50];
	char aResult[20];
	WM_HWIN hWin;
	CW_InputNumberWindow *pThisWin;
	pThisWin = (CW_InputNumberWindow *)FindWMBFromListByWin(pMsg->hWin);
	
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


	/*输入内容*/
	pThisWin->hEditBox = EDIT_CreateEx(X_START, 34, LCD_XSIZE - X_START * 2, 33,
	                     pMsg->hWin, WM_CF_SHOW, 0, 0, 100);
	EDIT_SetFont(pThisWin->hEditBox, &GUI_FontTahomaBold23);
	EDIT_SetTextAlign(pThisWin->hEditBox, GUI_TA_VCENTER);
	EDIT_SetTextColor(pThisWin->hEditBox, 0, GUI_RED);
	
	switch(pThisWin->sProperty.nWinType)
	{
		case eInputDriverLicence:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aDriverLicense) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			Voc_Cmd(eVOC_INPUT_LISENCE);
			break;
		case eInputPoliceID:
			pThisWin->hEditMaxLen = sizeof(g_sRecord.aPoliceID) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			EDIT_SetText(pThisWin->hEditBox, (const char *)g_sDeviceUserInfo.aPoliceID);
			Voc_Cmd(eVOC_INPUT_POLICEID);
			break;
		case eInputAdminPassword:
			pThisWin->hEditMaxLen = 10 - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			break;
		case eInputStardardHighAlcValue:
			pThisWin->hEditMaxLen = 15;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			if(g_nCalibrateUnit == 0)
			{
				sprintf(aTemp, "%.1f%s", g_fHStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit));
			}
			else if(g_nCalibrateUnit == 5)
			{
				float fTemp;
				fTemp = Get_Final_Convert_Display_Result(g_fHStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
				Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
				sprintf(aTemp, "%s%s", aResult,Get_Unit_String(g_nCalibrateUnit));
			}
			EDIT_SetText(pThisWin->hEditBox, aTemp);
			EDIT_SetCursorAtChar(pThisWin->hEditBox, Get_Non_Numeric_Position(aTemp));
			WM_DisableWindow(pThisWin->hEditBox);
			break;
		case eInputStardardLowAlcValue:
			pThisWin->hEditMaxLen = 15;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			if(g_nCalibrateUnit == 0)
			{
				sprintf(aTemp, "%.1f%s", g_fLStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit));
			}
			else if(g_nCalibrateUnit == 5)
			{
				float fTemp;
				fTemp = Get_Final_Convert_Display_Result(g_fLStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
				Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
				sprintf(aTemp, "%s%s", aResult,Get_Unit_String(g_nCalibrateUnit));
			}
			EDIT_SetText(pThisWin->hEditBox, aTemp);
			EDIT_SetCursorAtChar(pThisWin->hEditBox, Get_Non_Numeric_Position(aTemp));
			WM_DisableWindow(pThisWin->hEditBox);
			break;
		case eInputQuickTestAlcValue:
			pThisWin->hEditMaxLen = 15;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			if(g_nCalibrateUnit == 0)
			{
				sprintf(aTemp, "%.1f%s", g_fQuickTestStandardAlcoholValue,Get_Unit_String(g_nCalibrateUnit));
			}
			else if(g_nCalibrateUnit == 5)
			{
				float fTemp;
				fTemp = Get_Final_Convert_Display_Result(g_fQuickTestStandardAlcoholValue,g_nCalibrateUnit, g_sTestParam.nBrACBACK);
				Get_Format_Alc_Convert_String(fTemp, g_nCalibrateUnit,aResult,eFormatToFloat);
				sprintf(aTemp, "%s%s", aResult,Get_Unit_String(g_nCalibrateUnit));
			}
			EDIT_SetText(pThisWin->hEditBox, aTemp);
			EDIT_SetCursorAtChar(pThisWin->hEditBox, Get_Non_Numeric_Position(aTemp));
			WM_DisableWindow(pThisWin->hEditBox);
			break;
		case eInputLoginID:
			pThisWin->hEditMaxLen = sizeof(g_sCurrPoliceUserInfor.aPoliceID) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;
		case eSetLoginPassword:
			pThisWin->hEditMaxLen = sizeof(g_sCurrPoliceUserInfor.aPassword) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;
		case eSetVerifyLoginPassword:
			pThisWin->hEditMaxLen = sizeof(g_sCurrPoliceUserInfor.aPassword) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;
		case eInputLoginPassword:
			pThisWin->hEditMaxLen = MAX(sizeof(g_sCurrPoliceUserInfor.aPassword), 9) - 1;
			EDIT_SetMaxLen(pThisWin->hEditBox,pThisWin->hEditMaxLen);
			break;
		default:
			pThisWin->hEditMaxLen = 20;
			EDIT_SetMaxLen(pThisWin->hEditBox, pThisWin->hEditMaxLen);
			break;
	}

	WM_SetFocus(pThisWin->hEditBox);
}

#if SERVER_USE_XING_JIKONG_GU
static char DriverID_Information[20];
static const  char _PleaseInput_DriverID[]={0xE8,0xAF,0xB7,0xE8,0xBE,0x93,0xE5,0x85,0xA5,0xE9,0xA9,0xBE,0xE9,0xA9,0xB6,0xE8,0xAF,0x81,0xE5,0x8F,0xB7,0xEF,0xBC,0x81,0x00};   /*请输入驾驶证号！*/
static const  char _PleaseInput_PoliceID[]= {0xE8,0xAF,0xB7,0xE8,0xBE,0x93,0xE5,0x85,0xA5,0xE6,0x89,0xA7,0xE5,0x8B,0xA4,0xE6,0xB0,0x91,0xE8,0xAD,0xA6,0xE8,0xAD,0xA6,0xE5,0x8F,0xB7,0xEF,0xBC,0x81,0x00};   /*请输入执勤民警警号！*/
static void Alcohol_Value_Analyze_Left(CW_InputNumberWindow *pThisWin,WM_MESSAGE *pMsg)
{
	if((g_sRecord.fAlcValue>=20)&&(g_sXingJiKongGU.bOpenDevice==false)&&(g_sXingJiKongGU.bAdministration==false))
	{
		memset(DriverID_Information,0,sizeof(DriverID_Information));
		EDIT_GetText(pThisWin->hEditBox, (char *)DriverID_Information, sizeof(DriverID_Information));
		TRACE_PRINTF("DriverID_Information:%s\r\n",DriverID_Information);
		if(strlen(DriverID_Information)>0)
		{
			if(pMsg->hWin != GetFirstWinFromList())
			{
				WM_DeleteWindow(pMsg->hWin);
			}
		}
		else //added by lxl 20180926
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_WarningContent);
			switch(pThisWin->sProperty.nWinType)
			{
				case eInputPoliceID:
					s.pContent = _PleaseInput_PoliceID; 
					break;
					
				case eInputDriverLicence:
					s.pContent = _PleaseInput_DriverID; 
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
static void Alcohol_Value_Analyze_Middle(CW_InputNumberWindow *pThisWin,WM_MESSAGE *pMsg)
{
	if((g_sRecord.fAlcValue>=20)&&(g_sXingJiKongGU.bOpenDevice==false)&&(g_sXingJiKongGU.bAdministration==false))
	{
		memset(DriverID_Information,0,sizeof(DriverID_Information));
		EDIT_GetText(pThisWin->hEditBox, (char *)DriverID_Information, sizeof(DriverID_Information));
		TRACE_PRINTF("DriverID_Information:%s\r\n",DriverID_Information);
		if(strlen(DriverID_Information)>0)
		{
			_ConfirmButton_Function(pMsg);
		}
		else //added by lxl 20180926
		{
			sDialogWindowProperty s;
			s.nWinType = eDialogOK;
			s.pFont = _GetFont(Font_WarningContent);
			switch(pThisWin->sProperty.nWinType)
			{
				case eInputPoliceID:
					s.pContent = _PleaseInput_PoliceID; 
					break;
					
				case eInputDriverLicence:
					s.pContent = _PleaseInput_DriverID; 
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
		pThisWin=NULL;
		_ConfirmButton_Function(pMsg);
	}

}

#endif
static void _Callback(WM_MESSAGE *pMsg)
{
	CW_InputNumberWindow *pThisWin;
	pThisWin = (CW_InputNumberWindow *)FindWMBFromListByWin(pMsg->hWin);
	
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
						if(pThisWin->sProperty.nWinType == eInputStardardHighAlcValue || pThisWin->sProperty.nWinType == eInputStardardLowAlcValue)
						{
							char aTemp[12];
							EDIT_GetText(pThisWin->hEditBox,(char *)aTemp, sizeof(aTemp));
							if(strcmp(Get_Unit_String(g_nCalibrateUnit), aTemp))
							{
								EDIT_AddKey(pThisWin->hEditBox, GUI_KEY_BACKSPACE);
							}
						}
						else
						{
							EDIT_AddKey(pThisWin->hEditBox, GUI_KEY_BACKSPACE);
						}
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
			if(WM_GetId(pMsg->hWinSrc) == KEYBOARD_ID)
			{
				if(pMsg->Data.v == WM_NOTIFICATION_RELEASED)
				{
					char aTemp[5];
					BUTTON_GetText(pMsg->hWinSrc, aTemp, 5);
					EDIT_AddKey(pThisWin->hEditBox, GUI_UC_GetCharCode(aTemp));
				}
			}

			if(pMsg->Data.v == WM_NOTIFICATION_GOT_FOCUS)
			{
				if(pMsg->hWinSrc == pThisWin->hEditBox)
				{
					char x, y;
					WM_HWIN hWin;
					for(y = 0; y < 3; y++)
					{
						for(x = 0; x < 4; x++)
						{
							hWin = BUTTON_CreateEx(X_START + x * (Btn_Width + Btn_H_Interval), Y_START + y * (Btn_Height + Btn_V_Interval) , Btn_Width, Btn_Height,
							                       pMsg->hWin,
							                       WM_CF_SHOW, 1, KEYBOARD_ID);
							BUTTON_SetFont(hWin, &GUI_FontTahomaBold33);
							BUTTON_SetText(hWin, Btn_NumberTab[y * 4 + x]);
							BUTTON_SetBkColor(hWin, 0, 0xB59179);
							BUTTON_SetBkColor(hWin, 1, 0x5D4E41);
							BUTTON_SetFocussable(hWin, 0);
							BUTTON_SetTextColor(hWin, 0, GUI_BLACK);
							BUTTON_SetTextColor(hWin, 1, GUI_YELLOW);
							BUTTON_SetTextColor(hWin, 2, GUI_GRAY);
							switch(pThisWin->sProperty.nWinType)
							{
								case eInputDriverLicence:
									if(x == 3 && y == 1)
									{
										WM_DisableWindow(hWin);
									}
									break;
								case eInputPoliceID:
									if(x == 3 && y == 0)
									{
										WM_DisableWindow(hWin);
									}
									break;
								case eInputLoginID:
								case eInputAdminPassword:
								case eSetLoginPassword:
								case eSetVerifyLoginPassword:
								case eInputLoginPassword:
									if(x == 3 && (y == 0 || y == 1))
									{
										WM_DisableWindow(hWin);
									}
									break;
								case eInputStardardHighAlcValue:
								case eInputStardardLowAlcValue:
									if(x == 3 && (y == 0))
									{
										WM_DisableWindow(hWin);
									}
									break;
							}
						}
					}
				}
			}
			else if(pMsg->Data.v == WM_NOTIFICATION_LOST_FOCUS)
			{
				if(pMsg->hWinSrc == pThisWin->hEditBox)
				{
					WM_HWIN hWin;
					while(1)
					{
						hWin = WM_GetDialogItem(pMsg->hWin, KEYBOARD_ID);
						if(hWin)
							WM_DeleteWindow(hWin);
						else
							break;
					}
				}
			}
			else if(pMsg->Data.v ==  WM_NOTIFICATION_WIN_DELETE)
			{
				WM_SetFocus(pThisWin->hEditBox);
			}
			return;
			
		case WM_DELETE:
			Voc_Close();
			DeleteWindowFromList(pMsg->hWin);
			WM_NotifyParent(pMsg->hWin, WM_NOTIFICATION_WIN_DELETE);
			TRACE_PRINTF("eInputNumberWindow window is deleted!\r\n");
			break;
			
		default:
			break;
	}
	Exit_Callback_Hook(pMsg);
}

/*本窗口是可重入窗体*/
CW_InputNumberWindow *CW_InputNumberWindow_Create(WM_HWIN hParent, void *pData)
{
	WM_HWIN hWin;
	CW_InputNumberWindow *pThisWin = NULL;
	eUserWindow WinType = eInputNumberWindow;

	hWin = WM_CreateWindowAsChild(0, 0, LCD_X_SIZE, LCD_Y_SIZE,
	                              hParent, WM_CF_SHOW, NULL, sizeof(CW_InputNumberWindow));

	if(hWin)
	{
		TRACE_PRINTF("eInputNumberWindow windows is created!hWin = 0x%04X\r\n",hWin);

		pThisWin = (CW_InputNumberWindow *)((unsigned long)WM_H2P(hWin) + sizeof(WM_Obj));

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
		TRACE_PRINTF("eInputNumberWindow window can't be created!\r\n");
	}

	return pThisWin;
}

