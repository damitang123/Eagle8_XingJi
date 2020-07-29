#ifndef _USERHEAD_H_
#define _USERHEAD_H_
#include "platform.h"
#include "public_function.h"
#include "GenericTypeDefs.h"
#include "WM.h"
#include "GUI.h"
#include "DIALOG.h"
#include "LCDConf.h"
#include "uart.h"
#include "spi_flash.h"
#include "language.h"
#include "res.h"
#include "data.h"
#include "voice.h"

/*是否使用内置的大字号中文字*/
#define USE_BUILT_IN_SONGTI_BOLD33			1


#define 	Caption_Height							30
#define 	Bottom_Height							30

#define 	BK_COLOR_CLIENT						0xC0C0C0

#define WM_Key_Code		( ((const WM_KEY_INFO*)(pMsg->Data.p))->Key)
#define WM_Key_Status		(((const WM_KEY_INFO*)(pMsg->Data.p))->PressedCnt)

#define BtnSetBmp(bmp,hWin) 		&bmp,(WM_GetWindowSizeX(hWin)-bmp.XSize)>>1,(WM_GetWindowSizeY(hWin)-bmp.YSize)>>1
#define RGBConvert(red, green, blue)    (unsigned short) (((((unsigned short)(red) & 0xF8) >> 3) << 11) | ((((unsigned short)(green) & 0xFC) >> 2) << 5) | (((unsigned short)(blue) & 0xF8) >> 3))
#define _RGB(r,g,b)					((b<<16)+(g<<8)+r)
#define RGB2Hex(r,g,b)	(unsigned long)(((unsigned long)(b & 0xff) << 16) | \
									((unsigned long)(g & 0xff) << 8) | ((unsigned long)(r & 0xff)))


extern WM_HWIN g_hStartWin;

extern unsigned char g_bChagneLanguage;

extern unsigned char bYesNo;
#define STR_PASSWORD_MAX_LENGTH	16
extern char g_strPassWord[STR_PASSWORD_MAX_LENGTH];
extern unsigned char g_bManualTestWhenLocked;

extern GUI_FONT GUI_Font16_Unicode;
extern const GUI_FONT GUI_FontSongTiBold19;
extern const GUI_FONT GUI_FontSongTiBold24;
extern const GUI_FONT GUI_Font16_UTF8;
extern const GUI_FONT GUI_Font16_UTF8X2;
extern const GUI_FONT GUI_Font16_UTF8X3;
extern const unsigned char aUnicode_Width;

extern const GUI_FONT GUI_FontTahoma58;
extern const GUI_FONT GUI_FontTimesNewRoman73;
extern const GUI_FONT GUI_FontTabs;


typedef enum
{
	eUserMain = 1,
	eSettingMain,
	eAutoShutdownTimeConfig,

	eToolMain,
	
	eAdministor,
	eInputPassword,
	eToolChild,
	eCalibrationWin,
	eTestMain,
	eShowReport,
	eShowDialog,
	eSettingBacklight,
	eSettingBacklightTimeout,
	eSettingSystemInfo,
	eDebug,
	eSetTime,
	eSetCalibrationInterval,
	eSetServiceInterval,
	eSetALCUnit,
	eSetTestResultFormat,
	eSetAlcoholCriticalPoint,
	eSetBlow,
	eSetRollTestTimeout,
	eSetRollTestTimes,
	eSetRollTestFailure,
	eSetStartAndRestart,
	eSetEmergencyTelephone,
	eSetRelieveBlockState,
	eSetAntiCircumvention,
	eSetGPRS,
	eSettingLanguage,
	eTouchCalibrate,
	eAutoPowerOff,
	eInputWindow,
	eInputPlateNumWindow,
	eInputNumberWindow,
	eDisplayRecord,
	ePrintRecord,
	ePrinterSelectWin,
	ePrintCopys,
	eTimeZone,
	ePhotoShow,
	eFingerprintEnrollIdentify,
	eWinUserManage,
	eUserEnroll,
	eFingerprintIdentify,
	eSelectEnrollUser,
	eSampleMode,
	#if SERVER_USE_XING_JIKONG_GU
	eHeartRateMode,
	#endif
} eUserWindow;

