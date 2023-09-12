/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "longi4g.h"
#include "ql_osi_def.h"
#include "ql_fs.h"
#include "longi_nor_flash.h"
#include "ql_virt_at_demo.h"
#include "crc.h"




/*===========================================================================
 * Variate
 ===========================================================================*/
#define LED_1  QUEC_PIN_DNAME_GPIO_1 
#define LED_2  QUEC_PIN_DNAME_GPIO_2 
extern void Gsm_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
extern bool Meter_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
extern void DLMS_ReadMeterId(void);
extern bool CheckDLMSState(void);
void printgsmconfig(RepeaterConfigStruct  *pConfig);
void SendData2Modem(uint8_t * DataIn, uint16_t Len);
void SendData2Meter(uint8_t * DataIn, uint16_t Len);
void MeteInit(void);
void MeterUartIn(uint8_t *bufintmp, uint16_t len);

ql_errcode_e EnterDefaultConfig(void);
static bool leds_state;
static void ToggleLeds();
static void ConfigLeds();

#define QL_Longi_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_Longi_LOG(msg, ...)			QL_LOG(QL_Longi_LOG_LEVEL, "ql_Longi4G", msg, ##__VA_ARGS__)
#define QL_Longi_LOG_PUSH(msg, ...)		QL_LOG_PUSH("ql_Longi4g", msg, ##__VA_ARGS__)
#define UART_MAX_MSG_CNT	10
#define UART_MAX_MSG_SIZE	1028
ql_queue_t	GSMQueue;
ql_queue_t DebugUartqueue;
ql_queue_t MeterUartqueue;
RepeaterConfigStruct RepeaterConfig;

uint16_t blinkFlag=10;
uint16_t GsmTimer;
uint16_t MeterTimer;
static void ql_Longi4G_thread(void *ctx)
{
//	GsmQueueStruct   GsmRxMsg ={0};
//	MeterQueueStruct MeterRxMsg = {0};
//	char str[6] = "Geek";
//	char ch = '1';
	while(1)
	{
		
		if (MeterTimer)
		{
			if  ((--MeterTimer) ==0)
			{
				Meter_Handler( 0, 0 , TIME_OUT_EVENT);
			}
		}
		if (GsmTimer)
		{
			if ((CheckDLMSState()==0)  &&(--GsmTimer) ==0)
			{
				Gsm_Handler( 0, 0 , TIME_OUT_EVENT);
			}
		}
		ToggleLeds();
		ql_rtos_task_sleep_ms(200);  // y.f. wake every 1500 ms
	}
    ql_rtos_task_delete(NULL);	
		
#if 0		
	    ql_rtos_queue_wait (GSMQueue , (uint8_t*)&GsmRxMsg, 1024, QL_NO_WAIT);
		if (GsmRxMsg.len) 
		{
			Gsm_Handler((uint8_t*)GsmRxMsg.rcvbuf ,GsmRxMsg.len , DATA_IN_EVENT);
			GsmRxMsg.len = 0;
		}
		if (GsmTimer)
		{
			if ((--GsmTimer) ==0)
			{
				Gsm_Handler( 0, 0 , TIME_OUT_EVENT);
 
			}
		}
	    ql_rtos_queue_wait (MeterUartqueue , (uint8_t*)&MeterRxMsg, 1024, QL_NO_WAIT);
		if (MeterRxMsg.len) 
		{
			Meter_Handler((uint8_t *)&MeterRxMsg.rcvbuf, MeterRxMsg.len);	
			MeterRxMsg.len = 0;
		}

		if (blinkFlag)
		{
			if (--blinkFlag==0)
			{
				blinkFlag =150;
				ToggleLeds();
			}
		}
		ch++;
		strncat(str, &ch, 1);
		ql_rtos_task_sleep_ms(20);  // y.f. wake every 1500 ms
	}
    ql_rtos_task_delete(NULL);
#endif	
}

void ql_Longi4G_app_init(void)
{
	int ret = 0;
//	blinkFlag =100;
	QlOSStatus err = QL_OSI_SUCCESS;
	ql_task_t ql_Longi4G_task = NULL;
	//unsigned char data[100];
	char str[100] = {0};
    QL_Longi_LOG("Longi4G enter");
	ConfigLeds();
	
	ret = ql_rtos_queue_create(&DebugUartqueue, UART_MAX_MSG_SIZE, UART_MAX_MSG_CNT);
	if (ret) 
	{
		QL_Longi_LOG("ql_rtos_queue_create failed!!!!");
	
	}
	ret = ql_rtos_queue_create(&MeterUartqueue, UART_MAX_MSG_SIZE, UART_MAX_MSG_CNT);
	if (ret) 
	{
		QL_Longi_LOG("ql_rtos_queue_create failed!!!!");
	
	}
	ret = ql_rtos_queue_create(&GSMQueue, UART_MAX_MSG_SIZE, UART_MAX_MSG_CNT);
	if (ret) 
	{
		QL_Longi_LOG("ql_rtos_queue_create failed!!!!");
	
	}
	
    if (FLashCheck()==false)
    {// check configuration in flash 
		sprintf (str, "Powercom modem for LOngi version 1.00 first powerup \r\n") ;
		ql_rtos_queue_release(DebugUartqueue, strlen(str), (uint8 *)str, QL_NO_WAIT);
 	   if (EnterDefaultConfig() !=QL_SUCCESS)
	   while (1)
	   {
		   // if flash error kill 
		 sprintf (str, "flash error\r\n");
    	 ql_rtos_queue_release(DebugUartqueue, strlen(str), (uint8 *)str, QL_NO_WAIT);
		 ql_rtos_task_sleep_ms(6000);  // y.f. wake every 1500 ms
	   }		  
	  
    }else 
	{
		sprintf (str, "Powercom modem for LOngi version 1.00 config not changed\r\n");
		ql_rtos_queue_release(DebugUartqueue, strlen(str), (uint8 *)str, QL_NO_WAIT);
	}
	printgsmconfig(&RepeaterConfig);

	MeteInit();

	err = ql_rtos_task_create(&ql_Longi4G_task, 4096*2, APP_PRIORITY_NORMAL, "ql_Longi4G", ql_Longi4G_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_Longi_LOG("longi task create failed");
	}
}

