
#ifndef __BOT_HAL_GNSS_H__
#define __BOT_HAL_GNSS_H__
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * gnss data receiving callback function type
 */
typedef void (*bot_gnss_data_recv_cb)(char* data, unsigned int len);

/**
 * gnss hal init
 *
 * @return  0  On success, otherwise is error
 */
int bot_hal_gnss_init(void);

/**
 * gnss hal deinit
 *
 * @return  0  On success, otherwise is error
 */
int bot_hal_gnss_deinit(void);


/**
 * Register gnss data receive callback
 *
 * @param[in]  callback     callback function for gnss data receive
 *
 * @return  0  On success, otherwise is error
 */
int bot_hal_gnss_cb_register(bot_gnss_data_recv_cb callback);

#ifdef __cplusplus
}
#endif

#endif /* __BOT_HAL_GNSS_H__ */

