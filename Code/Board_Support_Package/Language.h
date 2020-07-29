#ifndef _LANGUAGE_H_
#define _LANGUAGE_H_

typedef enum
{
    eLanguage_English = 0,
    eLanguage_Chinese,
    eLanguage_Russian,
    eLanguage_Vietnamese,
    eLanguage_Spanish,
}eSystemLanguage;

extern  const  char** Strings[];
extern eSystemLanguage g_Language;

enum
{
	eAlcoholTestUnit = 0x0000 ,
	eMonday ,
	eTuesday ,
	eWednesday ,
	eThursday ,
	eFriday ,
	eSaturday ,
	eSunday ,
	eBack ,
	eDelete ,
	eNext ,
	ePrevious ,
	eQuery ,
	eTest ,
	eOption ,
	eSetting ,
	eBackLight ,
	eBackLightBrightness ,
	eBackLightTimeout ,
	eNeverSleep ,
	eS15Seconds ,
	eS30Seconds ,
	eS1Minutes ,
	eS3Minutes ,
	eS5Minutes ,
	eAutoOff ,
	eAutoOffTime ,
	eNeverAutoOff ,
	e1Minute ,
	e3Minutes ,
	e5Minutes ,
	e10Minutes ,
	e15Minutes ,
	e30Minutes ,
	e1Hour ,
	eLanguage ,
	eTimeDateSet ,
	eInvalidTimeDateSet ,
	eTouchScreenCalibrate ,
	ePrinterSelect ,
	ePleaseSelectPrinter ,
	ePrinterCopys ,
	ePleaseSelectPrinterCopys ,
	eRestoreSet ,
	eSystemInformation ,
	eHardware ,
	eSoftware ,
	eDeviceNo ,
	eCalibrateDate ,
	eAdmin ,
	eOK ,
	eExit ,
	eCancel ,
	eRetry ,
	eYes ,
	eNo ,
	eDefault ,
	eEnsureNeedTouchCalibrate ,
	eTouchCalibrateTip ,
	eT_Prepare ,
	eT_Clean ,
	eT_WaitBlow ,
	eT_Blowing ,
	eT_BlowBreak ,
	eT_Precessing ,
	ePassive ,
	eRefuse ,
	eRetest ,
	eAlcoholMaintainTest ,
	eQuickTest ,
	eCalibrationWithValue ,
	eAlcoholTest ,
	eRetest_Refuse ,
	eCalibration ,
	eAlcUnit ,
	eAlcUnitCategory ,
	eAlcUnitType ,
	eAlcConvFactor ,
	eBAC ,
	eBrAC ,
	eBlowParameter ,
	eBlowTime ,
	eSeconds ,
	eLevel ,
	eBlowStrengthLevel ,
	eSampleModeSelect ,
	eSampleModeSelectByTime ,
	eSampleModeSelectByEndBlow ,
	eTimeZoneSetting ,
	eAPN ,
	eServer ,
	ePleaseInputZFYID ,
	eDeviceDebug ,
	eInhal ,
	eCalibrate ,
	eClear ,
	eSave ,
	eStartTest ,
	eHighConcentrationCali ,
	eLowConcentrationCali ,
	eQuickTestCali ,
	ePleaseInputHighStandard ,
	ePleaseInputLowStandard ,
	ePleaseInputQuickTestStandard ,
	eInvalidStandardHConcentration ,
	eInvalidStandardLConcentration ,
	eTempCalibration ,
	eDeleteCaliDataTip ,
	eInvalidCaliData ,
	eTestReport ,
	eRestoreSetTip ,
	ePlateNumberInput ,
	ePleaseInputDriveLicence ,
	ePleaseInputPoliceID ,
	eTesteeNameInput ,
	eTestLocationInput ,
	ePoliceNameInput ,
	eDepartmentInput ,
	ePleaseInputPassword ,
	ePasswordError ,
	eNoRecord ,
	eR_RecordNumber ,
	eR_Date ,
	eR_Time ,
	eR_Temperature ,
	eR_TestMode ,
	eR_ActiveTest ,
	eR_PassiveTest ,
	eR_QuickTest ,
	eR_RefuseTest ,
	eR_TestResult ,
	eR_TestRestltNormal ,
	eR_TestRestltDrink ,
	eR_TestRestltDrunk ,
	eR_Testee ,
	eR_Location ,
	eR_Longitude ,
	eR_Longitude_East ,
	eR_Longitude_West ,
	eR_Latitude ,
	eR_Latitude_South ,
	eR_Latitude_North ,
	eR_PlateNumber ,
	eR_DriverLicense ,
	eR_PoliceID ,
	eR_PoliceName ,
	eR_Department ,
	ePrint ,
	ePrinterPair ,
	eNoExcutePrint ,
	eConnectPrinter ,
	eShutdownPrinter ,
	eConnectPrinterFail ,
	_PrinterNoPaper ,
	ePrinting ,
	ePrintFinish ,
	eBlueToothParing ,
	ePairFail ,
	ePairSuccessful ,
	eP_DeviceID ,
	eP_RecordNumber ,
	eP_Date ,
	eP_Time ,
	eP_Zero ,
	eP_TestMode ,
	eP_ActiveTest ,
	eP_PassiveTest ,
	eP_QucikTest ,
	eP_RefuseTest ,
	eP_BAC ,
	eP_BrAC ,
	eP_TestResult ,
	eP_TestRestltNormal ,
	eP_TestRestltDrink ,
	eP_TestRestltDrunk ,
	eP_Testee ,
	eP_Location ,
	eP_LongitudeLatitude ,
	eP_Longitude_East ,
	eP_Longitude_West ,
	eP_Latitude_South ,
	eP_Latitude_North ,
	eP_PlateNumber ,
	eDriverSignature ,
	eP_DriverLicense ,
	eP_SubjectSignature ,
	eP_PoliceName ,
	eP_PoliceID ,
	eP_Department ,
	ePoliceOfficerSignature ,
	eLowPowerTip ,
	eInvalidSimCardTip ,
	eDeviceIMEINotRegister ,
	eUserNmae ,
	eUserID ,
	eUserDepartment ,
	eUserBasicInfor ,
	eUserManage ,
	eDeleteAllUserInfor ,
	eConfirmDeleteAllUserInfor ,
	eAlertRegisterUserInfor ,
	ePleaseInputName ,
	ePleaseInputID ,
	ePleaseInputDepartment ,
	ePleaseSetLoginPassword ,
	ePleaseSetVerifyLoginPassword ,
	ePleaseInputLoginPassword ,
	eMaintainMode ,
	eAlertInputValidName ,
	eAlertInputValidPassword ,
	eAlertInputValidVerifyPassword ,
	eHeartRateSelect ,
};



