#ifndef __BOT_VENDOR_ZERO_CONFIG_H__
#define __BOT_VENDOR_ZERO_CONFIG_H__

#define BOT_ZERO_CONFIG_BUF_MIN_LEN (128)

/**
 * @brief set config
 *
 * @param[in]  config   - config Product Information.
 *
 * @return 0 on success, otherwise will be failed
 */
int bot_zero_config_set(const char *config);

/**
 * @brief get config
 *
 * @param[in]   buf_len  - the length of the array
 * @param[out]  config   - get MaaS Information, Array length greater than BOT_ZERO_CONFIG_BUF_MIN_LEN.
 *
 * @return 0 on success, otherwise will be failed
 */
int bot_zero_config_get(char *config, int buf_len);

#endif