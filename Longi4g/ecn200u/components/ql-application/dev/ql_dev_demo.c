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

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_api_dev.h"
#include "ql_dev_demo.h"
#include "ql_fs.h"

#define QL_DEV_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_DEV_LOG(msg, ...)			QL_LOG(QL_DEV_LOG_LEVEL, "ql_DEV", msg, ##__VA_ARGS__)
#define QL_DEV_LOG_PUSH(msg, ...)		QL_LOG_PUSH("ql_DEV", msg, ##__VA_ARGS__)

#define QL_DEV_TASK_STACK_SIZE      2048
#define QL_DEV_TASK_PRIO            APP_PRIORITY_NORMAL
#define QL_DEV_TASK_EVENT_CNT       5

#define QL_CUSTOM_API_FOR_HK		0

/**
通过at+qfaccustnvm指令可以在生产时将关键数据存储到factory分区，通过ql_fac_cust_nvm_fread函数可读取存储的数据，
此处为示例：
发送AT指令：at+qfaccustnvm="hw_version","sn2"            //可以存储10个字符串，此处只用了2个
ql_dev_get_hw_version读取at指令写入的hw_verison			//此处将ql_fac_cust_nvm_fread映射为ql_dev_get_hw_version
ql_dev_get_sn2读取at指令写入的sn2                          //此处将ql_fac_cust_nvm_fread映射为ql_dev_get_sn2
**/
#define ql_dev_get_hw_version(a,b,c)  	ql_fac_cust_nvm_fread(0,a,b,c)
#define ql_dev_get_sn2(a,b,c)	    	ql_fac_cust_nvm_fread(1,a,b,c)

/*
ql_get_dump_cnt/ql_clean_dump_cnt/ql_get_dump_file/ql_del_dump_file 
Customized API for HengKe
*/
#if QL_CUSTOM_API_FOR_HK
//get dump file
void ql_get_dump_file(char * buffer, int len)
{
	ql_dump_info_t dump_info = {0};
	dump_info.dump_buffer = buffer;
	dump_info.length = len;
	ql_dev_get_dump_info(&dump_info);
}

//delete dump file
void ql_del_dump_file()
{
	ql_dump_config_t dump_config = {0};
	dump_config.save_dump_info = true;
	dump_config.del_dump_file = true;
	dump_config.max_dump_cnt = 100;
	dump_config.clean_dump_cnt = false;
	ql_dev_config_dump(&dump_config);
}

//get dump cnt
int ql_get_dump_cnt()
{
	ql_dump_info_t dump_info = {0};
	ql_dev_get_dump_info(&dump_info);

	return dump_info.current_dump_cnt;
}

//clean dump cnt
void ql_clean_dump_cnt()
{
	ql_dump_config_t dump_config = {0};
	dump_config.save_dump_info = true;
	dump_config.del_dump_file = false;
	dump_config.max_dump_cnt = 100;
	dump_config.clean_dump_cnt = true;
	ql_dev_config_dump(&dump_config);
}
#endif

static void ql_dev_demo_thread(void *param)
{
	QlOSStatus err = QL_OSI_SUCCESS;
    QL_DEV_LOG("dev demo thread enter, param 0x%x", param);
	
	for (int n = 0; n < 10; n++)
    {
        QL_DEV_LOG("hello dev demo %d", n);
        ql_rtos_task_sleep_ms(500);
    }
	
	char devinfo[64] = {0};
	ql_dev_get_imei(devinfo, 64, 0);
	QL_DEV_LOG("IMEI: %s", devinfo);
    
	ql_dev_get_sn(devinfo, 64, 0);
	QL_DEV_LOG("SN: %s", devinfo);
    
	ql_dev_get_product_id(devinfo, 64);
	QL_DEV_LOG("Product: %s", devinfo);
    
	ql_dev_get_model(devinfo, 64);
	QL_DEV_LOG("Model: %s", devinfo);
    
	ql_dev_get_firmware_version(devinfo, 64);
	QL_DEV_LOG("FW: %s", devinfo);

	ql_dev_get_firmware_subversion(devinfo, 64);
	QL_DEV_LOG("subVer: %s", devinfo);
	
	uint8_t cfun = 0;
	ql_dev_get_modem_fun(&cfun, 0);
	QL_DEV_LOG("cfun: %d", cfun);
	ql_dev_set_modem_fun(QL_DEV_CFUN_FULL, 0, 0);

    //ql_log_set_port(QL_LOG_PORT_USB);//set the log port as USB AP port
    
	int32_t ql_temp = 0;
	ql_dev_get_temp_value(&ql_temp);
	QL_DEV_LOG("TEMP: %d", ql_temp);

#if QL_CUSTOM_API_FOR_HK
	//factory nv example
	char hw_version[64] = {0};
	ql_dev_get_hw_version(hw_version, 64, 1);
    QL_DEV_LOG("hw_version: %s", hw_version);

	//factory nv example
	char sn2[64] = {0};
	ql_dev_get_sn2(sn2, 64, 1);
    QL_DEV_LOG("sn2: %s", sn2);

	ql_rtos_task_sleep_s(5);
	//enable save dump info to file system
	ql_dump_config_t dump_config = {0};
	dump_config.save_dump_info = true;
	dump_config.del_dump_file = false;
	dump_config.max_dump_cnt = 100;
	dump_config.clean_dump_cnt = false;
	ql_dev_config_dump(&dump_config);

	//get dump file
	int buffer_len = 10000;
	char *dump_buffer = malloc(buffer_len);
	if(NULL == dump_buffer)
	{
		goto exit;
	}
	memset(dump_buffer, 0, buffer_len);
	ql_get_dump_file(dump_buffer, buffer_len);
	QL_DEV_LOG("dump buffer:%s, buffer_len=%d", dump_buffer, strlen(dump_buffer));
	free(dump_buffer);

	//get dump cnt
	int dump_cnt = ql_get_dump_cnt();
	QL_DEV_LOG("dump_cnt=%d", dump_cnt);

	//delete dump file
	ql_del_dump_file();

	//clean dump cnt 
	ql_clean_dump_cnt();

exit:
#endif
	err = ql_rtos_task_delete(NULL);
	if(err != QL_OSI_SUCCESS)
	{
		QL_DEV_LOG("task deleted failed");
	}
}

void ql_dev_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	ql_task_t dev_task = NULL;
	
	err = ql_rtos_task_create(&dev_task, QL_DEV_TASK_STACK_SIZE, QL_DEV_TASK_PRIO, "QDEVDEMO", ql_dev_demo_thread, NULL, QL_DEV_TASK_EVENT_CNT);
	if (err != QL_OSI_SUCCESS)
	{
		QL_DEV_LOG("dev demo task created failed");
	}
}

