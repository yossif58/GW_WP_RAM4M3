
#ifndef __BOT_HAL_AT_H__
#define __BOT_HAL_AT_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup hal_at
 *  uart hal API.
 *
 *  @{
 */

#include <stdint.h>
#include "bot_system.h"

/* Virtual AT command data processing function pointer */
typedef void (*bot_at_rx_cb)(unsigned char* data, unsigned int len);

/**
 * Send virtual at cmd to kernel
 *
 * @param[in]  cmd  at command string
 * @param[in]  len  length of at command string
 *
 * @return  0 : on success, otherwise: fail.
 */
int bot_hal_at_send(unsigned char *cmd, unsigned int len);

/**
 * Register virtual at command to return data processing function
 *
 * @param[in]  cb  pointer to data processing function
 *
 * @return  0 : on success, otherwise: fail.
 */
int bot_hal_at_cb_register(bot_at_rx_cb cb);

/**
 * virtual at command rx data callback
 *
 * @param[in]  data  received data
 * @param[in]  len   length of received data
 *
 * @return  0 : on success, otherwise: fail.
 */
int bot_hal_at_callback(unsigned char *data, unsigned int len);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __BOT_HAL_AT_H__ */