typedef enum
{
	ePlateNumber,
	eTestLocation,
	eInputTestee,
	eInputDepartment,
	eInputDriverLicence,
	eInputPoliceID,
	eInputPoliceName,
	/*以下3项用于输入密码,标定标准溶液值*/
	eInputAdminPassword,
	eInputStardardHighAlcValue,
	eInputStardardLowAlcValue,
	eInputQuickTestAlcValue,
	/*以下项目服务于新国标GB2017*/
	eInputLoginName,
	eInputLoginID,
	eInputLoginDepartment,
	eSetLoginPassword,
	eSetVerifyLoginPassword,
	eInputLoginPassword,
}eInputProperty;

typedef struct
{
	union
	{
		void* p;
		int nValue;
	} Data;
	void (*pFunNext)(void);
	eInputProperty nWinType;
	const char * aCaptionText;
}sInputWindowProperty;

typedef enum
{
	eDialogOK,
	eDialogYesNo,
	eDialogCommon,
}eDialogProperty;

typedef struct
{
	eDialogProperty nWinType;
	const char *pContent;
	const GUI_FONT * pFont;
	unsigned long nContentColor;
	unsigned long nBackGroundColor;
	unsigned char nAutoCloseTime;		/*单位是秒*/
	void (*pFunLeftYes)(void);
	void (*pFunRightNo)(void);
	void (*pFunMiddleOK)(void);
	const char *pLeftButton;
	const char *pRightButton;
	const char *pMiddleButton;
}sDialogWindowProperty;

typedef enum
{
	Font_Title = 0,
	Font_Left_Button,
	Font_Right_Button,
	Font_Middle_Button,
	Font_Content,
	Font_ListboxNormal,
	Font_ListboxForSystemInfor,
	Font_ShowTipContent,
	Font_AlcTestTips,
	Font_ToolWin_Content,
	Font_ToolWin_Units,
	Font_ServiceDueTime,
	Font_SetAlcUnit,
	Font_ResultDisplayFormCombox,
	Font_RemoteMessage,
	Font_DisplayRecord,
	Font_Printer,
	Font_UnitsBlow,
	Font_WarningContent, //added by lxl  20180927
} eWindowFontSelection;

typedef struct _ActiveWinList
{
	WM_HWIN			hWin;
	unsigned long 			Property;
	eUserWindow 			eWinType;
	struct _ActiveWinList 	*pPrev;
	struct _ActiveWinList 	*pNext;
} sWinManageList;

typedef struct _sWinTimerList
{
	unsigned char MsgType;	/*与用户消息(sUserMessage结构体)第一个字节相同*/
	unsigned long nPeriod;
	WM_HWIN	hWin;

	struct _sWinTimerList 	*pPrev;
	struct _sWinTimerList 	*pNext;
} sWinTimerList;

extern const GUI_FONT GUI_FontTahoma16;
extern const GUI_FONT GUI_FontTahomaBold18;
extern const GUI_FONT GUI_FontTahoma23;
extern const GUI_FONT GUI_FontTahomaBold23;
extern const GUI_FONT GUI_FontTahomaBold19;
extern const GUI_FONT GUI_FontTimesNewRoman19;
extern const GUI_FONT GUI_FontTimesNewRoman21;
extern const GUI_FONT GUI_FontTahomaBold33;
extern const GUI_FONT GUI_FontSongTiBold33;
extern const GUI_FONT GUI_FontSongTi24;
extern  const GUI_FONT GUI_FontSongTi18_Warning; //added by lxl 20180927


extern const GUI_FONT GUI_FontTriangle;
extern const GUI_FONT GUI_FontKaiTi33BoldTiny;
extern const GUI_FONT GUI_FontAraialNarrow22Tiny;
extern const GUI_FONT GUI_FontAraialNarrow18Tiny;
extern const GUI_FONT GUI_FontAraialNarrow16Tiny;
extern const GUI_FONT GUI_FontAraialNarrow20Tiny;
extern const GUI_FONT GUI_FontWeiruanYaHei18Tiny;


/******************************************************/
void Config_Button_Property(WM_HWIN hWin);
WM_HWIN Create_Left_Button(WM_HWIN hParent);
WM_HWIN Create_Right_Button(WM_HWIN hParent);
WM_HWIN Create_Middle_Button(WM_HWIN hParent);

void Paint_BackGround(void);
void Paint_Dialog_Header_Bottom(void);
void Enter_Callback_Hook(WM_MESSAGE *pMsg);
void Exit_Callback_Hook(WM_MESSAGE *pMsg);
WM_HWIN Create_Dialog_Caption(WM_MESSAGE *pMsg);
void Set_Dialog_Caption_Content(const char *s,WM_HWIN hCaption);
char * Get_Language_String(unsigned short nStringIndex);

