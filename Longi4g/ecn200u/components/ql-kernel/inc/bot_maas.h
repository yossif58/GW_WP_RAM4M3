/*
 * Copyright (C) 2019-2021 Alibaba Group Holding Limited
 */

#ifndef __BOT_MAAS_H__
#define __BOT_MAAS_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "bot_typedef.h"

/**
 * @brief init bot maas service
 *
 * @return 0 on success, otherwise will be failed
 */
int bot_maas_init(void);

/**
 * @brief maas version get
 *
 * @param[out] version - address to store the version string, at least 128 bytes length
 *
 * @return 0 on success, otherwise will be failed
 */
int bot_maas_version_get(char *version);

/**
 * @brief device register
 *
 * @param[in]  id   - ID of the device to register.
 * @param[in]  type - Type of the device to register.
 * @param[in]  data_ver - Version of asset data to register.
 *
 * @return 0 on success, otherwise will be failed
 */
int bot_maas_device_register(char *id, char *type, char *data_ver);

/**
 * @brief publish data to server
 *
 * @param[in]  data  - data to publish, it must be cjson format in string.
 * @param[in]  len   - the length of the data
 *
 * @return 0 on success, otherwise will be failed
 */
int bot_maas_data_publish(uint8_t *data, int len);

/**
 * @brief module connect status query.
 *
 * @return 1 on connect, otherwise on disconnect
 */
int bot_maas_connect_status_get(void);

/**
 * @brief device registration status query.
 *
 * @param[in]  id - ID of the device to register.
 *
 * @return 0 - Module not activated, device not registered
 * @return 1 - Module activated, device registered, 
 * @return 2 - Module activated, device not registered
 * @return other value means error.
 *
 */
int bot_maas_device_status_get(char *id);

#if defined(__cplusplus)
}
#endif

#endif /* __BOT_MAAS_H__ */


