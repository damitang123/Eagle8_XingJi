#include "platform.h"
#include "delay.h"

typedef struct
{
	unsigned long		nFileSize;
	unsigned long 		nFlashStartAddress;
	unsigned long		nFileChecksum;		/*整个BIN文件的校验和*/
	char				aDeviceString[20];	/*设备描述字符串*/
	char 			aFWVerionString[6];	/*软件版本描述字符串2+3字节*/	
	char 			aCustomCode[5]; 	/*从AAAA开始编号*/
}__attribute__ ((packed)) sProgramFileDesc;

void Enter_Upgrade_Mode(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	BKP_ClearFlag();
	BKP_WriteBackupRegister(0x10, 0x9A55); /*系统复位后，备份寄存器中的值不会因复位而改变*/
	delay_ms(100);
	NVIC_SystemReset();
}

#if EAGLE_1
const sProgramFileDesc g_sProgramFileDesc __attribute__((at(0x08003800))) = 
{
	0,
	0x08003000,
	0,
	"Eagle1",
	"01001",
	"AAAA"
};

#endif

#if EAGLE_8
const sProgramFileDesc g_sProgramFileDesc __attribute__((at(0x08003800))) = 
{
	0,
	0x08003000,
	0,
	"Eagle8",
	"01001",
	"AAAA"
};
#endif

