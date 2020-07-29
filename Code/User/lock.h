#ifndef _LOCK_H_
#define _LOCK_H_


#define STATE_OK					0
#define STATE_ALARM				1
#define STATE_LOCKED				2


extern unsigned char g_nCalibrationState;
extern unsigned short g_nCalibrattionLockDays;

extern unsigned char g_nServiceState;
extern unsigned short g_nServiceLockDays;

extern unsigned char g_nFirstTestState;
extern unsigned short g_nFirstTestLockDays;

extern unsigned char g_nRollingTestState;
extern unsigned short g_nRollingTestLockDays;

void CheckCalibrationState(void);
void DisplayCalibrationState(void);
void CheckServiceState(void);
void DisplayServiceState(void);
void CheckFirstTestState(void);
void DisplayFirstTestState(void);
void CheckRollingTestState(void);
void DisplayRollingTestState(void);
void Dispay_Contact_PhoneNumber(void);

#endif
