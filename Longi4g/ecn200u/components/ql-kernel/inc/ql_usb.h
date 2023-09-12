
/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
    
/*=================================================================

						EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN			  WHO		  WHAT, WHERE, WHY
------------	 -------	 -------------------------------------------------------------------------------

=================================================================*/


#ifndef QL_USB_H
#define QL_USB_H


#include "ql_api_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define USB_DETECT_TIME_MIN             0
#define USB_DETECT_TIME_MAX             100000000
#define USB_CHARGING_ONFF_TIME_MIN      1000
#define USB_CHARGING_ONFF_TIME_MAX      100000





/*===========================================================================
 * Enum
 ===========================================================================*/
typedef enum
{
	QL_USB_SUCCESS						= QL_SUCCESS,							 /*  operating is successful  */
	QL_USB_INVALID_PARAM				= (QL_COMPONENT_BSP_USB << 16) | 1000,   /*  invalid input param  */
	QL_USB_SYS_ERROR					= (QL_COMPONENT_BSP_USB << 16) | 1001,	 /*  system error  */
	QL_USB_DETECT_SAVE_NV_ERR           = (QL_COMPONENT_BSP_USB << 16) | 1002,	 /*  save detect time to NV failed */
	QL_USB_NO_SPACE						= (QL_COMPONENT_BSP_USB << 16) | 1003,   /*  no space to store data  */
	QL_USB_NOT_SUPPORT					= (QL_COMPONENT_BSP_USB << 16) | 1004,   /*  current operation not support  */
}ql_errcode_usb_e;

typedef enum
{
	QL_USB_HOTPLUG_OUT = 0,    //USB in plug out state
	QL_USB_HOTPLUG_IN  = 1	   //USB was inserted
}QL_USB_HOTPLUG_E;

typedef enum
{
	QL_USB_DET_MIN = 0,
	QL_USB_DET_CHARGER = 0,	   //usb will be enabled and disabled by VBUS high level and low level 
    QL_USB_DET_AON = 1,		   //usb will be enabled always, and won't be disabled 
    QL_USB_DET_NO_VBUS = 2,    //usb will be enabled and disabled by GPIO, the GPIO function not support now, so it is same as QL_USB_DET_AON now

	QL_USB_DET_MAX,
}QL_USB_DET_E;

typedef enum
{
	QL_USB_ENUM_NONE 	= -1,		//NO USB port will be enumerated

	QL_USB_ENUM_USBNET_COM, 		//usbnet(rndis/ecm), USB_AT, USB_DIAG, USB_MOS, USB_AP_LOG, USB_CP_LOG, USB_MODEM, USB_NMEA				
	QL_USB_ENUM_USBNET_COM_UAC, 	//usbnet(rndis/ecm), USB_AT, USB_DIAG, USB_MOS, USB_AP_LOG, USB_CP_LOG, USB_MODEM, UAC
	QL_USB_ENUM_USBNET_COM_PRINTER, //usbnet(rndis/ecm), USB_AT, USB_DIAG, USB_MOS, USB_AP_LOG, USB_CP_LOG, USB_MODEM, USB_PRINTER

	QL_USB_ENUM_MAX, 				//invalid
}QL_USB_ENMU_MODE_E;


typedef enum
{
	QL_USB_NV_TYPE_ENUM_MODE = 0,

	QL_USB_NV_TYPE_MAX,
}QL_USB_NV_TYPE_E;

/*===========================================================================
 * STRUCT
 ===========================================================================*/
typedef struct
{
	QL_USB_DET_E det_mode;
	uint 		 det_pin;    //not used now, but will used feature
	uint 		 reserve[2]; //reserved for futher using
}ql_usb_detmdoe_t;           //this structure will be writed to NV

typedef struct
{
    uint32_t usb_detect_time;
    uint32_t charging_on_time;
    uint32_t charging_off_time;
	ql_usb_detmdoe_t usb_det_mode;
} ql_usb_setting_s;

typedef struct
{
	uint8 enum_mode;
	uint reserved[4];
}ql_usb_nv_t;