const GUI_FONT *_GetFont(eWindowFontSelection nWindowIndex);


/*窗口操作类函数*/
void AddWindowToList(sWinManageList *pWin);
void DeleteWindowFromList(WM_HWIN hWin);


sWinManageList *FindWMBByWinType(eUserWindow WinType);
sWinManageList *FindWMBFromListByWin(WM_HWIN  hWin);
WM_HWIN GetWinHandleByWinType(eUserWindow WinType);

unsigned short GetLastWinIDFromList(void);
WM_HWIN GetFirstWinFromList(void);
WM_HWIN GetLastWinFromList(void);
sWinManageList *GetLastWMBFromList(void);
eUserWindow GetLastWinTypeFromList(void);

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hHeader;
	WM_HWIN hBottom;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hDate;
	WM_HWIN hTime;
	
	WM_HWIN hDrawBattery;
	WM_HWIN hDrawGPRSSignal;
	WM_HWIN hDrawGpsSignal;
} CW_UserMain;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hHeader;
	WM_HWIN hBottom;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hRecord;
	WM_HWIN hImage;
	unsigned long nCurrDisplayRecordIndex;
	unsigned char nCurrDisplayMode;
	bool bValidPhoto;
	bool bValidFingerprint;
} CW_DisplayRecordWindow;

typedef struct
{
	sWinManageList w_obj;
} CW_ToolMain;

typedef struct
{
	sWinManageList wObj;
} CW_SetTime;

typedef struct
{
	sWinManageList wObj;
	unsigned char i;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hCaption;
	WM_HWIN hListbox;
	char nLisboxReleasedCnt;
} CW_SettingMain,CW_AutoShutdownTimeConfig,CW_BacklightTimeout,CW_SystemInfo,
	CW_Administrator,CW_TimeZoneWindow,CW_PrintCopysWindow,
	CW_HeartRateWindow,CW_SampleModeConfig;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	
	WM_HWIN hAddUserButton;
	WM_HWIN hDeleteUserButton;
	
	WM_HWIN hCaption;
	WM_HWIN hListbox;
	char nLisboxReleasedCnt;

	char nListIndexLUT[33];
} CW_UserManage,CW_SelectEnrollUser;

typedef struct
{
	sWinManageList w_obj;
} CW_SettingLanguage;

typedef struct
{
	sWinManageList w_obj;
} CW_ToolChild;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hCaption;
	
	WM_HWIN hButtonHighConcentration;
	WM_HWIN hTextHighConcentration;
	WM_HWIN hButtonDefaultHighConcentration;
	
	WM_HWIN hButtonLowConcentration;
	WM_HWIN hTextLowConcentration;
	WM_HWIN hButtonDefaultLowConcentration;

	WM_HWIN hButtonQuickTest;
	WM_HWIN hTextQuickTest;
	WM_HWIN hButtonDefaultQuickTest;
	
	WM_HWIN hEnvTemperature;
	GUI_TIMER_HANDLE hTMTemperature;
} CW_Calibration;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hTimeProgressBar;
	WM_HWIN hBlowFlowVolume;
	WM_HWIN hCaption;
	WM_HWIN hTestTips;
	WM_HWIN hProcessProgress;

	WM_HWIN hQuickTestDraw;
	WM_HWIN hQuickTestDisplayValue;
	WM_HWIN hQuickTestDisplayTime;
	
	bool	bNeedRefreshState;
	unsigned short nPressValueBeforeBlow;
	signed char nEnvTemperatureBeforeBlow;
	float  fAlcSampleTemperature;
	unsigned long nStartBlowTime;
	unsigned long nBlowDurationTime;
	unsigned long nRequestBlowDurationTime;
	unsigned long nStartDelayTime;
	unsigned long nCameraStartCaptureTime;
	unsigned long nStartWaitBlowTime;

	bool bAlcSensorReady;
	bool bQuickTestAlcSernsorReady;

	/*下面几个参数供新国标GB2017使用.*/
	double fSumOfBlowFlowVolume;
	float fRequestBlowFlowVolume;
	unsigned short nLastBlowAdcValue;
	unsigned long nLastBlowAdcIntegralTime;

	/*清零计数.*/
	unsigned char nCleanCnt;
	unsigned char nCurrPumpState;
	/*判断是否达到采样条件.*/
	bool bAchieveSampleCondition;
	
	WM_HWIN hTestUnit;
	WM_HWIN hTestResult;
	WM_HWIN hTestBacBrac;
	GUI_TIMER_HANDLE hTMTest;
} CW_TestMain;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hCaption;
} CW_ShowReport;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hEditBox;
	unsigned short hEditMaxLen;
	sInputWindowProperty sProperty;
} CW_InputNumberWindow;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hEditBox;
	WM_HWIN hText;
	WM_HWIN hKeyBoard;
	unsigned short hEditMaxLen;
	unsigned char nInputType;
	sInputWindowProperty sProperty;
} CW_InputWindow,CW_InputPlateNumberWindow;


