/*
 * Copyright (C) 2021 Ant Group Holding Limited
 *
 */

#include "bot_default_config.h"
#include "bot_system.h"
#include "bot_system_utils.h"
#include "bot_maas.h"

/* 设备编号最大长度(参考《OpenCPU开发》文档“API接口”的“4.设备注册”部分) */
#define BOT_TEST_DEVICE_ID_MAX_SIZE        63
/* 业务数据最大长度(参考《OpenCPU开发》文档“API接口”的“5.设备数据”部分) */
#define BOT_TEST_USER_DATA_STRING_MAX_SIZE 1023
/* 模拟设备编号(仅供测试使用) */
#define BOT_TEST_DEVICE_ID                 "TEST12345678"
/* 模拟无效设备编号(用户根据实际情况修改) */
#define BOT_TEST_INVALID_DEVICE_ID         "0000"

static char g_maas_user_data[BOT_TEST_USER_DATA_STRING_MAX_SIZE + 1] = {0};
static char g_maas_device_id[BOT_TEST_DEVICE_ID_MAX_SIZE + 1]        = {0};

/**
 * @brief 设备编号校验（仅供参考，用户根据实际情况具体实现）
 * 
 * @param[in]  id：存储设备编号的内存地址
 * 
 * @return 校验成功，返回0；校验失败，返回-1；
 */
int user_device_id_check(const char *id)
{
    int len;
    /* 长度校验 */
    len = strlen(id);
    if (BOT_TEST_DEVICE_ID_MAX_SIZE < len) {
        return -1;
    }
    /* 有效性校验 */
    if (memcmp(id, BOT_TEST_INVALID_DEVICE_ID, len) == 0) {
        return -1;
    }
    return 0;
}

/**
 * @brief 获取设备的唯一编号（仅供参考，用户根据实际情况具体实现）
 * 
 * @param[out] id:   存储设备编号的内存地址
 * @param[in]  size：存储设备编号的最大长度
 * 
 * @return 获取成功，返回0；获取失败，返回-1。
 */
int user_device_id_get(char *id, const int size)
{
    char *tmp = NULL;
    int ret;

    if (id == NULL) {
        bot_printf("Input id is NULL\n\r");
        return -1;
    }
    /* TODO：用户应获取实际的设备编号 */
    tmp = BOT_TEST_DEVICE_ID;
    if (tmp == NULL) {
        bot_printf("Device id is NULL\n\r");
        return -1;
    }
    /* 设备编号校验 */
    ret = user_device_id_check(tmp);
    if (ret != 0) {
        bot_printf("Device id is invalid\n\r");
        return -1;
    } else {
        bot_snprintf(id, size, "%s", tmp);
    }
    return 0;
}

/**
 * @brief 获取用户业务数据（仅供参考，用户根据实际情况具体实现）
 * 
 * @param[out] data: 存储业务数据的内存地址
 * @param[in]  size：存储业务数据的最大长度
 * 
 * @return 成功，返回业务数据长度；失败，返回-1。
 */
int user_data_get(char *data, const int size)
{
    int len;

    if (data == NULL) {
        bot_printf("Input data is NULL\n\r");
        return -1;
    }
    len = 0;
    /* TODO：用户根据实际情况实现 */
    len += bot_snprintf(data + len, size - len, "{");
    len += bot_snprintf(data + len, size - len, "\"assetId\":\"%s\"", g_maas_device_id);
    len += bot_snprintf(data + len, size - len, ",\"soh\":%d", 90);
    len += bot_snprintf(data + len, size - len, ",\"soc\":%d", 90);
    len += bot_snprintf(data + len, size - len, ",\"voltage\":%d", 3000);
    len += bot_snprintf(data + len, size - len, ",\"temperature\":%d", 3000);
    len += bot_snprintf(data + len, size - len, ",\"fullChargeCycles\":%d", 12);
    len += bot_snprintf(data + len, size - len, ",\"batteryStatus\":%d", 1);
    len += bot_snprintf(data + len, size - len, ",\"source\":%d", 0);
    len += bot_snprintf(data + len, size - len, ",\"coordinateSystem\":%d", 0);
    len += bot_snprintf(data + len, size - len, ",\"longitude\":%0.6lf", 120.03777778);
    len += bot_snprintf(data + len, size - len, ",\"latitude\":%0.6lf", 60.01);
    len += bot_snprintf(data + len, size - len, ",\"altitude\":%0.6lf", 30.04);
    len += bot_snprintf(data + len, size - len, "}");
    bot_printf("user_data len: %d, %s\n\r", len, data);
    return len;
}

/**
 * @brief 应用示例
 * 
 */
void bot_app_entry(void)
{
    int ret = 0;
    int count = 0;
    int connect_status;
    int device_status;
    int user_data_len;

    /* 1. 获取设备唯一编号。若获取失败，请勿进行后续步骤 */
    bot_printf("Start getting device id\n\r");
    ret = user_device_id_get(g_maas_device_id, BOT_TEST_DEVICE_ID_MAX_SIZE);
    if (ret != 0) {
        bot_printf("It was fail to get device id\n\r");
        return;
    }

    /* 2. MaaS初始化 */
    bot_printf("Start initializing MaaS\n\r");
    ret = bot_maas_init();
    if (ret != 0) {
        bot_printf("It was fail to initialize, ret -0x%x\n\r", -ret);
        return;
    }

#if !BOT_COMP_AT
    /* 3. 连接状态查询。每秒查询一次，连接成功或者100秒超时退出 */
    bot_printf("Start getting the connection status\n\r");
    count = 100;
    while(count--) {
        connect_status = bot_maas_connect_status_get();
        if (connect_status == 1) {
            break;
        }
        bot_msleep(1000);
    }

    if (connect_status != 1) {
        bot_printf("It was timeout to get the connection status\n\r");
        return;
    }

    /* 4. 注册状态查询 */
    bot_printf("Start getting device status\n\r");
    device_status = bot_maas_device_status_get(g_maas_device_id);
    if (device_status != 1) {
        /* 5. 启动设备注册 */
        bot_printf("Start registering device id\n\r");
        ret = bot_maas_device_register(g_maas_device_id, "3001-64.0V30AH", "ADV1.0");
        if (ret != 0) {
            bot_printf("It was fail to register, ret -0x%x\n\r", -ret);
            return;
        }
        /* 6. 注册状态查询。每秒查询一次, 注册成功或者180秒超时退出 */
        bot_printf("Get the device status once per second\n\r");
        count = 180;
        while(count--) {
            device_status = bot_maas_device_status_get(g_maas_device_id);
            if (device_status == 1) {
                break;
            }
            bot_msleep(1000);
        }

        if (device_status != 1) {
            bot_printf("It was timeout to register device id\n\r");
            return;
        }
    }

    while(1){
        /* 7. 设备数据发布。本测试用例上报周期60秒 */
        bot_printf("Start publishing MaaS data\n\r");
        user_data_len = user_data_get(g_maas_user_data, BOT_TEST_USER_DATA_STRING_MAX_SIZE);
        if (user_data_len < 1) {
            bot_printf("It was fail to get user data, ret %d\n\r", ret);
        } else {
            ret = bot_maas_data_publish((uint8_t*)g_maas_user_data, user_data_len);
            if (ret != 0) {
                bot_printf("It was fail to publish MaaS data, ret -0x%x\n\r", -ret);
            } else {
                bot_printf("Successfully published MaaS data \n\r");
            }
        }
        bot_msleep(60 * 1000);
    }
#endif
}