#define _AlcoholTestUnit	Strings[g_Language][eAlcoholTestUnit]
#define _Monday	Strings[g_Language][eMonday]
#define _Tuesday	Strings[g_Language][eTuesday]
#define _Wednesday	Strings[g_Language][eWednesday]
#define _Thursday	Strings[g_Language][eThursday]
#define _Friday	Strings[g_Language][eFriday]
#define _Saturday	Strings[g_Language][eSaturday]
#define _Sunday	Strings[g_Language][eSunday]
#define _Back	Strings[g_Language][eBack]
#define _Delete	Strings[g_Language][eDelete]
#define _Next	Strings[g_Language][eNext]
#define _Previous	Strings[g_Language][ePrevious]
#define _Query	Strings[g_Language][eQuery]
#define _Test	Strings[g_Language][eTest]
#define _Option	Strings[g_Language][eOption]
#define _Setting	Strings[g_Language][eSetting]
#define _BackLight	Strings[g_Language][eBackLight]
#define _BackLightBrightness	Strings[g_Language][eBackLightBrightness]
#define _BackLightTimeout	Strings[g_Language][eBackLightTimeout]
#define _NeverSleep	Strings[g_Language][eNeverSleep]
#define _S15Seconds	Strings[g_Language][eS15Seconds]
#define _S30Seconds	Strings[g_Language][eS30Seconds]
#define _S1Minutes	Strings[g_Language][eS1Minutes]
#define _S3Minutes	Strings[g_Language][eS3Minutes]
#define _S5Minutes	Strings[g_Language][eS5Minutes]
#define _AutoOff	Strings[g_Language][eAutoOff]
#define _AutoOffTime	Strings[g_Language][eAutoOffTime]
#define _NeverAutoOff	Strings[g_Language][eNeverAutoOff]
#define _1Minute	Strings[g_Language][e1Minute]
#define _3Minutes	Strings[g_Language][e3Minutes]
#define _5Minutes	Strings[g_Language][e5Minutes]
#define _10Minutes	Strings[g_Language][e10Minutes]
#define _15Minutes	Strings[g_Language][e15Minutes]
#define _30Minutes	Strings[g_Language][e30Minutes]
#define _1Hour	Strings[g_Language][e1Hour]
#define _Language	Strings[g_Language][eLanguage]
#define _TimeDateSet	Strings[g_Language][eTimeDateSet]
#define _InvalidTimeDateSet	Strings[g_Language][eInvalidTimeDateSet]
#define _TouchScreenCalibrate	Strings[g_Language][eTouchScreenCalibrate]
#define _PrinterSelect	Strings[g_Language][ePrinterSelect]
#define _PleaseSelectPrinter	Strings[g_Language][ePleaseSelectPrinter]
#define _PrinterCopys	Strings[g_Language][ePrinterCopys]
#define _PleaseSelectPrinterCopys	Strings[g_Language][ePleaseSelectPrinterCopys]
#define _RestoreSet	Strings[g_Language][eRestoreSet]
#define _SystemInformation	Strings[g_Language][eSystemInformation]
#define _Hardware	Strings[g_Language][eHardware]
#define _Software	Strings[g_Language][eSoftware]
#define _DeviceNo	Strings[g_Language][eDeviceNo]
#define _CalibrateDate	Strings[g_Language][eCalibrateDate]
#define _Admin	Strings[g_Language][eAdmin]
#define _OK	Strings[g_Language][eOK]
#define __Exit	Strings[g_Language][eExit]
#define _Cancel	Strings[g_Language][eCancel]
#define _Retry	Strings[g_Language][eRetry]
#define _Yes	Strings[g_Language][eYes]
#define _No	Strings[g_Language][eNo]
#define _Default	Strings[g_Language][eDefault]
#define _EnsureNeedTouchCalibrate	Strings[g_Language][eEnsureNeedTouchCalibrate]
#define _TouchCalibrateTip	Strings[g_Language][eTouchCalibrateTip]
#define _T_Prepare	Strings[g_Language][eT_Prepare]
#define _T_Clean	Strings[g_Language][eT_Clean]
#define _T_WaitBlow	Strings[g_Language][eT_WaitBlow]
#define _T_Blowing	Strings[g_Language][eT_Blowing]
#define _T_BlowBreak	Strings[g_Language][eT_BlowBreak]
#define _T_Precessing	Strings[g_Language][eT_Precessing]
#define _Passive	Strings[g_Language][ePassive]
#define _Refuse	Strings[g_Language][eRefuse]
#define _Retest	Strings[g_Language][eRetest]
#define _AlcoholMaintainTest	Strings[g_Language][eAlcoholMaintainTest]
#define _QuickTest	Strings[g_Language][eQuickTest]
#define _CalibrationWithValue	Strings[g_Language][eCalibrationWithValue]
#define _AlcoholTest	Strings[g_Language][eAlcoholTest]
#define _Retest_Refuse	Strings[g_Language][eRetest_Refuse]
#define _Calibration	Strings[g_Language][eCalibration]
#define _AlcUnit	Strings[g_Language][eAlcUnit]
#define _AlcUnitCategory	Strings[g_Language][eAlcUnitCategory]
#define _AlcUnitType	Strings[g_Language][eAlcUnitType]
#define _AlcConvFactor	Strings[g_Language][eAlcConvFactor]
#define _BAC	Strings[g_Language][eBAC]
#define _BrAC	Strings[g_Language][eBrAC]
#define _BlowParameter	Strings[g_Language][eBlowParameter]
#define _BlowTime	Strings[g_Language][eBlowTime]
#define _Seconds	Strings[g_Language][eSeconds]
#define _Level	Strings[g_Language][eLevel]
#define _BlowStrengthLevel	Strings[g_Language][eBlowStrengthLevel]
#define _SampleModeSelect	Strings[g_Language][eSampleModeSelect]
#define _SampleModeSelectByTime	Strings[g_Language][eSampleModeSelectByTime]
#define _SampleModeSelectByEndBlow	Strings[g_Language][eSampleModeSelectByEndBlow]
#define _TimeZoneSetting	Strings[g_Language][eTimeZoneSetting]
#define _APN	Strings[g_Language][eAPN]
#define _Server	Strings[g_Language][eServer]
#define _PleaseInputZFYID	Strings[g_Language][ePleaseInputZFYID]
#define _DeviceDebug	Strings[g_Language][eDeviceDebug]
#define _Inhal	Strings[g_Language][eInhal]
#define _Calibrate	Strings[g_Language][eCalibrate]
#define _Clear	Strings[g_Language][eClear]
#define _Save	Strings[g_Language][eSave]
#define _StartTest	Strings[g_Language][eStartTest]
#define _HighConcentrationCali	Strings[g_Language][eHighConcentrationCali]
#define _LowConcentrationCali	Strings[g_Language][eLowConcentrationCali]
#define _QuickTestCali	Strings[g_Language][eQuickTestCali]
#define _PleaseInputHighStandard	Strings[g_Language][ePleaseInputHighStandard]
#define _PleaseInputLowStandard	Strings[g_Language][ePleaseInputLowStandard]
#define _PleaseInputQuickTestStandard	Strings[g_Language][ePleaseInputQuickTestStandard]
#define _InvalidStandardHConcentration	Strings[g_Language][eInvalidStandardHConcentration]
#define _InvalidStandardLConcentration	Strings[g_Language][eInvalidStandardLConcentration]
#define _TempCalibration	Strings[g_Language][eTempCalibration]
#define _DeleteCaliDataTip	Strings[g_Language][eDeleteCaliDataTip]
#define _InvalidCaliData	Strings[g_Language][eInvalidCaliData]
#define _TestReport	Strings[g_Language][eTestReport]
#define _RestoreSetTip	Strings[g_Language][eRestoreSetTip]
#define _PlateNumberInput	Strings[g_Language][ePlateNumberInput]
#define _PleaseInputDriveLicence	Strings[g_Language][ePleaseInputDriveLicence]
#define _PleaseInputPoliceID	Strings[g_Language][ePleaseInputPoliceID]
#define _TesteeNameInput	Strings[g_Language][eTesteeNameInput]
#define _TestLocationInput	Strings[g_Language][eTestLocationInput]
#define _PoliceNameInput	Strings[g_Language][ePoliceNameInput]
#define _DepartmentInput	Strings[g_Language][eDepartmentInput]
#define _PleaseInputPassword	Strings[g_Language][ePleaseInputPassword]
#define _PasswordError	Strings[g_Language][ePasswordError]
#define _NoRecord	Strings[g_Language][eNoRecord]
#define _R_RecordNumber	Strings[g_Language][eR_RecordNumber]
#define _R_Date	Strings[g_Language][eR_Date]
#define _R_Time	Strings[g_Language][eR_Time]
#define _R_Temperature	Strings[g_Language][eR_Temperature]
#define _R_TestMode	Strings[g_Language][eR_TestMode]
#define _R_ActiveTest	Strings[g_Language][eR_ActiveTest]
#define _R_PassivelTest	Strings[g_Language][eR_PassiveTest]
#define _R_QuickTest	Strings[g_Language][eR_QuickTest]
#define _R_RefuseTest	Strings[g_Language][eR_RefuseTest]
#define _R_TestResult	Strings[g_Language][eR_TestResult]
#define _R_TestRestltNormal	Strings[g_Language][eR_TestRestltNormal]
#define _R_TestRestltDrink	Strings[g_Language][eR_TestRestltDrink]
#define _R_TestRestltDrunk	Strings[g_Language][eR_TestRestltDrunk]
#define _R_Testee	Strings[g_Language][eR_Testee]
#define _R_Location	Strings[g_Language][eR_Location]
#define _R_Longitude	Strings[g_Language][eR_Longitude]
#define _R_Longitude_East	Strings[g_Language][eR_Longitude_East]
#define _R_Longitude_West	Strings[g_Language][eR_Longitude_West]
#define _R_Latitude	Strings[g_Language][eR_Latitude]
#define _R_Latitude_South	Strings[g_Language][eR_Latitude_South]
#define _R_Latitude_North	Strings[g_Language][eR_Latitude_North]
#define _R_PlateNumber	Strings[g_Language][eR_PlateNumber]
#define _R_DriverLicense	Strings[g_Language][eR_DriverLicense]
#define _R_PoliceID	Strings[g_Language][eR_PoliceID]
#define _R_PoliceName	Strings[g_Language][eR_PoliceName]
#define _R_Department	Strings[g_Language][eR_Department]
#define _Print	Strings[g_Language][ePrint]
#define _PrinterPair	Strings[g_Language][ePrinterPair]
#define _NoExcutePrint	Strings[g_Language][eNoExcutePrint]
#define _ConnectPrinter	Strings[g_Language][eConnectPrinter]
#define _ShutdownPrinter	Strings[g_Language][eShutdownPrinter]
#define _ConnectPrinterFail	Strings[g_Language][eConnectPrinterFail]
#define _PrinterNoPaper	Strings[g_Language][_PrinterNoPaper]
#define _Printing	Strings[g_Language][ePrinting]
#define _PrintFinish	Strings[g_Language][ePrintFinish]
#define _BlueToothParing	Strings[g_Language][eBlueToothParing]
#define _PairFail	Strings[g_Language][ePairFail]
#define _PairSuccessful	Strings[g_Language][ePairSuccessful]
#define _P_DeviceID	Strings[g_Language][eP_DeviceID]
#define _P_RecordNumber	Strings[g_Language][eP_RecordNumber]
#define _P_Date	Strings[g_Language][eP_Date]
#define _P_Time	Strings[g_Language][eP_Time]
#define _P_Zero	Strings[g_Language][eP_Zero]
#define _P_TestMode	Strings[g_Language][eP_TestMode]
#define _P_ActiveTest	Strings[g_Language][eP_ActiveTest]
#define _P_PassiveTest	Strings[g_Language][eP_PassiveTest]
#define _P_QucikTest	Strings[g_Language][eP_QucikTest]
#define _P_RefuseTest	Strings[g_Language][eP_RefuseTest]
#define _P_BAC	Strings[g_Language][eP_BAC]
#define _P_BrAC	Strings[g_Language][eP_BrAC]
#define _P_TestResult	Strings[g_Language][eP_TestResult]
#define _P_TestRestltNormal	Strings[g_Language][eP_TestRestltNormal]
#define _P_TestRestltDrink	Strings[g_Language][eP_TestRestltDrink]
#define _P_TestRestltDrunk	Strings[g_Language][eP_TestRestltDrunk]
#define _P_Testee	Strings[g_Language][eP_Testee]
#define _P_Location	Strings[g_Language][eP_Location]
#define _P_LongitudeLatitude	Strings[g_Language][eP_LongitudeLatitude]
#define _P_Longitude_East	Strings[g_Language][eP_Longitude_East]
#define _P_Longitude_West	Strings[g_Language][eP_Longitude_West]
#define _P_Latitude_South	Strings[g_Language][eP_Latitude_South]
#define _P_Latitude_North	Strings[g_Language][eP_Latitude_North]
#define _P_PlateNumber	Strings[g_Language][eP_PlateNumber]
#define _P_DriverSignature	Strings[g_Language][eDriverSignature]
#define _P_DriverLicense	Strings[g_Language][eP_DriverLicense]
#define _P_SubjectSignature	Strings[g_Language][eP_SubjectSignature]
#define _P_PoliceName	Strings[g_Language][eP_PoliceName]
#define _P_PoliceID	Strings[g_Language][eP_PoliceID]
#define _P_Department	Strings[g_Language][eP_Department]
#define _P_PoliceOfficerSignature	Strings[g_Language][ePoliceOfficerSignature]
#define _LowPowerTip	Strings[g_Language][eLowPowerTip]
#define _InvalidSimCardTip	Strings[g_Language][eInvalidSimCardTip]
#define _DeviceIMEINotRegister	Strings[g_Language][eDeviceIMEINotRegister]
#define _UserName	Strings[g_Language][eUserNmae]
#define _UserID	Strings[g_Language][eUserID]
#define _UserDepartment	Strings[g_Language][eUserDepartment]
#define _UserBasicInfor	Strings[g_Language][eUserBasicInfor]
#define _UserManage	Strings[g_Language][eUserManage]
#define _DeleteAllUserInfor	Strings[g_Language][eDeleteAllUserInfor]
#define _ConfirmDeleteAllUserInfor	Strings[g_Language][eConfirmDeleteAllUserInfor]
#define _AlertRegisterUserInfor	Strings[g_Language][eAlertRegisterUserInfor]
#define _PleaseInputName	Strings[g_Language][ePleaseInputName]
#define _PleaseInputID	Strings[g_Language][ePleaseInputID]
#define _PleaseInputDepartment	Strings[g_Language][ePleaseInputDepartment]
#define _PleaseSetLoginPassword	Strings[g_Language][ePleaseSetLoginPassword]
#define _PleaseSetVerifyLoginPassword	Strings[g_Language][ePleaseSetVerifyLoginPassword]
#define _PleaseInputLoginPassword	Strings[g_Language][ePleaseInputLoginPassword]
#define _MaintainMode	Strings[g_Language][eMaintainMode]
#define _AlertInputValidName	Strings[g_Language][eAlertInputValidName]
#define _AlertInputValidPassword	Strings[g_Language][eAlertInputValidPassword]
#define _AlertInputValidVerifyPassword	Strings[g_Language][eAlertInputValidVerifyPassword]
#define _HeartRateModeSelect    Strings[g_Language][eHeartRateSelect]


#endif