typedef struct
{
	sWinManageList wObj;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hCaption;
	WM_HWIN hSliderValue;
	WM_HWIN hButtonBackLightTimeout;
	unsigned char nSliderValue;
} CW_BackLight;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hContent;
	GUI_TIMER_HANDLE hTimer;
	WM_HWIN hPrev;
	WM_HWIN hNext;
	sDialogWindowProperty sProperty;
} CW_ShowDialog;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hPhoto;
	WM_HWIN hMiddleButton;
	unsigned long nJpegSize;
} CW_ShowPhoto;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hContent;
	WM_HWIN hCaption;
	unsigned char nFPIdentifyFailCnt;
	signed char nFPLastWorkState;
	signed char nFPEnrollLastStep;
	GUI_TIMER_HANDLE hTMFp;
} CW_FingerPrintEnroll,CW_FingerPrintIdentify;


typedef struct
{
	sWinManageList wObj;
	WM_HWIN hLeftButton;
	WM_HWIN hRightButton;
	WM_HWIN hMiddleButton;
	WM_HWIN hContent;
	WM_HWIN hCaption;
	unsigned char nIdentifyCnt;
	signed char nFPLastWorkState;
	signed char nFPEnrollLastStep;
	GUI_TIMER_HANDLE hTMFp;
} CW_FingerPrintEnrollIdentify;

typedef struct
{
	sWinManageList wObj;
	bool bNeedRefeshWindow;
	unsigned short x0,y0;
	unsigned short x1,y1;
	unsigned short x2,y2;
	unsigned short x3,y3;
	unsigned char nCaliIndex;
	WM_HWIN hTextTimer;
	unsigned long nStartTime;
	GUI_TIMER_HANDLE hTimer;
} CW_TouchCalibrate;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hBlowTimeInc;
	WM_HWIN hBlowTimeDec;
	WM_HWIN hBlowPressInc;
	WM_HWIN hBlowPressDec;
	WM_HWIN hBlowTime;
	WM_HWIN hBlowPress;
	unsigned long nBlowTime;
	unsigned char nBlowPress; 
} CW_SetBlow;

typedef struct
{
	sWinManageList wObj;
	
	WM_HWIN hCurUnitTypeInc;
	WM_HWIN hCurUnitTypeDec;
	WM_HWIN hTestUnitInc;
	WM_HWIN hTestUnitDec;
	WM_HWIN hBrACBACKInc;
	WM_HWIN hBrACBACKDec;
	
	WM_HWIN hCurUnitType;
	WM_HWIN hTestUnit;
	WM_HWIN hBrACBACK;
	
	unsigned char nCurUnitType;
	unsigned char nTestUnit;
	unsigned short nBrACBACK; 
} CW_SetALCUnits;

typedef struct
{
	sWinManageList wObj;
	bool bInhalTest;
	unsigned char nInhalTestCnt;
	GUI_TIMER_HANDLE hTMDebug;
} CW_Debug;

typedef struct
{
	sWinManageList wObj;
	WM_HWIN hMiddleButton;
	WM_HWIN hContent;
	bool bNeedRefreshState;
	unsigned char ePrinterState;
	unsigned char nPrintCopys;
	unsigned char nRetryCnt;
	unsigned long nStartTime;
	GUI_TIMER_HANDLE hTMPrinter;
} CW_PrintRecordWindow;



typedef struct
{
	sWinManageList w_obj;
	char  strPassword[STR_PASSWORD_MAX_LENGTH];
} CW_InputPassword;


typedef struct
{
	sWinManageList w_obj;
} CW_SetCalibrationInterval;

typedef struct
{
	sWinManageList w_obj;
} CW_SetServiceInterval;



typedef struct
{
	sWinManageList w_obj;
} CW_SetTestResultFormat;

typedef struct
{
	sWinManageList w_obj;
} CW_SetAlcoholCriticalPoint;




typedef struct
{
	sWinManageList w_obj;
} CW_SetRollTestTimeout;

