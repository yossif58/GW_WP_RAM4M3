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
//#define FLASH_ADDR         0x60360000 
#define FLASH_ADDR         0x6036f000 
//#define FLASH_ADDR     0x60558000
//#define FLASH_ADDR     0x60368000  // left 100000 only for app flash code and use the other from 0x60350000 til 60370000 for config
//y.f. left 4k flash for configuration from  0x60368000 till 0x60370000 this is done by changing partinfo_8910_8m_opencpu_novolte file :
// partinfo_8910_8m_opencpu_novolte    "CONFIG_APPIMG_FLASH_SIZE": "0x11f000", default was 0x120000
// y.f. legacy routines taken from st repeater 
extern ql_queue_t DebugUartqueue;
extern RepeaterConfigStruct RepeaterConfig;
extern void printgsmconfig(RepeaterConfigStruct  *pConfig);
//void static embed_nor_flash_demo_thread(void *param);
void     GetConfigFromFlash(void)
{
	  ql_embed_nor_flash_read(FLASH_ADDR,(unsigned char*)&RepeaterConfig,sizeof(RepeaterConfigStruct));
}
static RepeaterConfigStruct flashconfig= {0};
bool FLashCheck(void)
{
  uint16_t crc;
  char str[100];
    ql_errcode_e ret=QL_SUCCESS;
	memset((unsigned char*)&flashconfig, 0x32, sizeof(RepeaterConfigStruct));

	
    ret = ql_embed_nor_flash_read(FLASH_ADDR,(unsigned char*)&flashconfig,sizeof(RepeaterConfigStruct));
 
   if (ret != QL_SUCCESS)
   {
	   	sprintf (str, "embed nor flash read faild\r\n");
        ql_rtos_queue_release(DebugUartqueue,strlen(str), (uint8_t*)str, QL_NO_WAIT);
 
   }
	else
	{
		sprintf (str, "embed nor flash read ok\r\n");
		ql_rtos_queue_release(DebugUartqueue,strlen(str), (uint8_t*)str, QL_NO_WAIT);
	}	
	crc = CrcBlockCalc((unsigned char*)&flashconfig, sizeof(RepeaterConfigStruct)-2);
  if (flashconfig.Crc != crc) 
  {
  

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
	  ql_rtos_queue_release(DebugUartqueue,strlen(str), (uint8_t*)str, QL_NO_WAIT);
	  printgsmconfig(&flashconfig);

      return QL_SUCCESS;
	}
	
     sprintf (str, "readback not passed \r\n");
     ql_rtos_queue_release(DebugUartqueue,strlen(str), (uint8_t*)str, QL_NO_WAIT);
	

	 return QL_GENERAL_ERROR;
  
}




