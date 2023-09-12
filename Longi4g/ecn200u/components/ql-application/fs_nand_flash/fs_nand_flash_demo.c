/**  
  @file
  fs_nand_flash_demo.c

  @brief
  quectel nand flash demo.

*/
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
24/05/2021        Neo         Init version
=================================================================*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_gpio.h"
#include "ql_api_osi.h"
#include "ql_api_spi.h"
#include "ql_api_spi_flash.h"
#include "ql_api_spi_nand_flash.h"
#include "ql_api_fs_nand_flash.h"
#include "ql_log.h"
#include "ql_fs.h"
#include "fs_nand_flash_demo.h"


#define QL_NAND_FLASH_LOG_LEVEL       	        QL_LOG_LEVEL_INFO
#define QL_NAND_FLASH_LOG(msg, ...)			    QL_LOG(QL_NAND_FLASH_LOG_LEVEL, "ql_FS_NAND_DEMO", msg, ##__VA_ARGS__)
#define QL_NAND_FLASH_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_FS_NAND_DEMO", msg, ##__VA_ARGS__)

//If the total sector number is smaller than 66130, format it as FM_FAT.Otherwise, format it as FM_FAT32
#define QL_FM_FAT                       0x01
#define QL_FM_FAT32                     0x02

ql_task_t fs_nand_flash_demo_task = NULL;


#define QL_CUR_SPI_PORT             QL_SPI_PORT1
#define QL_CUR_SPI_CS_PIN           QL_CUR_SPI1_CS_PIN
#define QL_CUR_SPI_CS_FUNC          QL_CUR_SPI1_CS_FUNC
#define QL_CUR_SPI_CLK_PIN          QL_CUR_SPI1_CLK_PIN
#define QL_CUR_SPI_CLK_FUNC         QL_CUR_SPI1_CLK_FUNC
#define QL_CUR_SPI_DO_PIN           QL_CUR_SPI1_DO_PIN
#define QL_CUR_SPI_DO_FUNC          QL_CUR_SPI1_DO_FUNC
#define QL_CUR_SPI_DI_PIN           QL_CUR_SPI1_DI_PIN
#define QL_CUR_SPI_DI_FUNC          QL_CUR_SPI1_DI_FUNC

int ql_nand_flash_pin_init(void)
{
    ql_errcode_gpio gpio_ret;
    if (QL_CUR_SPI_CS_PIN == QUEC_PIN_NONE || QL_CUR_SPI_CS_PIN == QUEC_PIN_NONE || \
        QL_CUR_SPI_DO_PIN == QUEC_PIN_NONE || QL_CUR_SPI_DI_PIN == QUEC_PIN_NONE)
    {
        QL_NAND_FLASH_LOG("pin err");
        return -1;
    }

    gpio_ret = ql_pin_set_func(QL_CUR_SPI_CS_PIN, QL_CUR_SPI_CS_FUNC);
    if (gpio_ret != QL_GPIO_SUCCESS)
    {
        QL_NAND_FLASH_LOG("set pin err");
        return -1;
    }
    gpio_ret = ql_pin_set_func(QL_CUR_SPI_CLK_PIN, QL_CUR_SPI_CLK_FUNC);
    if (gpio_ret != QL_GPIO_SUCCESS)
    {
        QL_NAND_FLASH_LOG("set pin err");
        return -1;
    }
    gpio_ret = ql_pin_set_func(QL_CUR_SPI_DO_PIN, QL_CUR_SPI_DO_FUNC);
    if (gpio_ret != QL_GPIO_SUCCESS)
    {
        QL_NAND_FLASH_LOG("set pin err");
        return -1;
    }
    gpio_ret = ql_pin_set_func(QL_CUR_SPI_DI_PIN, QL_CUR_SPI_DI_FUNC);
    if (gpio_ret != QL_GPIO_SUCCESS)
    {
        QL_NAND_FLASH_LOG("set pin err");
        return -1;
    }

    return 0;
}

