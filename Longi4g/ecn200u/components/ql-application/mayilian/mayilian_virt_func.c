/**  @file
  quec_app_virt_func.c

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
27/04/2022             Init version
=================================================================*/
#include "ql_gnss.h"

#ifdef CONFIG_QUEC_PROJECT_FEATURE_MAYILIAN
#ifndef CONFIG_QUEC_PROJECT_FEATURE_GNSS
__attribute__((weak)) ql_GnssState ql_gnss_state_info_get(void)
{
    return GNSS_CLOSE;
}

__attribute__((weak)) ql_errcode_gnss ql_gnss_nmea_get(ql_uart_port_number_e uart_port,unsigned char *pbuff, uint32 len)
{
    return QL_GNSS_NOT_SUPPORT_ERR;
}

__attribute__((weak)) ql_errcode_gnss ql_gnss_switch(ql_GnssSW gnss_sw)
{
    return QL_GNSS_NOT_SUPPORT_ERR;
}

__attribute__((weak)) ql_errcode_gnss ql_gnss_callback_register(ql_gnss_callback gnss_cb)
{
    return QL_GNSS_NOT_SUPPORT_ERR;
}
#endif
#endif




