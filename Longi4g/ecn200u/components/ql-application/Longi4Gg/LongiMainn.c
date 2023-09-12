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
extern void Gsm_Init(void);
void printgsmconfig(RepeaterConfigStruct  *pConfig);
void SendData2Modem(uint8_t * DataIn, uint16_t Len);
void SendData2Meter(uint8_t * DataIn, uint16_t Len);
void MeteInit(void);
ql_errcode_e EnterDefaultConfig(void);
uint16_t blinkcntr =10;
uint16_t blinkFlag=CONNECTION_2_METER;
static bool leds_state;
static void ToggleLeds(void);
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
MeterQueueStruct InfoDebug;
uint16_t GsmTimer;
uint16_t MeterTimer;

static void ql_Longi4G_thread(void *ctx)
{
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

}

void ql_Longi4G_app_init(void)
{
	int ret = 0;

	QlOSStatus err = QL_OSI_SUCCESS;
	ql_task_t ql_Longi4G_task = NULL;
	char *str = malloc(100 * sizeof(char));
//    QL_Longi_LOG("Longi4G enter");
	ConfigLeds();
	
    ql_rtos_task_sleep_ms(7000);  // y.f. wake every 1500 ms	
    if (FLashCheck()==false)
    {// check configuration in flash 
	   sprintf (str, "Powercom modem for LOngi version 1.00 first powerup \r\n") ;
	   Send2DebugUart((uint8_t *)str, strlen(str));
 	   if (EnterDefaultConfig() !=QL_SUCCESS)
	   while (1)
	   {
		   // if flash error kill 
		 sprintf (str, "flash error\r\n");
 	     Send2DebugUart((uint8_t *)str, strlen(str));
		 ql_rtos_task_sleep_ms(6000);  // y.f. wake every 1500 ms
	   }		  
    }
	else 
	{
		sprintf (str, "Powercom modem for LOngi version 1.00 config not changed\r\n");
	    Send2DebugUart((uint8_t *)str, strlen(str));
	}
	printgsmconfig(&RepeaterConfig);
	ret = ql_rtos_queue_create(&DebugUartqueue, UART_MAX_MSG_SIZE, UART_MAX_MSG_CNT);
	if (ret) 
	{
		sprintf(str,"ql_rtos_queue_create failed!!!! ");
	    Send2DebugUart((uint8_t *) str, strlen(str));
	}
	ret = ql_rtos_queue_create(&MeterUartqueue, UART_MAX_MSG_SIZE, UART_MAX_MSG_CNT);
	if (ret) 
	{
		sprintf(str,"ql_rtos_queue_create failed!!!! ");
	    Send2DebugUart((uint8_t *) str, strlen(str));

	}
	ret = ql_rtos_queue_create(&GSMQueue, UART_MAX_MSG_SIZE, UART_MAX_MSG_CNT);
	if (ret) 
	{
		sprintf(str,"ql_rtos_queue_create failed!!!! ");
	    Send2DebugUart((uint8_t *) str, strlen(str));

	}
	MeteInit();
    Gsm_Init();
	free(str);

	err = ql_rtos_task_create(&ql_Longi4G_task, 4096*2, APP_PRIORITY_NORMAL, "ql_Longi4G", ql_Longi4G_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		sprintf(str,"ql_rtos_longi task_create failed!!!! ");
	    Send2DebugUart((uint8_t *) str, strlen(str));
	}

}