typedef struct
{
	sWinManageList w_obj;
} CW_SetRollTestTimes;

typedef struct
{
	sWinManageList w_obj;
} CW_SetRollTestFailure;

typedef struct
{
	sWinManageList w_obj;
} CW_SetStartAndRestart;

typedef struct
{
	sWinManageList w_obj;
} CW_SetEmergencyTelephone;

typedef struct
{
	sWinManageList w_obj;
} CW_SetRelieveBlockState;

typedef struct
{
	sWinManageList w_obj;
} CW_SetAntiCircumvention;

typedef struct
{
	sWinManageList w_obj;
} CW_SetGPRS;



typedef struct
{
	sWinManageList w_obj;
} CW_AutoPowerOff;







typedef struct
{
	sWinManageList w_obj;
} CW_PrinterSelectWindow;

typedef struct
{
	sWinManageList w_obj;
} CW_PhotoShowWindow;

typedef struct
{
	sWinManageList w_obj;
} CW_FingerPrintVerifyWindow;




CW_ToolMain *CW_ToolMain_Create(unsigned short ID);
CW_InputPassword *CW_InputPassword_Create(unsigned short ID);
CW_ToolChild *CW_ToolChild_Create(unsigned short ID);
CW_ShowDialog *CW_ShowTip_Create(unsigned short ID, const char *pTextCaption, const char *pTextLeft, const char *pTextRight, const GUI_FONT *pFont, unsigned long nFrontColor, const char *textContent, unsigned short nAutoCloseTimer);
CW_SetCalibrationInterval *CW_SetCalibrationInterval_Create(unsigned short ID);
CW_SetServiceInterval *CW_SetServiceInterval_Create(unsigned short ID);
CW_SetTestResultFormat *CW_SetTestResultFormat_Create(unsigned short ID);
CW_SetAlcoholCriticalPoint *CW_SetAlcoholCriticalPoint_Create(unsigned short ID);

CW_SetRollTestTimeout *CW_SetRollTestTimeout_Create(unsigned short ID);
CW_SetRollTestTimes *CW_SetRollTestTimes_Create(unsigned short ID);
CW_SetRollTestFailure *CW_SetRollTestFailure_Create(unsigned short ID);
CW_SetStartAndRestart *CW_SetStartAndRestart_Create(unsigned short ID);
CW_SetEmergencyTelephone *CW_SetEmergencyTelephone_Create(unsigned short ID);
CW_SetRelieveBlockState *CW_SetRelieveBlockState_Create(unsigned short ID);
CW_SetAntiCircumvention *CW_SetAntiCircumvention_Create(unsigned short ID);
CW_SetGPRS *CW_SetGPRS_Create(unsigned short ID);
CW_SettingLanguage *CW_SettingLanguage_Create(unsigned short ID);
CW_AutoPowerOff *CW_AutoPowerOff_Create(unsigned short ID);

CW_PrinterSelectWindow *CW_PrinterSelectWindow_Create(unsigned short ID);


