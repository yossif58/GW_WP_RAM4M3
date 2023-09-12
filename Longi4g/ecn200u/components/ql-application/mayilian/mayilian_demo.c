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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mayilian_demo.h"

#include "bot_system_utils.h"
#include "bot_system.h"
#include "bot_hal_at.h"
#include "bot_hal_uart.h"
#include "bot_hal_rtc.h"
#include "bot_hal_gnss.h"
#include "bot_hal_flash.h"

#define BOT_QUEUE_TEST          1
#define BOT_SEM_TEST            1
#define BOT_MUTEX_TEST          1
#define BOT_VIRT_AT_TEST        1
#define BOT_UART_TEST           1
#define BOT_RTC_TEST            1
#define BOT_GNSS_TEST           1
#define BOT_FLASH_TEST          1


void *bot_handle = NULL;
void *bot_test_handle = NULL;

#if BOT_QUEUE_TEST
void *bot_mq = NULL;
typedef struct
{
    char type;
    char data[8];
}bot_queue;
#endif

#if BOT_SEM_TEST
void *bot_sem = NULL;
#endif

#if BOT_MUTEX_TEST
void *bot_mutex = NULL;
#endif

#if BOT_VIRT_AT_TEST
static void bot_hal_virt_at_cb(unsigned char *data, unsigned int len)
{
    bot_printf("bot at recv len=%d,data=%s", len, data);
}
#endif

#if BOT_UART_TEST
static void bot_hal_uart_rx_cb(int port, void* data, unsigned int len, void* arg)
{
    char *bot_data = data;
    int arg_data = (int)arg;
    bot_printf("bot uart recv port=%d,data=%s,len=%d,arg=%d", port, bot_data, len, arg_data);
}
#endif

#if BOT_GNSS_TEST
static void bot_hal_gnss_data_recv_cb(char *data, unsigned int len)
{
    bot_printf("bot gnss data len=%d,data=%s", len, data);
}
#endif