void  ConfigLeds(void)
{
	 ql_pin_set_func(LED_1, 0);
	 ql_pin_set_func(LED_2, 0);
     ql_gpio_init(GPIO_1, GPIO_OUTPUT, 0xff, LVL_LOW);
     ql_gpio_init(GPIO_2, GPIO_OUTPUT, 0xff, LVL_HIGH);
}
void ToggleLeds(void)
{
	if (blinkcntr)
	{
		if (--blinkcntr==0)
		{
			blinkcntr =10;
			switch (blinkFlag)
			{
				default:
				case CONNECTION_2_METER:
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
				break;
				
				case READ_PARAM_FROM_METER:
					ql_gpio_set_level(GPIO_1, LVL_HIGH);
					ql_gpio_set_level(GPIO_2, LVL_LOW);
				break;
				case CONNECTION_2_METER:
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
				break;				
				case WAIT_FOR_REGISTRATION:
					if (leds_state )
					{
						leds_state = false;
						ql_gpio_set_level(GPIO_1, LVL_HIGH);
						ql_gpio_set_level(GPIO_2, LVL_LOW);
					}
					else
					{
						leds_state = true;
						ql_gpio_set_level(GPIO_2, LVL_LOW);
						ql_gpio_set_level(GPIO_1, LVL_LOW);
					}
				break;				
				case CONNECION_2_PROVIDER:
					ql_gpio_set_level(GPIO_1, LVL_LOW);
					ql_gpio_set_level(GPIO_2, LVL_HIGH);
				break;
				case CONNECTION_2_SERVER:
					if (leds_state )
						
					{
						leds_state = false;
						ql_gpio_set_level(GPIO_1, LVL_LOW);
						ql_gpio_set_level(GPIO_2, LVL_LOW);
					}
					else
					{
						leds_state = true;
						ql_gpio_set_level(GPIO_2, LVL_HIGH);
						ql_gpio_set_level(GPIO_1, LVL_HIGH);
					}
				break;
				case SERVER_DATA_MODE:
						ql_gpio_set_level(GPIO_2, LVL_HIGH);
						ql_gpio_set_level(GPIO_1, LVL_HIGH);
				break;
				case DATA_IN_METER:
						ql_gpio_set_level(GPIO_2, LVL_LOW);
						ql_gpio_set_level(GPIO_1, LVL_HIGH);
						blinkFlag=SERVER_DATA_MODE;
				break;
				case DATA_IN_SERVER:
						ql_gpio_set_level(GPIO_2, LVL_HIGH);
						ql_gpio_set_level(GPIO_1, LVL_LOW);
						blinkFlag=SERVER_DATA_MODE;
				break;		
			}
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
  memcpy (RepeaterConfig.GsmConfig.Port ,"2335",4);
  memcpy (RepeaterConfig.GsmConfig.ServerIP ,"31.154.79.218",13);
// moved to WriteInternalFlashConfig  RepeaterConfig.Crc =  CrcBlockCalc((unsigned char*)&RepeaterConfig, sizeof(RepeaterConfigStruct)-2);
  return (WriteInternalFlashConfig());
}
void printgsmconfig(RepeaterConfigStruct  *pConfig)
{
	char *str = malloc(100 * sizeof(char));
	sprintf(str,"Server IP= ");
	Send2DebugUart((uint8_t *) str, strlen(str));
	memcpy(str, &pConfig->GsmConfig.ServerIP[0], 20);
	Send2DebugUart((uint8_t *) str, strlen(str));
	sprintf(str,"\r\n Server Port=");
	Send2DebugUart((uint8_t *) str, strlen(str));
	memcpy(str, &pConfig->GsmConfig.Port[0], 5);
	Send2DebugUart((uint8_t *) str, strlen(str));
	sprintf(str,"\r\n APN =");
	Send2DebugUart((uint8_t *) str, strlen(str));
	memcpy(str, &pConfig->GsmConfig.APN[0], 20);
	Send2DebugUart((uint8_t *) str, strlen(str));
	free (str);
	
}
void SendData2Modem(uint8_t * DataIn, uint16_t Len)
{
  MeterQueueStruct *InfoMeter = malloc(sizeof(MeterQueueStruct));
  InfoMeter->len =Len;
  InfoMeter->rcvbuf = DataIn;
  ql_rtos_queue_release(GSMQueue, InfoMeter->len,  (uint8_t*)InfoMeter, QL_NO_WAIT);
  Send2DebugUart((uint8_t *)DataIn, Len);
  ql_rtos_task_sleep_ms(10);  
  free(InfoMeter);

}
void SendData2Meter(uint8_t * DataIn, uint16_t Len)
{
  MeterQueueStruct *InfoMeter = malloc(sizeof(MeterQueueStruct));
  InfoMeter->len =Len;
  InfoMeter->rcvbuf = DataIn;
  ql_rtos_queue_release(MeterUartqueue, InfoMeter->len,  (uint8_t*)InfoMeter, QL_NO_WAIT);
  Send2DebugUart((uint8_t *)DataIn, Len);
  ql_rtos_task_sleep_ms(10);  
  free(InfoMeter); 
 
}
void MeteInit(void)
{

	DLMS_ReadMeterId();
#if 0	
	while (CheckDLMSState()==0) 
	{
	    ql_rtos_task_sleep_ms(1000);  // y.f. wake every 1500 ms
		Meter_Handler(0,0,TIME_OUT_EVENT); // wait here till read meter id is finished 
	}
#endif	

}
