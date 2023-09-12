/**  @file
  quec_virt_func_modem.c

  @brief
  This file is used to run virt function.

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
#include "at_response.h"
#include "at_engine.h"
#include "quec_cust_feature.h"
#include "quec_common.h"
#include "at_command.h"
#include "at_param.h"
#include "ql_api_osi.h"
#include "quec_at_engine.h"
#include "quec_log.h"
//#include "quec_rtos.h"
#include "ql_api_osi.h"
#include "ql_api_usbnet.h"

#define QUEC_VIRT_MODEM_FUNC_LOG(msg, ...)  custom_log("VIRT_MFUNC", msg, ##__VA_ARGS__)

//usbnet
#ifndef CONFIG_QUEC_PROJECT_FEATURE_USBNET
__attribute__((weak)) bool quec_usbnet_sendevent(uint32_t nMsg, uint32_t nParam1, uint8_t nParam2, uint8_t nParam3, uint16_t nParam4)
{
	return false;
}

__attribute__((weak)) ql_task_t *quec_get_usbnet_threadID(void)
{
	return NULL;
}

__attribute__((weak)) ql_usbnet_errcode_e ql_usbnet_get_type(ql_usbnet_type_e *usbnet_type)
{
	return QL_USBNET_EXECUTE_ERR;
}

__attribute__((weak)) ql_usbnet_errcode_e ql_usbnet_set_type(ql_usbnet_type_e usbnet_type)
{
	return QL_USBNET_EXECUTE_ERR;
}

__attribute__((weak)) void quec_exec_qnetdevctl_cmd(atCommand_t *cmd)
{
	return;
}
#endif/*CONFIG_QUEC_PROJECT_FEATURE_USBNET*/


//pbk
#ifndef CONFIG_QUEC_PROJECT_FEATURE_PBK
__attribute__((weak)) int quec_pbk_process_output_name(uint8_t *dest, uint8_t destlen, uint8_t cscs_charset, uint8_t *source, uint8_t sourcelen)
{
    return 0;
}
__attribute__((weak)) void quec_pbk_init(uint8_t nSim)
{
    return;
};
__attribute__((weak)) int quec_pbk_add_lastdialling_phone(uint8_t nSim, const char *phone_num, uint16_t len)
{
    return 0;
};
#endif/*CONFIG_QUEC_PROJECT_FEATURE_PBK*/

//volte
#ifndef CONFIG_QUEC_PROJECT_FEATURE_VOLTE
__attribute__((weak)) void quec_volte_init(void)
{
    return;
};
#endif/*CONFIG_QUEC_PROJECT_FEATURE_VOLTE*/

