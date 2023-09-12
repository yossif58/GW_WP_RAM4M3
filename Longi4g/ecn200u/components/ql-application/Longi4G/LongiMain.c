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
#include "gpio_int_demo.h"
//#include "ql_virt_at_demo.h"
//#include "socket_demo.h"
#include "crc.h"

#pragma pack(1)


/*===========================================================================
 * Variate
 ===========================================================================*/
#define LED_1  QUEC_PIN_DNAME_GPIO_1 
#define LED_2  QUEC_PIN_DNAME_GPIO_2 
//MeterQueueStruct InfoMeter;
//extern void Gsm_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
extern bool Meter_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
extern void DLMS_ReadMeterId(void);
extern bool CheckDLMSState(void);
//extern void SendReadParam_msg(void);
bool meter_event_in=false;

//extern bool Gsm_event_in;
//extern bool AT_timer_event;
bool Gsm_timer_event;
extern uint16_t meterlen;  
extern uint8_t  MeterDatain [];
//extern uint16_t Gsmlen; 
//extern uint8_t  GsmDatain [];
extern void Send2DebugUart(uint8_t *pDdataIn ,uint32_t len);
#ifdef LONGI_PUSH
ql_timer_t Push_timer ;
bool Push_timer_event=false;
extern uint32_t PushHandler(uint8_t *pInfo, uint16_t DataLen , uint8_t EventType);

#else
extern void SendSync_msg(void);
#endif

void printgsmconfig(RepeaterConfigStruct  *pConfig);
//void SendData2GSM(uint8_t * pDdataIn, uint16_t len);
//void clrMeterEvent(void);
//void SendData2Modem(uint8_t * DataIn, uint16_t Len);
//void SendData2Meter(uint8_t * DataIn, uint16_t Len);
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
//ql_queue_t	GSMQueue;
ql_queue_t DebugUartqueue;
ql_queue_t MeterUartqueue;
RepeaterConfigStruct RepeaterConfig;
uint16_t blinkcntr =10;
uint16_t blinkFlag=CONNECTION_2_METER;
//uint16_t GsmTimer;
uint16_t MeterTimer;
ql_timer_t Uart_debounce_timer = NULL;
ql_timer_t Gsm_debounce_timer = NULL;
ql_timer_t AT_debounce_timer = NULL;
#ifdef LONGI_PUSH
ql_timer_t Push_timer = NULL;
#endif
uint16_t send_meter_timeout_cntr=0;
static void ql_Longi4G_thread(void *ctx)
{



	//char ch = '1';
	while(1)
	{
#if 0		
		ql_event_t longi_main_event;
		if(ql_event_wait(&longi_main_event, 500) != 0)
		{
			switch (longi_main_event.id)
			{
				case METER_EVENT_IN:
					Meter_Handler(MeterDatain,meterlen,DATA_IN_EVENT); 
				meterlen =0;
				break;
//				case METER_EVENT_TIME_OUT:
//					Meter_Handler( 0, 0 , TIME_OUT_EVENT);
//				break;
				default:
				break;
			}
		}
#endif		
		if (meter_event_in)
		{
			meter_event_in =false;

			Meter_Handler(MeterDatain,meterlen,DATA_IN_EVENT); 
			meterlen =0;
		}
		else
		if (MeterTimer)
		{
			if  ((--MeterTimer) ==0)
			{
				Meter_Handler( 0, 0 , TIME_OUT_EVENT);
			}
		}
		ToggleLeds();
		ql_rtos_task_sleep_ms(200);  // y.f. wake every 100 ms	
#ifdef LONGI_PUSH		
		if (Push_timer_event)
		{
			Push_timer_event = false;
			PushHandler(0, 0 , 0); // y.f. 4/9/23 send scheduler and start read push process if needed 
	    }
#else	
		if ((++send_meter_timeout_cntr% 100) ==0)
		{
			send_meter_timeout_cntr =0;
            SendSync_msg();
		}		
#endif
	//	char *str = calloc (1,1024);
#ifdef nowaait	
		if (meter_event_in)
		{
			meter_event_in =false;
		//	Send2DebugUart((uint8_t*)MeterDatain ,meterlen);
			Meter_Handler(MeterDatain,meterlen,DATA_IN_EVENT); 
			meterlen =0;
		}
		else
			
		if (MeterTimer)
		{
			if  ((--MeterTimer) ==0)
			{
				Meter_Handler( 0, 0 , TIME_OUT_EVENT);
			}
		}
	
		
		ToggleLeds();
		ql_rtos_task_sleep_ms(100);  // y.f. wake every 100 ms
	
		
	//	free (str);
	//	str = NULL;
#endif	
	}
    ql_rtos_task_delete(NULL);	
		

}
static void _ql_uart_timer_callback(void *ctx)
{
#if 0
	ql_event_t event;
	event.id = METER_EVENT_IN;
    ql_rtos_event_send(ql_Longi4G_thread, &event); //Feed a watchdog for the specified task.
#endif
    ql_rtos_timer_stop(Uart_debounce_timer);
	meter_event_in =true;
}


