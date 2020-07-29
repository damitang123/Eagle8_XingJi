#include "platform.h"
#include "delay.h"

typedef struct
{
	unsigned long		nFileSize;
	unsigned long 		nFlashStartAddress;
	unsigned long		nFileChecksum;		/*����BIN�ļ���У���*/
	char				aDeviceString[20];	/*�豸�����ַ���*/
	char 			aFWVerionString[6];	/*����汾�����ַ���2+3�ֽ�*/	
	char 			aCustomCode[5]; 	/*��AAAA��ʼ���*/
}__attribute__ ((packed)) sProgramFileDesc;

void Enter_Upgrade_Mode(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	BKP_ClearFlag();
	BKP_WriteBackupRegister(0x10, 0x9A55); /*ϵͳ��λ�󣬱��ݼĴ����е�ֵ������λ���ı�*/
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

