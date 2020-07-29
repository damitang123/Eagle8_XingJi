#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include "stm32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define EAGLE_1		0
#define EAGLE_8		1

#define __DEBUG__

#ifdef __DEBUG__
	#define TRACE_PRINTF  User_Debug_Printf
	#define TRACE_DEBUG()	do{User_Debug_Printf("%s,%s,%d\r\n",__FILE__,__FUNCTION__,__LINE__);}while(0)
	void User_Debug_Printf(const char *fmt, ...);
#else
	#define TRACE_PRINTF(...)
	#define TRACE_DEBUG()
#endif

#define  ARR_SIZE( _a )  ( sizeof( (_a) ) / sizeof( (_a[0]) ) )
#define  STRUCT_OFFSET(type,field) ((unsigned long)&((type *)0)->field)
#define  MAX(a,b)	((a) > (b) ? (a) : (b))

#define LCD_X_SIZE					240
#define LCD_Y_SIZE					320

/*ϵͳ�������ú�*/
#define LAN_ENGLISH					0
#define LAN_CHINESE					1
#define LAN_RUSSIAN					2
#define LAN_VIETNAMESE				3
#define LAN_SPANISH					4

/*Ĭ������������*/
#define SYSTEM_LANGUAGE			LAN_CHINESE

/*����ʹ��4G��PCB*/
#define USE_4G_PCB	

#if defined(USE_4G_PCB)
//#warning "Please confirm select 4G model!"
#endif

#if (EAGLE_1 + EAGLE_8 ) != 1
#error "Only one device type can be configed!\r\n"
#endif

#if EAGLE_8
#define QUICK_TEST	1//��ӥ8�Ŵ����ĵĹ��ܣ������Ҫ�� 20181106
#endif

/*ʹ���¹���2017��ʱ����������*/
#define USE_GB_T21254_2017			1

#if USE_GB_T21254_2017
	/*�Ƿ�ʹ�ø�ѹ����.*/
	//#define USE_NEGATIVE_PRESSURE_PIPE //����Ҫ��ѹ��

	#if defined(USE_NEGATIVE_PRESSURE_PIPE)
		#warning "USE_NEGATIVE_PRESSURE_PIPE is config!"//�������Ҫ��Ҫ ��Ҫȷ�� added by lxl 20180814
	#endif
#endif


/*ʹ�������滻ͼ��.*/
#define USE_TEXT_REPLACE_ICON		1

/*�Ƿ�����GPRSģ��*/
#define USE_GPRS					1

/*������ʾ��ȡ֤���Ծƾ�Ũ��ֵ*/
#define MAX_DISPLAY_ALC_VALUE		550

/*������ʾ�Ŀ��žƾ�Ũ��ֵ*/
#define MAX_QUICK_TEST_DISPLAY_ALC_VALUE			100.0



#if EAGLE_1

#define FW_VERSION_LOGO			"V300"

#define HW_VERSION					"HW_V2017_1218"

#define FW_VERSION					"ST_Eagle1_V302"

#if defined(USE_4G_PCB)
	#undef FW_VERSION
	#define FW_VERSION				"ST_Eagle1_4G_V300"
#endif

#elif EAGLE_8

#define FW_VERSION_LOGO				"V307" //"V306"  �Ǽʿع�Ϊ306 2020529

#define HW_VERSION					"HW_V2017_1218"

#define FW_VERSION					"ST_Eagle8_V302" 	

#if defined(USE_4G_PCB)
	#undef FW_VERSION
	#define FW_VERSION				"ST_Eagle8_4G_V307" //"ST_Eagle8_4G_V306" 20200602
#endif

#endif



/***********************************************************************************************************/
/*�Ƿ�������������Խ��,ֻҪ���˲��Խ����һ������.*/
#define ALWAYS_SAVE_TEST_RESULT	 1 //�˳����ز⣬�ػ�����Ҫ����Modified by lxl 20180817

/*LCD��������*/
#define LCD_DRIVER_HX8347_D			0
#define LCD_DRIVER_ILI9341			1	/*����*/
#if LCD_DRIVER_HX8347_D + LCD_DRIVER_ILI9341 != 1
#error "Please select the correct lcd driver ic!"
#endif


/*����������*/
#define SERVER_USE_KEYUN_DAI		0	/*ʹ�ÿ��˴���������*/
#define SERVER_USE_KEYUN_YE_NEW	0	/*ʹ�ÿ���Ҷ���·�����*/
#define SERVER_USE_XING_JIKONG_GU	1	/*ʹ���Ǽʿعɷ�����*/

#if (SERVER_USE_KEYUN_YE_NEW + SERVER_USE_KEYUN_DAI+SERVER_USE_XING_JIKONG_GU) != 1
#error "Only one server can be configed!\r\n"
#endif

/*����ģ������*/
#define USE_G100						0
#define USE_SIM800C					0
#define USE_4G							1

#if (USE_G100 + USE_SIM800C +  USE_4G) != 1
#error "Only one GPRS model can be config!"
#endif

/*�����ǿͻ�����***********************************************************************************************/
#define CUST_CHINA_COMMON					/*���Ĺ���*/
//#define CUST_CHINA_TAIYUAN					/*̫ԭ�ͻ�.��Ӣ��*/
//#define CUST_CHINA_SZJAZN					/*���ھ�������--��С��*/
//#define CUST_CHINA_EAGLE8_TO_EAGLE1		/*��ӥ8�Ÿ���ӥ1��*/


/*̫ԭ�ͻ�.��Ӣ��*/
#if defined(CUST_CHINA_TAIYUAN)

#if SERVER_USE_KEYUN_DAI != 1
#error "Please select the correct server config!\r\n";
#endif

#undef ALWAYS_SAVE_TEST_RESULT
#define ALWAYS_SAVE_TEST_RESULT 	1

#endif



/***********************************************************************************************************/

#endif

