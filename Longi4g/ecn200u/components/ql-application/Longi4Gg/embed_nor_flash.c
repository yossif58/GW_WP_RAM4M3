/**  @file
  embed_nor_flash_demo.c

  @brief
  This file is demo of embed nor flash read or wrtie.

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

=================================================================*/

/*===========================================================================
 * include files
 ===========================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_embed_nor_flash.h"
#include "longi4g.h"
#define LTEQL_FLASH_H
#include "longi_nor_flash.h"
#include "crc.h"
/*===========================================================================
 * Macro Definition
 ===========================================================================*/

#define QL_APP_EMBED_NOR_FLASH_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_EMBED_NOR_FLASH_LOG(msg, ...)             QL_LOG(QL_APP_EMBED_NOR_FLASH_LOG_LEVEL, "QL_APP_EMBED_NOR_FLASH", msg, ##__VA_ARGS__)

#define SECTOR_SIZE  4096
//#define FLASH_ADDR   0x60368000 
#define FLASH_ADDR     0x60558000
//y.f. left 4k flash for configuration from  0x60368000 till 0x60370000 this is done by changing partinfo_8910_8m_opencpu_novolte file :
// partinfo_8910_8m_opencpu_novolte    "CONFIG_APPIMG_FLASH_SIZE": "0x11f000", default was 0x120000
// y.f. legacy routines taken from st repeater 
extern void Send2DebugUart(uint8_t *DataIn, uint16_t Len);
extern RepeaterConfigStruct RepeaterConfig;
extern void printgsmconfig(RepeaterConfigStruct  *pConfig);
void     GetConfigFromFlash(void)
{
	  ql_embed_nor_flash_read(FLASH_ADDR,(unsigned char*)&RepeaterConfig,sizeof(RepeaterConfigStruct));
}
static RepeaterConfigStruct flashconfig;
bool FLashCheck(void)
{
  uint16_t crc;
  ql_errcode_e ret=QL_SUCCESS;
  ret= ql_embed_nor_flash_read(FLASH_ADDR,(unsigned char*)&flashconfig,sizeof(RepeaterConfigStruct));
  if (ret != QL_SUCCESS) return false;
  crc = CrcBlockCalc((unsigned char*)&flashconfig, sizeof(RepeaterConfigStruct)-2);
  if (flashconfig.Crc != crc) 
  {
  
	printgsmconfig(&flashconfig);
	return false;
  }
 else   
 {
	memcpy((unsigned char *)& RepeaterConfig , (unsigned char*)&flashconfig,sizeof(RepeaterConfigStruct));
    return true;    
 }
}
ql_errcode_e WriteInternalFlashConfig(void)
{
 
 
  uint16_t ConfigSize;
  ql_errcode_e ret=QL_SUCCESS;
 // y.f. 3/3/2021 add new crc 
  RepeaterConfig.Crc =  CrcBlockCalc((unsigned char*)&RepeaterConfig, sizeof(RepeaterConfigStruct)-2);
  ConfigSize = sizeof(RepeaterConfigStruct);
  while (ConfigSize % 4)
    ConfigSize++; // be sure that we have modulo 4 size 

  //erase FLASH_ADDR sector
    ret=ql_embed_nor_flash_erase(FLASH_ADDR,SECTOR_SIZE);

    if(ret!=QL_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("embed nor flash erase faild,erase addr:0x%X",FLASH_ADDR);
		return ret;
    }

    ret=ql_embed_nor_flash_write(FLASH_ADDR,(unsigned char*)&RepeaterConfig,sizeof(RepeaterConfigStruct));
	
    if(ret!=QL_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("write embed nor flash faild");
		return ret;
    }
	
	memset((unsigned char*)&flashconfig,0x55,sizeof(RepeaterConfigStruct));
	ret=ql_embed_nor_flash_read(FLASH_ADDR,(unsigned char*)&flashconfig,sizeof(RepeaterConfigStruct));
	 if(ret!=QL_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("read embed nor flash faild");
		return ret;
    }
	
	char str [100];
	if (memcmp((unsigned char*)&flashconfig,(unsigned char*)&RepeaterConfig,sizeof(RepeaterConfigStruct))==0)
	{
	  sprintf (str, "readback passed \r\n");
	  Send2DebugUart((uint8_t *)str, strlen(str));
	  printgsmconfig(&flashconfig);

      return QL_SUCCESS;
	}
	
     sprintf (str, "readback not passed \r\n");
	 Send2DebugUart((uint8_t *)str, strlen(str));	

	 return QL_GENERAL_ERROR;
  
}
#if 0
ql_task_t embed_nor_flash_task = NULL;

 void static embed_nor_flash_demo_thread(void *param)
{
    ql_errcode_e ret=QL_SUCCESS;

    char flash_buff[128]={0};

    char *test_write_str="123456789abcdefghijklmnqpqrst";

    QL_EMBED_NOR_FLASH_LOG("==========embed flash demo start============");

    ql_rtos_task_sleep_ms(10);

    //erase FLASH_ADDR sector
    ret=ql_embed_nor_flash_erase(FLASH_ADDR,SECTOR_SIZE);

    if(ret!=QL_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("embed nor flash erase faild,erase addr:0x%X",FLASH_ADDR);
    }

    ret=ql_embed_nor_flash_write(FLASH_ADDR,(void *)test_write_str,strlen(test_write_str));
	
    if(ret!=QL_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("write embed nor flash faild");
    }
	
    ql_rtos_task_sleep_ms(10);

    ret=ql_embed_nor_flash_read(FLASH_ADDR,(void *)flash_buff,strlen(test_write_str));
    
    if(ret!=QL_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("read embed nor flash faild");
    }
	
    QL_EMBED_NOR_FLASH_LOG("read addr 0x%X,content:%s",FLASH_ADDR,flash_buff);
    
    ql_rtos_task_sleep_ms(10);

    QL_EMBED_NOR_FLASH_LOG("==========embed flash demo finished============");
    
  	ql_rtos_task_delete(embed_nor_flash_task);	
  
}

void embed_nor_flash_app_init(void)
{
    int err = QL_OSI_SUCCESS;
        
    err = ql_rtos_task_create(&embed_nor_flash_task, 8*1024,23,"embed_flash_DEMO", embed_nor_flash_demo_thread, NULL,1);
	
    if (err != QL_OSI_SUCCESS)
    {
        QL_EMBED_NOR_FLASH_LOG("embed nor flash demo task created failed");
    }
}
#endif




