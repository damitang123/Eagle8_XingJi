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

/*系统语言配置宏*/
#define LAN_ENGLISH					0
#define LAN_CHINESE					1
#define LAN_RUSSIAN					2
#define LAN_VIETNAMESE				3
#define LAN_SPANISH					4

/*默认是中文语言*/
#define SYSTEM_LANGUAGE			LAN_CHINESE

/*定义使用4G的PCB*/
#define USE_4G_PCB	

#if defined(USE_4G_PCB)
//#warning "Please confirm select 4G model!"
#endif

#if (EAGLE_1 + EAGLE_8 ) != 1
#error "Only one device type can be configed!\r\n"
#endif

#if EAGLE_8
#define QUICK_TEST	1//天鹰8号带快拍的功能，这个需要打开 20181106
#endif

/*使用新国标2017的时候这样定义*/
#define USE_GB_T21254_2017			1

#if USE_GB_T21254_2017
	/*是否使用负压吹管.*/
	//#define USE_NEGATIVE_PRESSURE_PIPE //不需要负压管

	#if defined(USE_NEGATIVE_PRESSURE_PIPE)
		#warning "USE_NEGATIVE_PRESSURE_PIPE is config!"//这个到底要不要 需要确认 added by lxl 20180814
	#endif
#endif


/*使用文字替换图标.*/
#define USE_TEXT_REPLACE_ICON		1

/*是否启用GPRS模块*/
#define USE_GPRS					1

/*最大可显示的取证测试酒精浓度值*/
#define MAX_DISPLAY_ALC_VALUE		550

/*最大可显示的快排酒精浓度值*/
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

#define FW_VERSION_LOGO				"V307" //"V306"  星际控股为306 2020529

#define HW_VERSION					"HW_V2017_1218"

#define FW_VERSION					"ST_Eagle8_V302" 	

#if defined(USE_4G_PCB)
	#undef FW_VERSION
	#define FW_VERSION				"ST_Eagle8_4G_V307" //"ST_Eagle8_4G_V306" 20200602
#endif

#endif



/***********************************************************************************************************/
/*是否无条件保存测试结果,只要有了测试结果就一定保存.*/
#define ALWAYS_SAVE_TEST_RESULT	 1 //退出和重测，关机都需要保存Modified by lxl 20180817

/*LCD驱动定义*/
#define LCD_DRIVER_HX8347_D			0
#define LCD_DRIVER_ILI9341			1	/*新屏*/
#if LCD_DRIVER_HX8347_D + LCD_DRIVER_ILI9341 != 1
#error "Please select the correct lcd driver ic!"
#endif


/*服务器配置*/
#define SERVER_USE_KEYUN_DAI		0	/*使用科运戴工服务器*/
#define SERVER_USE_KEYUN_YE_NEW	0	/*使用科运叶工新服务器*/
#define SERVER_USE_XING_JIKONG_GU	1	/*使用星际控股服务器*/

#if (SERVER_USE_KEYUN_YE_NEW + SERVER_USE_KEYUN_DAI+SERVER_USE_XING_JIKONG_GU) != 1
#error "Only one server can be configed!\r\n"
#endif

/*上网模块配置*/
#define USE_G100						0
#define USE_SIM800C					0
#define USE_4G							1

#if (USE_G100 + USE_SIM800C +  USE_4G) != 1
#error "Only one GPRS model can be config!"
#endif

/*以下是客户配置***********************************************************************************************/
#define CUST_CHINA_COMMON					/*中文公版*/
//#define CUST_CHINA_TAIYUAN					/*太原客户.赵英会*/
//#define CUST_CHINA_SZJAZN					/*深圳警安智能--王小锋*/
//#define CUST_CHINA_EAGLE8_TO_EAGLE1		/*天鹰8号改天鹰1号*/


/*太原客户.赵英会*/
#if defined(CUST_CHINA_TAIYUAN)

#if SERVER_USE_KEYUN_DAI != 1
#error "Please select the correct server config!\r\n";
#endif

#undef ALWAYS_SAVE_TEST_RESULT
#define ALWAYS_SAVE_TEST_RESULT 	1

#endif



/***********************************************************************************************************/

#endif

