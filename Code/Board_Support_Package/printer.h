#ifndef _PRINTER_H_
#define _PRINTER_H_

typedef enum
{
	ePrinterIntegrated	= 0,
	ePrinterBluetooth	= 1,
	ePrinterNeedle		= 2,
}ePrinterType;

typedef enum
{
	eNoPairAction = 0,
	ePairFailed,
	ePairSuccessed
} EBlueToothPairStatus;

typedef enum
{
	ePrinterIdle,
	ePrinterStart,
	ePrinterReset,
	ePrinterWaitEnterPairMode,
	ePrinterExitSetMode,
	ePrinterWaitEnterSetMode,
	ePrinterWaitPairSuccessful,
	ePairSuccessfulDelay,
	ePrinterPairConnectFail,
	ePrinterSendPrintData,
	ePrinterSendPirntReq,
	ePrinterWaitPrintFinish,
	ePrinterOutOfPaper,
	ePrinterSendPrintPhotoData,
	ePrinterSendPrintPhotoReq,
	ePrinterFinish,
	ePrinterExit,
} ePrinterState;

typedef enum
{
	eUARTPhaseHead0,
	eUARTPhaseHead1,
	eUARTPhaseCmd,
	eUARTPhaseDataLength,
	eUARTPhaseData,
	eUARTPhaseCrc1,
	eUARTPhaseCrc2,
	eUARTPhaseEnd,
} eUARTRecievePhase;

typedef struct
{
	bool bReadyOK;
	bool bOutOfPaper;
	bool bFinishPrint;
}sPrinter;

extern sPrinter g_sPrinter;

void Printer_Init(void);
void Printer_Shutdown(void);
void Printer_Process_Command(void);
void Printer_Uart_Send(unsigned char ch);


#endif
