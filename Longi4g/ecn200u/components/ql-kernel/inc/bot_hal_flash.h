/*
 *
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
*/
/**
 * @file bot_hal_flash.h
 *
 * @brief bot flash header file.
 *
 * @details This header file defines the functional interface of flash operation.
 *
 */

#ifndef __BOT_HAL_FLASH_H__
#define __BOT_HAL_FLASH_H__

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Flash partition id defines
 */
typedef enum {
    BOT_PARTITION_ID_KV, /* kv partition */
    BOT_PARTITION_ID_MAX
} bot_partition_id_e;

/*
 * Description of the flash partition
 */
typedef struct {
    const char       *partition_description;
    unsigned int      partition_start_addr;
    unsigned int      partition_length;
    unsigned int      partition_options;
} bot_hal_logic_partition_t;

/**
 * get flash area information
 *
 * @param[in]  id              The target flash logical partition which should be read
 * @param[out] partition      Point to the flash area information
 *
 *
 * @return  0 : On success, otherwise: fail.
 */
int bot_hal_flash_info_get(bot_partition_id_e id, bot_hal_logic_partition_t *partition);

/**
 * Read data from an area on a Flash to data buffer
 *
 * @param[in]      id          The target flash logical partition which should be read
 * @param[in/out]  offset      Point to the start address that the data is read, and
 *                             point to the last unread address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  buffer          Point to the data buffer that stores the data read from flash
 * @param[in]  buffer_len      The length of the buffer
 *
 * @return  0 : On success, otherwise: fail.
 */
int bot_hal_flash_read(bot_partition_id_e id, unsigned int *offset, void *buffer, unsigned int buffer_len);

/**
 * This function will initialize flash
 *
 * @return  0 : on success, otherwise will be failed
 */
int bot_hal_flash_init(void);

/**
 * Write data to an area on a flash logical partition without erase
 *
 * @param[in]      id          The target flash logical partition which should be read which should be written
 * @param[in/out]  offset      Point to the start address that the data is written to, and
 *                             point to the last unwritten address after this function is
 *                             returned, so you can call this function serval times without
 *                             update this start address.
 * @param[in]  buffer          point to the data buffer that will be written to flash
 * @param[in]  buffer_len      The length of the buffer
 *
 * @return  0: On success, otherwise: fail.
 */

int bot_hal_flash_write(bot_partition_id_e id, unsigned int *offset, const void *buffer, unsigned int buffer_len);

/**
 * Erase an area on a Flash logical partition
 *
 * @note  Erase on an address will erase all data on a sector that the
 *        address is belonged to, this function does not save data that
 *        beyond the address area but in the affected sector, the data
 *        will be lost.
 *
 * @param[in]  id            The target flash logical partition which should be erased
 * @param[in]  offset        Start address of the erased flash area
 * @param[in]  size          Size of the erased flash area
 *
 * @return  0:  On success, otherwise: fail.
 */

int bot_hal_flash_erase(bot_partition_id_e id, unsigned int offset, unsigned int size);



#ifdef __cplusplus
}
#endif

#endif /* __BOT_HAL_FLASH_H__ */

