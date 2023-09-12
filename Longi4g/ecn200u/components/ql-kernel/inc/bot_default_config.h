/*
 *
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
*/
/**
 * @file bot_default_config.h
 *
 * @brief bot config header file.
 *
 * @details This header file defines the config of bot.
 *
 *
 */
#ifndef __BOT_CONFIG_H__
#define __BOT_CONFIG_H__

#include "bot_platform_user_config.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* LOG config */
#ifndef  BOT_COMP_LOG
#define  BOT_COMP_LOG   0
#endif

/* KV config */
#ifndef  BOT_COMP_KV
#define  BOT_COMP_KV    0
#endif

/* AT config */
#ifndef  BOT_COMP_AT
#define  BOT_COMP_AT    0
#endif

/* GNSS config */
#ifndef  BOT_COMP_GNSS
#define  BOT_COMP_GNSS  0
#endif

/* MEM config */
#ifndef  BOT_COMP_MEM
#define  BOT_COMP_MEM   0
#endif

#ifdef __cplusplus
}
#endif

#endif /* __BOT_CONFIG_H__ */