void ql_mayilian_demo_task_pthread(void *ctx)
{
    bot_printf("bot task start");

#if BOT_QUEUE_TEST
    if (bot_mq)
    {
        bot_queue mq = {0};
        int ret = bot_queue_recv(bot_mq, &mq, sizeof(bot_queue), 5000);
        if (ret < 0)
        {
            bot_printf("bot wait queue timeout");
        }
        else 
        {
            bot_printf("bot mq.type=%d,mq.data[0]=%x", mq.type, mq.data[0]);
        }
        bot_queue_delete(bot_mq);
        bot_mq = NULL;
    }
#endif   

#if BOT_SEM_TEST
    if (bot_sem)
    {
        int ret = bot_sem_wait(bot_sem, 5000);
        if (ret < 0)
        {
            bot_printf("bot wait sem timeout");
        }
        else 
        {
            bot_printf("bot get sem");
        }
        bot_queue_delete(bot_sem);
        bot_sem = NULL;
    }
#endif

#if BOT_MUTEX_TEST
    if (bot_mutex)
    {
        bot_mutex_lock(bot_mutex);
        bot_printf("bot mutex run first");
        bot_msleep(5000);
        bot_printf("bot mutex run first ...");
        bot_mutex_unlock(bot_mutex);
    }
#endif

#if BOT_VIRT_AT_TEST
    int virt_ret = bot_hal_at_cb_register(bot_hal_virt_at_cb);
    if (virt_ret < 0)
    {
        bot_printf("bot at cb err");
    }
    else 
    {
        char *cmd0 = "ATI\r\n";
      
        virt_ret = bot_hal_at_send((unsigned char *)cmd0, strlen((char *)cmd0));
        if (virt_ret < 0)
        {
            bot_printf("bot at send err");
        }
    }
#endif

#if BOT_UART_TEST
    int uart_ret  = 0;
    bot_uart_dev_t uart = {0};
    uart.config.baud_rate = 115200;
    uart.config.data_width = DATA_WIDTH_8BIT;
    uart.config.parity = NO_PARITY;
    uart.config.stop_bits = STOP_BITS_1;
    uart.config.flow_control = FLOW_CONTROL_DISABLED;
    uart.config.mode = MODE_TX_RX;
    uart.port = 0;
    uart_ret = bot_hal_uart_init(&uart);
    if (uart_ret < 0)
    {
        bot_printf("bot uart init err");
    }
    else 
    {
        uart_ret = bot_hal_uart_send(&uart, "0123456789", 10, 50);
        if (uart_ret < 0)
        {
            bot_printf("bot uart send err");
        }

        char uart_recv[50+1] = {0};
        unsigned int real_len = 0;
        uart_ret = bot_hal_uart_recv(&uart, uart_recv, 50, &real_len, 5000);
        if (uart_ret < 0)
        {
            bot_printf("bot uart reav err");
        }
        bot_printf("bot uart_recv=%s,real_len=%d", uart_recv, real_len);
        
        
        uart_ret = bot_hal_uart_recv_cb_register(&uart, bot_hal_uart_rx_cb, (void *)200);
        if (uart_ret < 0)
        {
            bot_printf("bot uart reg err");
        }

        //等待上位机发送数据
        bot_msleep(10000);

        uart_ret = bot_hal_uart_deinit(&uart);
        if (uart_ret < 0)
        {
            bot_printf("bot uart deinit err");
        }
    }
#endif

#if BOT_RTC_TEST
    bot_rtc_dev_t rtc = {0};

    int rtc_ret = bot_hal_rtc_init(&rtc);
    if (rtc_ret < 0)
    {
        bot_printf("bot rtc init err");
    }
    else 
    {
        bot_rtc_time_t get_time = {0};
        rtc_ret = bot_hal_rtc_get_time(&rtc, &get_time);
        if (rtc_ret < 0)
        {
            bot_printf("bot rtc get time err");
        }
        else 
        {
            bot_printf("bot rtc year=%d,mon=%d,day=%d,hour=%d,min=%d,sec=%d,wday=%d", get_time.year, get_time.month, get_time.date, get_time.hour, get_time.min, get_time.sec, get_time.weekday);
        }

        bot_rtc_time_t set_time = {0};
        set_time.year = 52;
        set_time.month = 3;
        set_time.date = 6;
        set_time.hour = 8;
        set_time.min = 18;
        set_time.sec = 40;
        set_time.weekday = 7;
        rtc_ret = bot_hal_rtc_set_time(&rtc, &set_time);
        if (rtc_ret < 0)
        {
            bot_printf("bot rtc set time err");
        }
        else 
        {
            memset(&get_time, 0x00, sizeof(get_time));
            rtc_ret = bot_hal_rtc_get_time(&rtc, &get_time);
            if (rtc_ret < 0)
            {
                bot_printf("bot rtc get time err");
            }
            else 
            {
                bot_printf("bot rtc year=%d,mon=%d,day=%d,hour=%d,min=%d,sec=%d,wday=%d", get_time.year, get_time.month, get_time.date, get_time.hour, get_time.min, get_time.sec, get_time.weekday);
            }
        }

        int rtc_ret = bot_hal_rtc_deinit(&rtc);
        if (rtc_ret < 0)
        {
            bot_printf("bot rtc deinit err");
        }
    }
#endif

#if BOT_GNSS_TEST
    int gnss_ret = bot_hal_gnss_init();
    if (gnss_ret < 0)
    {
        bot_printf("bot gnss init err");
    }
    else 
    {
        gnss_ret = bot_hal_gnss_cb_register(bot_hal_gnss_data_recv_cb);
        if (gnss_ret < 0)
        {
            bot_printf("bot gnss register err");
        }

        //等待回调里接收GNSS数据
        bot_msleep(60*1000);
        int gnss_ret = bot_hal_gnss_deinit();
        if (gnss_ret < 0)
        {
            bot_printf("bot gnss deinit err");
        }
    }
#endif

#if BOT_FLASH_TEST

#define BOT_FLASH_ERASE_SIZE        4096
#define BOT_FLASH_TEST_STR          "20220323_1234567890ABCDEF"
    bot_hal_logic_partition_t partition = {0};
    int flash_ret = bot_hal_flash_info_get(BOT_PARTITION_ID_KV, &partition);
    if (flash_ret < 0)
    {
        bot_printf("bot flash info get err");
    }
    else 
    {
        bot_printf("bot partition desc=%s,start_add=%x,length=%d,opt=%d", partition.partition_description, partition.partition_start_addr, partition.partition_length, partition.partition_options);

        unsigned int offset = 0;
        flash_ret = bot_hal_flash_erase(BOT_PARTITION_ID_KV, offset, BOT_FLASH_ERASE_SIZE);
        if (flash_ret < 0)
        {
            bot_printf("bot flash erase err");
        }

        flash_ret = bot_hal_flash_write(BOT_PARTITION_ID_KV, &offset, BOT_FLASH_TEST_STR, strlen(BOT_FLASH_TEST_STR));
        if (flash_ret < 0)
        {
            bot_printf("bot flash write err");
        }

        bot_printf("bot flash write offset=%d", offset);

        char read_buff[128] = {0};
        offset = 0;

        flash_ret = bot_hal_flash_read(BOT_PARTITION_ID_KV, &offset, read_buff, strlen(BOT_FLASH_TEST_STR));
        if (flash_ret < 0)
        {
            bot_printf("bot flash read err");
        }

        bot_printf("bot flash read offset=%d,data=%s", offset, read_buff);
    }
#endif
    bot_printf("bot test end");
    bot_task_delete(bot_handle);
}