/*===========================================================================
 * function
 ===========================================================================*/

/*****************************************************************
	!!!!!   don't  block the callback , as is run in interrupt   !!!!!!
* Function: ql_usb_hotplug_cb
*
* Description: the defination of usb hotplug callback
* 
* Parameters:
* 	state	  		Indicates whether the USB action is unplugged or inserted 
*	ctx         	not used now 
*
* Return:
* 	0
*****************************************************************/
typedef int(*ql_usb_hotplug_cb)(QL_USB_HOTPLUG_E state, void *ctx); //

/*****************************************************************
* 
* Function: ql_usb_bind_hotplug_cb
*
* Description: bind usb hotplug callback to kernel
* 
* Attention:
   1. the callback will be run in interrupt, so it is forbidden to block the callback;
   2. it is forbidden to call Audio start/stop/close， file write/read，CFW（releated to RPC）in interrupt;
   3. it is forbidden to enter critical in interrupt
   4. it is suggested for users to  perform simple operations , or send event(no timeout) to inform your task in interrupt

* Parameters:
* 	hotplug_callback	  [in]callback
*
* Return:
* 	0
*****************************************************************/
ql_errcode_usb_e ql_usb_bind_hotplug_cb(ql_usb_hotplug_cb hotplug_callback);

/*****************************************************************
* Function: ql_usb_get_hotplug_state
*
* Description: get the usb hotplug state
* 
* Parameters:
* 	hotplug_callback	  [in]callback
*
* Return:
* 	QL_USB_HOTPLUG_OUT   : USB is not insrrted
*	QL_USB_HOTPLUG_IN    : USB is inserted
*****************************************************************/
QL_USB_HOTPLUG_E ql_usb_get_hotplug_state(void);

/*****************************************************************
* Function: ql_usb_set_detmode
*
* Description: set usb detect mode
* 
* Parameters:
* 	usb_mode    [in] detected mode, take effect after reset
*
* Return:
*   0:          success
*   others:     ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_set_detmode(ql_usb_detmdoe_t *usb_mode);

/*****************************************************************
* Function: ql_usb_get_detmode
*
* Description: get the usb detect mode
* 
* Parameters:
* 	ql_usb_detmdoe_t	[out]usb detect mode
*
* Return:
*   0:          success
*   others:     ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_get_detmode(ql_usb_detmdoe_t *mode);

/*****************************************************************
* Function: ql_set_usb_detect_max_time
*
* Description: Set the maximum time to detect the connection of 
*              USB DP/DM data line after USB insertion
*              设置USB插入后最大的检测时间，如果过了这个时间，还没有
*              检测到DP/DM线上有数据返回，则关闭检测，省电
* 
* Parameters:
*   ms      [in] the maximum detect time to set
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*
*****************************************************************/
ql_errcode_usb_e ql_set_usb_detect_max_time(uint32_t ms);

/*****************************************************************
* Function: ql_get_usb_detect_max_time
*
* Description: get the maximum USB detection time set
*              
* Parameters:
*   ms      [out]  the maximum detect time
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*
*****************************************************************/
ql_errcode_usb_e ql_get_usb_detect_max_time(uint32_t *ms);

/*****************************************************************
* Function: ql_usb_get_enum_mode
*
* Description: to get the usb enumeration mode             
*
* Return:
*	QL_USB_ENMU_MODE_E
*
*****************************************************************/
QL_USB_ENMU_MODE_E ql_usb_get_enum_mode(void);

/*****************************************************************
* Function: ql_usb_set_enum_mode
*
* Description: to set the enumeration mode of usb port.For UAC or
			   usb printer mode, the USB nmea PORT will not be enumerated		   
*
* Parameters:
*	mode	   [in] see it in QL_USB_ENMU_MODE_E for detail
*
* Return:
*   0          success
*   other      ql_errcode_usb_e
*****************************************************************/
ql_errcode_usb_e ql_usb_set_enum_mode(QL_USB_ENMU_MODE_E mode);


#ifdef __cplusplus
} /*"C" */
#endif

#endif /* QL_USB_H */