static void _ql_gsm_timer_callback(void *ctx)
{
	
    ql_rtos_timer_stop(Gsm_debounce_timer);
	Gsm_timer_event = true;

//    ql_rtos_event_send(headset_det_task, &event);
}
#ifdef LONGI_PUSH
static void _ql_Push_timer_callback(void *ctx)
{
    ql_rtos_timer_start(Push_timer,20000,1);
	Push_timer_event = true;
   
}
#endif

#if 0
static void _ql_AT_timer_callback(void *ctx)
{
    ql_rtos_timer_stop(AT_debounce_timer);
	AT_timer_event = true;

//    ql_rtos_event_send(headset_det_task, &event);
}
#endif

void ql_Longi4G_app_init(void)
{
	int ret = 0;
//	blinkFlag =100;
	QlOSStatus err = QL_OSI_SUCCESS;
	ql_task_t ql_Longi4G_task = NULL;
	//unsigned char data[100];
	
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
#if 0	
	ret = ql_rtos_queue_create(&GSMQueue, UART_MAX_MSG_SIZE, UART_MAX_MSG_CNT);
	if (ret) 
	{
		QL_Longi_LOG("ql_rtos_queue_create failed!!!!");
	
	}
#endif
	
    ql_rtos_task_sleep_ms(1000);  // wait for stabilaize
 
    char str[100] = {0};
    if (FLashCheck()==false)
    {

		
		
		// check configuration in flash 
		sprintf (str, "Powercom modem for LOngi version 1.02 first powerup \r\n") ;
		Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode 
	//	ql_rtos_queue_release(DebugUartqueue, strlen(str), (uint8 *)str, QL_NO_WAIT);
 	   if (EnterDefaultConfig() !=QL_SUCCESS)
	   while (1)
	   {
	   
		   
		   // if flash error kill 
		 sprintf (str, " write config to flash failed \r\n");
	     Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode  
		 ql_rtos_queue_release(DebugUartqueue,strlen(str), (uint8_t*)str, QL_NO_WAIT);
		 ql_rtos_task_sleep_ms(6000);  // y.f. wake every 1500 ms

		while (1)
		{
			ql_rtos_task_sleep_ms(12000);
			ql_gpio_set_level(GPIO_1, LVL_HIGH);
			ql_gpio_set_level(GPIO_2, LVL_HIGH);
			ql_rtos_task_sleep_ms(12000);		
			ql_gpio_set_level(GPIO_1, LVL_LOW);
			ql_gpio_set_level(GPIO_2, LVL_LOW);		
			
		}
			   
		 
		 
		 
	   }		  
	  
    }else 
	{

		sprintf (str, "Powercom modem for LOngi version 1.02 config not changed\r\n");
		Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode 
		ql_rtos_task_sleep_ms(10);  // wait for stabilaize
	}

	printgsmconfig(&RepeaterConfig);

	MeteInit();
	ql_gpio_set_level(GPIO_1, LVL_HIGH);
	ql_gpio_set_level(GPIO_2, LVL_HIGH);

#if 0	
	while (1)
	{
		ql_rtos_task_sleep_ms(12000);
		ql_gpio_set_level(GPIO_1, LVL_HIGH);
	    ql_gpio_set_level(GPIO_2, LVL_HIGH);
        ql_rtos_task_sleep_ms(12000);		
	//	sprintf (str, "Powercom cont test \r\n") ;
	//	Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode
		ql_gpio_set_level(GPIO_1, LVL_LOW);
		ql_gpio_set_level(GPIO_2, LVL_LOW);		
		
	}
#endif	
	err = ql_rtos_task_create(&ql_Longi4G_task, 4096*2, APP_PRIORITY_NORMAL, "ql_Longi4G", ql_Longi4G_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_Longi_LOG("longi task create failed");
	}
    err = ql_rtos_timer_create(&Uart_debounce_timer, NULL, _ql_uart_timer_callback, NULL);
#ifdef LONGI_PUSH	
    err = ql_rtos_timer_create(&Push_timer, NULL, _ql_Push_timer_callback, NULL);
#endif	
	err = ql_rtos_timer_create(&Gsm_debounce_timer, NULL, _ql_gsm_timer_callback, NULL);
	ql_gpioint_app_init();
//	err = ql_rtos_timer_create(&AT_debounce_timer, NULL, _ql_AT_timer_callback, NULL);

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
//	char str[6] = "Geek";	
	if (blinkcntr)
	{
		if (--blinkcntr==0)
		{
			blinkcntr =10;
//			Send2DebugUart((uint8_t*)str ,6);
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
				case WAIT_FOR_REGISTRATION:
					if (leds_state )
					{
						leds_state = false;
						ql_gpio_set_level(GPIO_1, LVL_LOW);
						ql_gpio_set_level(GPIO_2, LVL_HIGH);
					}
					else
					{
						leds_state = true;
						ql_gpio_set_level(GPIO_2, LVL_LOW);
						ql_gpio_set_level(GPIO_1, LVL_LOW);
					}
				break;
				
				case CONNECION_2_PROVIDER:
				if (leds_state )
					{
						leds_state = false;
						ql_gpio_set_level(GPIO_1, LVL_HIGH);
						ql_gpio_set_level(GPIO_2, LVL_HIGH);
					}
					else
					{
						leds_state = true;
						ql_gpio_set_level(GPIO_2, LVL_HIGH);
						ql_gpio_set_level(GPIO_1, LVL_LOW);
					}
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
 // memcpy (RepeaterConfig.GsmConfig.Port ,"2334",4);
 // memcpy (RepeaterConfig.GsmConfig.ServerIP ,"2.55.107.55",11);
  memcpy (RepeaterConfig.GsmConfig.Port ,"2339",4);
  memcpy (RepeaterConfig.GsmConfig.ServerIP ,"31.154.79.218",13);
// moved to WriteInternalFlashConfig  RepeaterConfig.Crc =  CrcBlockCalc((unsigned char*)&RepeaterConfig, sizeof(RepeaterConfigStruct)-2);
  return (WriteInternalFlashConfig());
}
void printgsmconfig(RepeaterConfigStruct  *pConfig)
{
	char str[100] = {0};
 	sprintf(str,"Server IP= ");
   	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	memcpy((uint8_t *)str, &pConfig->GsmConfig.ServerIP[0], strlen((const char *)&pConfig->GsmConfig.ServerIP[0]));
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
    sprintf(str,"\r\n Server Port=");
   	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
 	memcpy(str, &pConfig->GsmConfig.Port[0], strlen((const char*)&pConfig->GsmConfig.Port[0]));
	Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode 
    sprintf(str,"\r\n APN =");
	Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode 
	memcpy(str, &pConfig->GsmConfig.APN[0], strlen((const char*)&pConfig->GsmConfig.APN[0]));
	Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode 
}

void MeteInit(void)
{
	DLMS_ReadMeterId();
}
#if 0
void clrMeterEvent(void)
{
	memset (MeterDatain,0,1024);
	meter_event_in =false;
	meterlen =0;
}
#endif