void  ConfigLeds(void)
{
	 ql_pin_set_func(LED_1, 0);
	 ql_pin_set_func(LED_2, 0);
     ql_gpio_init(GPIO_1, GPIO_OUTPUT, 0xff, LVL_HIGH);
     ql_gpio_init(GPIO_2, GPIO_OUTPUT, 0xff, LVL_LOW);
}
void ToggleLeds(void)
{
					
    if (blinkFlag)
		if (--blinkFlag==0)
		{			
			blinkFlag=10;
			if (leds_state )
			{
				leds_state = false;
				ql_gpio_set_level(GPIO_1, LVL_HIGH);
				ql_gpio_set_level(GPIO_2, LVL_LOW);
			}
			else
			{
				leds_state = true;
				ql_gpio_set_level(GPIO_2, LVL_HIGH);
				ql_gpio_set_level(GPIO_1, LVL_LOW);
			}
		}
}
ql_errcode_e EnterDefaultConfig(void)
{
  memset((uint8_t*)&RepeaterConfig,0,sizeof(RepeaterConfigStruct));
  RepeaterConfig.protocol= DLMS_PROTOCOL; // y.f. longi 1/8/2022
  RepeaterConfig.MeterSerial.BaudRate =  9600;
  RepeaterConfig.MeterSerial.DataBit = 8;
  RepeaterConfig.MeterSerial.Parity = NONE;
  RepeaterConfig.MeterSerial.StopBit =1;
  RepeaterConfig.GSMSerial.BaudRate =  115200;
  RepeaterConfig.GSMSerial.DataBit = 8;
  RepeaterConfig.GSMSerial.Parity = NONE;
  RepeaterConfig.GSMSerial.StopBit =1;
  RepeaterConfig.DebugSerial.BaudRate =  9600;
  RepeaterConfig.DebugSerial.DataBit = 8;
  RepeaterConfig.DebugSerial.Parity = NONE;
  RepeaterConfig.DebugSerial.StopBit =1;
  memcpy (RepeaterConfig.GsmConfig.APN ,"internet",8);
 // memcpy (RepeaterConfig.GsmConfig.Port ,"2334",4);
 // memcpy (RepeaterConfig.GsmConfig.ServerIP ,"2.55.107.55",11);
  memcpy (RepeaterConfig.GsmConfig.Port ,"2335",4);
  memcpy (RepeaterConfig.GsmConfig.ServerIP ,"31.154.79.218",13);
// moved to WriteInternalFlashConfig  RepeaterConfig.Crc =  CrcBlockCalc((unsigned char*)&RepeaterConfig, sizeof(RepeaterConfigStruct)-2);
  return (WriteInternalFlashConfig());
}
void printgsmconfig(RepeaterConfigStruct  *pConfig)
{
	char str[100] = {0};
 	sprintf(str,"Server IP= ");
	ql_rtos_queue_release(DebugUartqueue, 11, (uint8 *)str, QL_NO_WAIT);
	memcpy(str, &pConfig->GsmConfig.ServerIP[0], 20);
	ql_rtos_queue_release(DebugUartqueue, 20, (uint8 *)str, QL_NO_WAIT);
    sprintf(str,"\r\n Server Port=");
	ql_rtos_queue_release(DebugUartqueue, 15, (uint8 *)str, QL_NO_WAIT);
	memcpy(str, &pConfig->GsmConfig.Port[0], 5);
    ql_rtos_queue_release(DebugUartqueue, 5, (uint8 *)str, QL_NO_WAIT);
    sprintf(str,"\r\n APN =");
	ql_rtos_queue_release(DebugUartqueue, 10, (uint8 *)str, QL_NO_WAIT);
	memcpy(str, &pConfig->GsmConfig.APN[0], 20);
    ql_rtos_queue_release(DebugUartqueue, 20, (uint8 *)str, QL_NO_WAIT);
	
}
#if 0
void SendData2Modem(uint8_t * DataIn, uint16_t Len)
{
	ql_rtos_queue_release(GSMQueue, Len, DataIn, QL_NO_WAIT);
	ql_rtos_task_sleep_ms(100);  
}
#endif

void SendData2Meter(uint8_t * DataIn, uint16_t Len)
{
  ql_rtos_queue_release(MeterUartqueue, Len, DataIn, QL_NO_WAIT);
  ql_rtos_task_sleep_ms(100);  
}
void MeteInit(void)
{
	//char str[100] = {0};
	DLMS_ReadMeterId();

}
void MeterUartIn(uint8_t *bufintmp, uint16_t len)
{
	Meter_Handler(bufintmp,len,DATA_IN_EVENT); 
}