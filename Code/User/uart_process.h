#ifndef _USB_PROCESS_H_
#define _USB_PROCESS_H_

typedef enum
{
	CMD_LINK_TICK = 0x01,
	/*���Բ���*/
	CMD_GET_TEST_PARAM,
	CMD_SET_TEST_PARAM,
	CMD_RECOVER_DEFAULT_TEST_PARAM,
	/*ϵͳ����*/
	CMD_GET_SYSTEM_PARAM,
	CMD_SET_SYSTEM_PARAM,
	CMD_RECOVER_DEFAULT_SYSTEM_PARAM,
	/*��ӡ����*/
	CMD_GET_PRINTER_PARAM,
	CMD_SET_PRINTER_PARAM,
	CMD_RECOVER_DEFAULT_PRINTER_PARAM,
	/*�ƾ�У׼����*/
	CMD_GET_ALC_CALIBRATION_PARAM,
	CMD_SET_ALC_CALIBRATION_PARAM,
	CMD_RECOVER_DEFAULT_ALC_CALIBRATION_PARAM,
	/*�¶�У׼����*/
	CMD_GET_TEMP_CALIBRATION_PARAM,
	CMD_SET_TEMP_CALIBRATION_PARAM,
	CMD_RECOVER_DEFAULT_TEMP_CALIBRATION_PARAM,
	/*�豸ID����*/
	CMD_GET_DEVICE_ID_PARAM,
	CMD_SET_DEVICE_ID_PARAM,
	CMD_RECOVER_DEFAULT_DEVICE_ID_PARAM,
	/*����������*/
	CMD_GET_SERVER_PARAM,
	CMD_SET_SERVER_PARAM,
	CMD_RECOVER_DEFAULT_SERVER_PARAM,
	/*��¼��ز���*/	
	CMD_GET_RECORDS_COUNTER,
	CMD_EXPORT_RECORDS_REQUEST,						
	CMD_EXPORT_RECORDS_PHASE1,						
	CMD_EXPORT_RECORDS_PHASE2,						
	CMD_BREAK_RECORDS_EXPORT_RECORDS,				
	CMD_EXPORT_RECORD_BY_ID,							
	CMD_IMPORT_RECORDS_BY_ID,						
	CMD_CLEAR_ALL_RECORDS,
	/*����*/
	CMD_SET_SYSTEM_TIME,
	CMD_SET_LCD_BRIGHTNESS,
	CMD_SET_VOICE_VOLUME,
	/*�û���½��Ϣ*/
	CMD_SET_USER_LOGIN_INFORMATION,
	CMD_GET_USER_LOGIN_INFORMATION_BY_ID,
	CMD_DEL_USER_LOGIN_INFORMATION,
	/*�ܳ���ʱ��*/
	CMD_GET_TOTAL_INHALE_TIIME_COUNTER,
	CMD_CLEAR_TOTAL_INHALE_TIIME_COUNTER,
	/*�ܲ��Դ���*/
	CMD_GET_TOTAL_TEST_COUNTER,
	CMD_CLEAR_TOTAL_TEST_COUNTER,

	/*USB���ݵ���¼��.*/
	CMD_START_EXPORT_RECORDS_USB,					/*��λ������Ҫʵ��-ռλ*/
	CMD_EXPORT_RECORDS_BASIC_DATA_USB,				/*��λ������Ҫʵ��-ռλ*/
	CMD_EXPORT_RECORDS_IMAGE_HEADER_USB,			/*��λ������Ҫʵ��-ռλ*/
	CMD_EXPORT_RECORDS_IMAGE_DATA_USB,			/*��λ������Ҫʵ��-ռλ*/
	CMD_EXPORT_RECORDS_FINGERPRINT_DATA_USB,		/*��λ������Ҫʵ��-ռλ*/
	CMD_CORFIRM_RECORDS_VALID_EXPORT_USB,			/*��λ������Ҫʵ��-ռλ*/
	CMD_FINISH_EXPORT_RECORDS_USB,					/*��λ������Ҫʵ��-ռλ*/
	CMD_CANCEL_EXPORT_RCORDS_USB,					/*��λ������Ҫʵ��-ռλ*/

	/*�����豸ʹ���������Ϣ*/
	CMD_SET_USER_DEVICE_INFOR,
	CMD_GET_USER_DEVICE_INFOR,
	CMD_RECOVER_USER_DEVICE_INFOR,

	/*ͨ��USB�ϴ���¼��������*/
	CMD_GET_DEVICE_RECORDS_INFOR,
	CMD_NOTIFY_RCORD_SUCCEED_UPLOAD,

	CMD_TRACE_LOG_ENABLE = 0x78,
	CMD_WRITE_CALIBRATION_PARAMETER = 0x79,
	
	CMD_RESTART_SYSTEM = 0x7A,

	CMD_DELETE_LOGO_PARAMETER = 0x7C,
	CMD_FILE_DESCRIPTION = 0x7D,
	CMD_FILE_GRUOPS = 0x7E,
}eConsoleCmd;