static void ql_fs_nand_flash_demo_task_thread(void *ctx)
{
    ql_errcode_spi_nand_e ret;
    ql_errcode_nand_flash_e nand_ret;
    int64 err = 0;
    uint8_t opt = QL_FM_FAT32;

    //step1:pin init----customers can use the custom pin to initialize
    if (ql_nand_flash_pin_init())
    {
        QL_NAND_FLASH_LOG("pin err");
        goto QL_FTL_TASK_EXIT;
    }

    //step2:spi init
#if 0
    ret = ql_spi_nand_init(QL_CUR_SPI_PORT, QL_SPI_CLK_25MHZ);
#else
    ql_spi_nand_config_s spi_nand_config = {0};
    spi_nand_config.port = QL_CUR_SPI_PORT;
    spi_nand_config.spiclk = QL_SPI_CLK_25MHZ;
    spi_nand_config.input_sel = QL_SPI_DI_1;
    spi_nand_config.transmode = QL_SPI_DIRECT_POLLING;
    spi_nand_config.cs = QL_SPI_CS0;
    ret = ql_spi_nand_init_ext(spi_nand_config);
#endif
    if (ret != QL_SPI_FLASH_SUCCESS)
    {
        QL_NAND_FLASH_LOG("init err");
        goto QL_FTL_NAND_EXIT;
   }
    
    //step3:mkfs nand flash----the first time using flash or the file system is damaged needs to format the file system and mount file system 
    //mkfs operation can not be performed frequently. it will erase the whole block, which is very time-consuming
#if 0
    //opt = QL_FM_FAT;//if sector number < 66130,format it as FM_FAT.Otherwise, format it as FM_FAT32
    //1G nand = 60928 sector numbers,so should format as FM_FAT.
    //2G nand > 66130 sector numbers,so should format as FM_FAT32.
    nand_ret = ql_nand_flash_mkfs(opt);
    if(nand_ret != QL_NAND_FLASH_SUCCESS)
    {
        QL_NAND_FLASH_LOG("nand flash mkfs err");
        goto QL_FTL_NAND_EXIT;
    }
    ql_rtos_task_sleep_ms(100);
#else
    opt = opt;//only prevents compilation errors
    //step4:mount nand flash----mount file system
    nand_ret = ql_nand_flash_mount();
    if(nand_ret != QL_NAND_FLASH_SUCCESS)
    {
        QL_NAND_FLASH_LOG("nand flash mount err");
        goto QL_FTL_NAND_EXIT;
    }
#endif

    //step5:read id & read total size
    ql_nand_hw_info_t nand_info;
    ql_nand_get_hw_info(&nand_info);
    QL_NAND_FLASH_LOG("mid=%x, total_size = %d", nand_info.mid, nand_info.blknum * nand_info.blksize);

    //step6:using file system API to complete read, write ect. 
    int fd = 0;
	char buffer[200];
	char *str = TEST_STR_NAND;

	err = ql_mkdir(DIR_PATH_NAND, 0);
	if(err < 0)
	{
		if(err == QL_DIR_DIR_ALREADY_EXIST)
		{
			QL_NAND_FLASH_LOG("dir exist, not create");
		}
		else
		{
			QL_NAND_FLASH_LOG("make dir failed");
			goto QL_FTL_TASK_EXIT;
		}
	}

	fd = ql_fopen(FILE_PATH_NAND, "wb+");
	if(fd < 0)
	{
		QL_NAND_FLASH_LOG("open file failed");
		err = fd;
		goto QL_FTL_TASK_EXIT;
	}

	err = ql_fwrite(str, strlen(str) + 1, 1, fd);   //strlen not include '\0'
	if(err < 0)
	{
		QL_NAND_FLASH_LOG("write file failed");
		ql_fclose(fd);
		goto QL_FTL_TASK_EXIT;
	}

	err = ql_frewind(fd);
	if(err < 0)
	{
		QL_NAND_FLASH_LOG("rewind file failed");
		ql_fclose(fd);
		goto QL_FTL_TASK_EXIT;
	}

	err = ql_fread(buffer, ql_fsize(fd), 1, fd);
	if(err < 0)
	{
		QL_NAND_FLASH_LOG("read file failed");
		ql_fclose(fd);
		goto QL_FTL_TASK_EXIT;
	}
    QL_NAND_FLASH_LOG("file read result is %s", buffer);

	ql_fclose(fd);

    //step7:umount nand flash----umount file system
    nand_ret = ql_nand_flash_umount();
    if(nand_ret != QL_NAND_FLASH_SUCCESS)
    {
        QL_NAND_FLASH_LOG("nand flash umount err");
        goto QL_FTL_NAND_EXIT;
    }

    //err
QL_FTL_NAND_EXIT:
    ql_spi_release(QL_CUR_SPI_PORT);
QL_FTL_TASK_EXIT:
    if(err < 0)
	{
		QL_NAND_FLASH_LOG("errcode is %x", err);
	}
    QL_NAND_FLASH_LOG("ql_rtos_task_delete");
	err = ql_rtos_task_delete(NULL);
	if(err != QL_OSI_SUCCESS)
	{
		QL_NAND_FLASH_LOG("task deleted failed");
	}
}

QlOSStatus ql_fs_nand_flash_demo_init(void)
{	
	QlOSStatus err = QL_OSI_SUCCESS;

	err = ql_rtos_task_create(&fs_nand_flash_demo_task, FS_NAND_FLASH_DEMO_TASK_STACK_SIZE, FS_NAND_FLASH_DEMO_TASK_PRIO, "ql_nand_flash", ql_fs_nand_flash_demo_task_thread, NULL, FS_NAND_FLASH_DEMO_TASK_EVENT_CNT);
	if(err != QL_OSI_SUCCESS)
	{
		QL_NAND_FLASH_LOG("demo_task created failed");
        return err;
	}

    return err;
}

