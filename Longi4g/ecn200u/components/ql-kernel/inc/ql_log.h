/**  @file
  ql_log.h

  @brief
  This file is used to define log interface for different Quectel Project.

*/

/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/
	
#ifndef QL_LOG_H
#define QL_LOG_H
	
#include "osi_log.h"
#include "ql_api_common.h"

#ifdef __cplusplus
extern "C" {
#endif


/*========================================================================
 *  Macro Definition
 *========================================================================*/
#define QL_LOG_ERRCODE_BASE (QL_COMPONENT_LOG<<16)

#define QL_LOG_LEVEL_NEVER		OSI_LOG_LEVEL_NEVER
#define QL_LOG_LEVEL_ERROR		OSI_LOG_LEVEL_ERROR
#define QL_LOG_LEVEL_WARN		OSI_LOG_LEVEL_WARN
#define QL_LOG_LEVEL_INFO		OSI_LOG_LEVEL_INFO
#define QL_LOG_LEVEL_DEBUG		OSI_LOG_LEVEL_DEBUG
#define QL_LOG_LEVEL_VERBOSE	OSI_LOG_LEVEL_VERBOSE

#define QL_LOG_OUTPUT_LEVEL		QL_LOG_LEVEL_INFO
#define QL_LOG_TAG				OSI_MAKE_LOG_TAG('Q', 'O', 'P', 'N')

#define QL_LOG_PRINTF(level, fmt, ...)                                          \
    do                                                                          \
    {                                                                           \
        if (QL_LOG_OUTPUT_LEVEL >= level)                                       \
            osiTracePrintf((level << 28) | (QL_LOG_TAG), fmt, ##__VA_ARGS__);   \
    } while (0)
			
#define QL_LOG_PUSH(module_str, msg, ...)			QL_LOG_PRINTF(QL_LOG_OUTPUT_LEVEL, "%s:%d "msg"", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define QL_LOG(level, module_str, msg, ...)			QL_LOG_PRINTF(level, "%s %d "msg"", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#undef printf
#define printf(fmt, args...)	QL_LOG_PRINTF(QL_LOG_OUTPUT_LEVEL, fmt, ##args)

#define QUEC_FSTRACE_SUPPORT_NOR_FLASH 0  //Saving logs to Norflash is not supported by default, because the capacity is too small
                                          //This configuration does not take effect in the SDK. You need to generate a new SDK

//[0]: aplog, [1] profile, [2] bsdump, [3] cplog, [4] zsplog, [5] moslog, [11] debug/usb [12] sdcard, [13]nandflash, [14]spi4nor, [15]spi6nor
#define QUEC_FS_TRACE_PORT_BEFORE   (1<<11)
#define QUEC_FS_TRACE_PORT_SD       (1<<12)
#define QUEC_FS_TRACE_PORT_NAND     (1<<13)
#define QUEC_FS_TRACE_PORT_EXTNOR   (1<<14)
#define QUEC_FS_TRACE_PORT_EXT      (1<<15)

#define FSTRACE_FNAME_MAX (64)
#define FS_MOUNT_POINT_MAX (16)     //Maximum length of mount path

/*========================================================================
 *  Enumeration Definition
 *========================================================================*/
//log component detail errcode, 4 bytes
typedef enum
{
    QL_LOG_SUCCESS = QL_SUCCESS,
    
    QL_LOG_EXECUTE_ERR                      = 1|QL_LOG_ERRCODE_BASE,
    QL_LOG_MEM_ADDR_NULL_ERR,
    QL_LOG_INVALID_PARAM_ERR,
    QL_LOG_BUSY_ERR,
    QL_LOG_SEMAPHORE_CREATE_ERR,
    QL_LOG_SEMAPHORE_TIMEOUT_ERR,
    
    QL_LOG_PORT_SET_ERR                    = 15|QL_LOG_ERRCODE_BASE,
} ql_errcode_log_e;

typedef enum
{
    QL_LOG_PORT_NONE = 0x0,
    QL_LOG_PORT_UART = 0x1,
    QL_LOG_PORT_USB  = 0x2,
    QL_LOG_PORT_SDCARD = 0x4,    //SDCARD
    QL_LOG_PORT_EXNAND = 0x5,    //NAND FLASH
#if QUEC_FSTRACE_SUPPORT_NOR_FLASH
    QL_LOG_PORT_EXNOR  = 0x6,   //SPI4 NOR FLASH
    QL_LOG_PORT_EXT    = 0x7,   //SPI6 NOR FLASH
#endif
}ql_log_port_e;

typedef struct
{
    int64 total_size_max;
    int aplog_fsize_max;                      // aplog each file size
    int cplog_fsize_max;                      // cplog each file size
    int moslog_fsize_max;                     // moslog each file size
    int prolog_fsize_max;                     // profile each file size
    char fs_trace_mount_point_len;
    char fs_trace_mount_point[FS_MOUNT_POINT_MAX]; //"/sdcard0" "/exnand" "/exnor" "/ext"
    char fstrace_fname[FSTRACE_FNAME_MAX];
}quec_fs_trace_path_t;

typedef enum
{
    QL_LOG_PCAP_NONE        = 0,
    QL_LOG_PCAP_PPP         = (1 << 0),
    QL_LOG_PCAP_ECM_RNDIS   = (1 << 1),
}ql_log_pcap_e;
/*========================================================================
 *	function Definition
 *========================================================================*/
/*
* to config the log port for AP, it will take effect after reboot.
*/
ql_errcode_log_e ql_log_set_port(ql_log_port_e dst_port);

/*
* to enable or disable quectel log
*/
void ql_quec_trace_enable(bool allow_Put);

/*
* Initialization Saves logs in the file system.
* Logs before fstrace init can be output via USB or debug port, which can be configured in prvTraceInit of app_start.
* Debug port is used by default.
*/
void ql_fs_trace_init(void);

ql_errcode_log_e ql_log_pcap_get_from_nv();
ql_errcode_log_e ql_log_pcap_get(ql_log_pcap_e* flag);
ql_errcode_log_e ql_log_pcap_set(ql_log_pcap_e flag);

#ifdef __cplusplus
} /*"C" */
#endif

#endif /* QL_LOG_H */