void ql_mayilian_demo_test_task_pthread(void *ctx)
{
    bot_printf("bot test task start");
    bot_msleep(1000);

#if BOT_QUEUE_TEST    
    if (bot_mq)
    {
        bot_queue mq = {0};
        mq.type = 1;
        mq.data[0] = 0x55;
        int ret = bot_queue_send(bot_mq, &mq, sizeof(bot_queue), 5000);
        if (ret < 0)
        {
            bot_printf("bot send queue timeout");
        }
    }
#endif

#if BOT_SEM_TEST
    if (bot_sem)
    {
        bot_sem_post(bot_sem);
    }
#endif

#if BOT_MUTEX_TEST
    if (bot_mutex)
    {
        bot_mutex_lock(bot_mutex);
        bot_printf("bot mutex run second");
        bot_mutex_unlock(bot_mutex);
        bot_mutex_delete(bot_mutex);
        bot_mutex = NULL;
    }
#endif
    bot_task_delete(bot_test_handle);
}

extern void bot_app_entry(void);
void ql_mayilian_demo_init(void)
{	
#if 1
    int ret = 0;
    char test_data[64] = {0};

    bot_snprintf(test_data, sizeof(test_data), "test %d", 100);
    bot_printf("bot %s", test_data);

    char *test_mal = bot_os_alloc(128);
    if (test_mal)
    {
        memset(test_mal, 0x00, 128);
        bot_printf("bot test_mal=%p", test_mal);
        test_mal = bot_os_realloc(test_mal, 160);
        bot_printf("bot test_mal=%p", test_mal);
        bot_os_free(test_mal);
        test_mal = NULL;
    }

    struct timeval start_tv = {0};
    struct timeval end_tv = {0};
    bot_gettimeofday(&start_tv, NULL);
    bot_printf("bot start_tv.tv_sec =%lld,start_tv.tv_usec=%d", start_tv.tv_sec, start_tv.tv_usec);
    unsigned long long cur_time = bot_uptime();
    bot_printf("bot cur_time =%lld", cur_time);
    bot_msleep(10);
    unsigned long long next_time = bot_uptime();
    bot_printf("bot next_time =%lld", next_time);
    bot_printf("bot time gap =%lld", next_time - cur_time);
    bot_gettimeofday(&end_tv, NULL);
    bot_printf("bot end_tv.tv_sec =%lld,end_tv.tv_usec=%d", end_tv.tv_sec, end_tv.tv_usec);
    bot_printf("bot run time gap =%lld", ((end_tv.tv_sec*1000000+end_tv.tv_usec)-(start_tv.tv_sec*1000000+start_tv.tv_usec)));

    //unsigned int *addr = (unsigned int *)0x60250000;
    //*addr = 0;
    int default_priority = bot_task_default_priority_get();
    bot_printf("bot default_priority=%d", default_priority);

    bot_platform_init();
    bot_mem_para_t mem = {0};
    bot_mem_info_get(&mem);
    bot_printf("bot mem.start=%p", mem.start);
    bot_printf("bot mem.len=%p", mem.len);

    bot_task_param_t task_param = 
    {
        .priority = default_priority,
        .stack_addr = NULL,
        .stack_size = 4*1024,
        .name = "bot_task",
    };
    ret = bot_task_create(&bot_handle, ql_mayilian_demo_task_pthread, NULL, &task_param, NULL);
    if (ret < 0)
    {
        bot_printf("bot task create err");
    }
    bot_printf("bot bot_handle=%p", bot_handle);

    bot_task_param_t task_test_param = 
    {
        .priority = default_priority,
        .stack_addr = NULL,
        .stack_size = 4*1024,
        .name = "bot_test_task",
    };
    ret = bot_task_create(&bot_test_handle, ql_mayilian_demo_test_task_pthread, NULL, &task_test_param, NULL);
    if (ret < 0)
    {
        bot_printf("bot task create err");
    }
    bot_printf("bot bot_handle=%p", bot_test_handle);
    
#if BOT_QUEUE_TEST 
    bot_mq = bot_queue_create(4, sizeof(bot_queue));
    bot_printf("bot bot_mq=%p", bot_mq);
#endif

#if BOT_SEM_TEST
    bot_sem = bot_sem_create();
    bot_printf("bot bot_sem=%p", bot_sem);
#endif

#if BOT_MUTEX_TEST
    bot_mutex = bot_mutex_create();
    bot_printf("bot bot_mutex=%p", bot_mutex);
#endif

#else
    bot_app_entry();
#endif
}