CW_UserMain *CW_UserMain_Create(WM_HWIN hParent, void *pData);
CW_DisplayRecordWindow *CW_DisplayRecordWindow_Create(WM_HWIN hParent, void *pData);
CW_SettingMain *CW_SettingMain_Create(WM_HWIN hParent, void *pData);
CW_AutoShutdownTimeConfig * CW_AutoShutdownTimeConfig_Create(WM_HWIN hParent, void *pData);
CW_TestMain *CW_TestMain_Create(WM_HWIN hParent, void *pData);
CW_ShowReport *CW_ShowReport_Create(WM_HWIN hParent, void *pData);
CW_InputWindow *CW_InputWindow_Create(WM_HWIN hParent, void *pData);
CW_InputNumberWindow *CW_InputNumberWindow_Create(WM_HWIN hParent, void *pData);
CW_InputPlateNumberWindow *CW_InputPlateNumWindow_Create(WM_HWIN hParent, void *pData);
CW_BackLight *CW_Setting_BackLight_Create(WM_HWIN hParent, void *pData);
CW_BacklightTimeout *CW_Setting_BackLightTimeout_Create(WM_HWIN hParent, void *pData);
CW_ShowDialog *CW_ShowDialog_Create(WM_HWIN hParent, void *pData);
CW_ShowPhoto *CW_ShowPhoto_Create(WM_HWIN hParent, void *pData);
CW_UserManage  *CW_UserManage_Create(WM_HWIN hParent, void *pData);
CW_FingerPrintEnroll *CW_FingerPrintEnroll_Create(WM_HWIN hParent, void *pData);
CW_FingerPrintIdentify *CW_FingerPrintIdentify_Create(WM_HWIN hParent, void *pData);
CW_SystemInfo *CW_Setting_SystemInfo_Create(WM_HWIN hParent, void *pData);
CW_SetTime *CW_SetTime_Create(WM_HWIN hParent, void *pData);
CW_TouchCalibrate *CW_TouchCalibrate_Create(WM_HWIN hParent, void *pData);
CW_Administrator *CW_Administrator_Create(WM_HWIN hParent, void *pData);
CW_TimeZoneWindow *CW_TimeZoneWindow_Create(WM_HWIN hParent, void *pData);
CW_SetBlow *CW_SetBlow_Create(WM_HWIN hParent, void *pData);
CW_SetALCUnits *CW_SetALCUnit_Create(WM_HWIN hParent, void *pData);
CW_Debug *CW_Debug_Window_Create(WM_HWIN hParent, void *pData);
CW_Calibration *CW_Calibration_Create(WM_HWIN hParent, void *pData);
CW_PrintCopysWindow *CW_PrintCopysWindow_Create(WM_HWIN hParent, void *pData);
CW_PrintRecordWindow *CW_PrintRecordWindow_Create(WM_HWIN hParent, void *pData);
CW_SelectEnrollUser * CW_SelectEnrollUser_Create(WM_HWIN hParent, void *pData);
CW_FingerPrintEnrollIdentify *CW_FingerPrintEnrollIdentify_Create(WM_HWIN hParent, void *pData);
CW_SampleModeConfig * CW_Sample_Mode_Config_Create(WM_HWIN hParent, void *pData);
#if SERVER_USE_XING_JIKONG_GU
CW_HeartRateWindow *CW_HeartRateWindow_Create(WM_HWIN hParent, void *pData);
#endif
WM_HWIN CW_InputKeyBoard_Create(int x0, int y0, int width, int height, WM_HWIN hParent, void *pData);


#define WM_UPDATE_TIME        	WM_USER+1 	/*更新时间*/
#define WM_UPDATE_PROGBAR	WM_USER+2	/*更新进度条*/
#define WM_UPDATE_TEMPERATURE WM_USER+3	/*更新温度*/
#define WM_ALCOHOL_TEST		WM_USER+4	/*检测测试状态是否有变化*/
#define WM_WINDOW_TIMER		WM_USER+5	/*检查窗口是否超时*/
#define WM_CHANGE_LANGUAGE	WM_USER+8	/*判断语言是否改变.*/
#define WM_CHECK_PRINTER_STATE WM_USER+9	/*检测打印机状态是否有变化*/
#define WM_REFRESH_DIS_RECORD WM_USER+10	/*更新显示记录*/
#define WM_LOWPOWER_TIP		WM_USER+11
#define WM_INVALID_SIMCARD_TIP	WM_USER+12
#define WM_CHECK_FPVERIFY_STATE WM_USER+13	/*检测指纹验证是否有变化*/
#define WM_FP_ENROLL_SCAN		 WM_USER+14	/*指纹登记扫描.*/
#define WM_FP_ENROLL_SUCCEED   WM_USER+15	/*指纹登记成功.*/
#define WM_NOTIFY_GPRS_UPLOAD_EVENT		(WM_USER + 16)

#define WM_NOTIFICATION_WIN_DELETE		(WM_NOTIFICATION_USER + 1)
#define WM_NOTIFICATION_CONFIRM_STRING	(WM_NOTIFICATION_USER + 2)
#define WM_NOTIFICATION_VERIFY_PASSWORD	(WM_NOTIFICATION_USER + 3)


#define LEFT_BUTTON_WIDTH		70
#define RIGHT_BUTTON_WIDTH  	70
#define MIDDLE_BUTTON_WIDTH	80

#define BUTTON_ID_LEFT			GUI_ID_BUTTON0
#define BUTTON_ID_RIGHT		GUI_ID_BUTTON1
#define BUTTON_ID_MIDDLE		GUI_ID_BUTTON2

#define BUTTON_TEXT_COLOR_RELEASED  	GUI_WHITE
#define BUTTON_TEXT_COLOR_PRESSED   	GUI_YELLOW


/*列表框处理结构体*/
typedef  void (* pFunUser)(WM_HWIN hParent, void *pData);

typedef struct
{
	unsigned short 	nStringIndex;
	pFunUser 		pFun;
}sListProcessTable;

#endif

