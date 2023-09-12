/*
 *
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
*/
/**
 * @file bot_network.h
 *
 * @brief antchain network header file.
 *
 * @details This header file defines the functional interface of network
 *
 */

#ifndef __BOT_NETWORK_H__
#define __BOT_NETWORK_H__

#include <stdint.h>
#include <stddef.h>
#if defined(__cplusplus)
extern "C" {
#endif

typedef struct{
	char netctrl_sta;
    bool sta_init;
}bot_netctrl_parm_t;

/********** network **********/
/**
 * @brief netowrk init. It must be first called before calling other network function.
 *
 * @param[in]  none
 *
 */
int bot_network_init(void);

/********** network **********/
/**
 * @brief get network rssi
 *
 * @param[out]  rssi  received signal strength indicator
 * @return 0: success, others: fail
 */
int bot_network_rssi_get( signed short *rssi);
/**
 * @brief get network ccid
 *
 * @param[out]  iccid integrate circuit card identity 
 * @return 0: success, others: fail
 */
int bot_network_ccid_get(char *iccid);
/**
 * @brief get network imei
 *
 * @param[out]  imei international mobile equipment identity
 * @return 0: success, others: fail
 */
int bot_network_imei_get(char *imei);

/**
 * @brief get lbs information
 *
 * @param[out]  mcc mobile country code
 * @param[out]  mnc mobile network code
 * @param[out]  cid cell id 
 * @param[out]  lac location area code 
 * @return 0: success others: fail
 */
int bot_network_lbs_get(unsigned short *mcc, unsigned short *mnc, unsigned int *cid, unsigned int *lac);

/**
 * @brief get rat information
 *
 * @param[out]  rat radio access technology
 * @return 0: success others: fail
 */
int bot_network_rat_get(unsigned char *rat);

/**
 * @brief Check network is available
 *
 * @return 1: available 0: not available
 */
int bot_network_is_ready(void);


#if defined(__cplusplus)
}
#endif
#endif /* __BOT_NETWORK_H__ */