typedef enum
{
	ACK_LINK_TICK = 0x81,
	/*���Բ���*/
	ACK_GET_TEST_PARAM,
	ACK_SET_TEST_PARAM,
	ACK_RECOVER_DEFAULT_TEST_PARAM,
	/*ϵͳ����*/
	ACK_GET_SYSTEM_PARAM,
	ACK_SET_SYSTEM_PARAM,
	ACK_RECOVER_DEFAULT_SYSTEM_PARAM,
	/*��ӡ����*/
	ACK_GET_PRINTER_PARAM,
	ACK_SET_PRINTER_PARAM,
	ACK_RECOVER_DEFAULT_PRINTER_PARAM,
	/*�ƾ�У׼����*/
	ACK_GET_ALC_CALIBRATION_PARAM,
	ACK_SET_ALC_CALIBRATION_PARAM,
	ACK_RECOVER_DEFAULT_ALC_CALIBRATION_PARAM,
	/*�¶�У׼����*/
	ACK_GET_TEMP_CALIBRATION_PARAM,
	ACK_SET_TEMP_CALIBRATION_PARAM,
	ACK_RECOVER_DEFAULT_TEMP_CALIBRATION_PARAM,
	/*�豸ID����*/
	ACK_GET_DEVICE_ID_PARAM,
	ACK_SET_DEVICE_ID_PARAM,
	ACK_RECOVER_DEFAULT_DEVICE_ID_PARAM,
	/*����������*/
	ACK_GET_SERVER_PARAM,
	ACK_SET_SERVER_PARAM,
	ACK_RECOVER_DEFAULT_SERVER_PARAM,
	/*��¼��ز���*/	
	ACK_GET_RECORDS_COUNTER,
	ACK_EXPORT_RECORDS_BEGIN,
	ACK_EXPORT_RECORDS_TRANSMITTING,
	ACK_EXPORT_RECORDS_FINISH,
	ACK_BREAK_RECORDS_EXPORT_RECORDS,
	ACK_EXPORT_RECORD_BY_ID,
	ACK_IMPORT_RECORDS_BY_ID,
	ACK_CLEAR_ALL_RECORDS,
	/*����*/
	ACK_SET_SYSTEM_TIME,
	ACK_SET_LCD_BRIGHTNESS,
	ACK_SET_VOICE_VOLUME,
	/*�û���½��Ϣ*/
	ACK_SET_USER_LOGIN_INFORMATION,
	ACK_GET_USER_LOGIN_INFORMATION_BY_ID,
	ACK_DEL_USER_LOGIN_INFORMATION,
	/*�ܳ���ʱ��*/
	ACK_GET_TOTAL_INHALE_TIIME_COUNTER,
	ACK_CLEAR_TOTAL_INHALE_TIIME_COUNTER,
	/*�ܲ��Դ���*/
	ACK_GET_TOTAL_TEST_COUNTER,
	ACK_CLEAR_TOTAL_TEST_COUNTER,

	/*USB���ݵ���¼��.*/
	ACK_START_EXPORT_RECORDS_USB,
	ACK_EXPORT_RECORDS_BASIC_DATA_USB,
	ACK_EXPORT_RECORDS_IMAGE_HEADER_USB,
	ACK_EXPORT_RECORDS_IMAGE_DATA_USB,
	ACK_EXPORT_RECORDS_FINGERPRINT_DATA_USB,
	ACK_CORFIRM_RECORDS_VALID_EXPORT_USB,
	ACK_FINISH_EXPORT_RECORDS_USB,
	ACK_CANCEL_EXPORT_RCORDS_USB,

	/*�����豸ʹ���������Ϣ*/
	ACK_SET_USER_DEVICE_INFOR,
	ACK_GET_USER_DEVICE_INFOR,
	ACK_RECOVER_USER_DEVICE_INFOR,

	/*ͨ��USB�ϴ���¼��������*/
	ACK_GET_DEVICE_RECORDS_INFOR,
	ACK_NOTIFY_RCORD_SUCCEED_UPLOAD,

	ACK_TRACE_LOG_ENABLE = 0xF8,
	ACK_WRITE_CALIBRATION_PARAMETER = 0xF9,

	ACK_RESTART_SYSTEM = 0xFA,

	ACK_DELETE_LOGO_PARAMETER = 0xFC,
	ACK_FILE_DESCRIPTION = 0xFD,
	ACK_FILE_GRUOPS = 0xFE,
        
	ACK_USB_LOG = 0xFF,
}eConsoleAckCmd;

void Uart_Usb_Cmd_Parser(void);
void Receive_Data_From_UART1(unsigned char RevData);

#endif