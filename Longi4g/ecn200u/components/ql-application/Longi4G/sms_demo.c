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
10/10/2020  marvin          create

=================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_common.h"
#include "ql_api_osi.h"
#include "ql_api_sms.h"
#include "ql_log.h"
#include "longi4g.h"
#include "longi_nor_flash.h"
#define  MAX_SMS_SIZE 220
bool ConfigChanged = false;
const char SMS_IN_HEADER[]   = {"+GSMS:"};
ql_task_t sms_task = NULL;
ql_sem_t  sms_init_sem = NULL;
ql_sem_t  sms_list_sem = NULL;
extern void Send2DebugUart(uint8_t *DataIn, uint16_t Len);
extern RepeaterConfigStruct RepeaterConfig;
void PrintSMSError (void);
uint16_t  ReadConfigFromSMS(uint8_t *pDdataIn, uint16_t Len);
void PrepareAckConfig(char *SMS_bufferTransmit);
void PrepareMeterIdAck(char *SMS_bufferTransmit);
bool sms_message_rx_event = false;
uint16_t new_msg_index;
char     oa_Tel[QL_TEL_MAX_LEN*4+1];    //Originating Address
void user_sms_event_callback(uint8_t nSim, int event_id, void *ctx)
{
	char str[100] = {0};
	switch(event_id)
	{
		case QL_SMS_INIT_OK_IND:
		{
			sprintf(str,"\r\n receive QL_SMS_INIT_OK_IN"); 
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			
			QL_SMS_LOG("QL_SMS_INIT_OK_IND");
			ql_rtos_semaphore_release(sms_init_sem);
			break;
		}
		case QL_SMS_NEW_MSG_IND:
		{
			ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
			sprintf(str,"\r\n receive sms new msg sim=%d, index=%d, storage memory=%d", nSim, msg->index, msg->mem); 
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			sms_message_rx_event = true;
			new_msg_index = msg->index;
		
			QL_SMS_LOG("sim=%d, index=%d, storage memory=%d", nSim, msg->index, msg->mem);
			break;
		}
		case QL_SMS_LIST_IND:
		{
#if 0
			ql_sms_msg_s *msg = (ql_sms_msg_s *)ctx;
			QL_SMS_LOG("sim=%d,index=%d, msg = %s",nSim, msg->index, msg->buf);
#endif
			break;
		}
        case QL_SMS_LIST_EX_IND:
        {
 			sprintf(str,"\r\n QL_SMS_LIST_EX_IND"); 
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 

			ql_sms_recv_s *msg = (ql_sms_recv_s *)ctx;
            QL_SMS_LOG("index=%d,os=%s,tooa=%u,status=%d,fo=0x%x,dcs=0x%x,scst=%d/%d/%d %d:%d:%d±%d,uid=%u,total=%u,seg=%u,dataLen=%d,data=%s",
                msg->index,msg->oa,msg->tooa,msg->status,msg->fo,msg->dcs,
                msg->scts.uYear,msg->scts.uMonth,msg->scts.uDay,msg->scts.uHour,msg->scts.uMinute,msg->scts.uSecond,msg->scts.iZone,
                msg->uid,msg->msg_total,msg->msg_seg,msg->dataLen,msg->data);
            break;
        }
		case QL_SMS_LIST_END_IND:
		{
			sprintf(str,"\r\n QL_SMS_LIST_END_IND"); 
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			
			QL_SMS_LOG("QL_SMS_LIST_END_IND");
			ql_rtos_semaphore_release(sms_list_sem);
			break;
		}
		case QL_SMS_MEM_FULL_IND:
		{
			sprintf(str,"\r\n QL_SMS_MEM_FULL_IND"); 
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 

			ql_sms_new_s *msg = (ql_sms_new_s *)ctx;
			QL_SMS_LOG("QL_SMS_MEM_FULL_IND sim=%d, memory=%d",nSim,msg->mem);
			break;
		}
		default :
			break;
	}
}


void sms_demo_task(void * param)
{
	char addr[20] = {0};
	char str[600] = {0};
	uint8_t nSim = 0;
	uint16_t SMSResult;
	char SmsOut[200];
	sprintf(str,"\r\n start sms task "); 
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 

//	QL_SMS_LOG("enter");
	ql_sms_callback_register(user_sms_event_callback);
	
	//wait sms ok
 //    if(ql_rtos_semaphore_wait(sms_init_sem, QL_WAIT_FOREVER))
    {
	//	QL_SMS_LOG("Waiting for SMS init timeout");
		sprintf(str,"\r\n rx sms init succssed "); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}
	if(QL_SMS_SUCCESS == ql_sms_get_center_address(nSim, addr, sizeof(addr))){
		sprintf(str,"\r\n ql_sms_get_center_address OK "); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 

	//	QL_SMS_LOG("ql_sms_get_center_address OK, addr=%s",addr);
	}else{
		sprintf(str,"\r\n ql_sms_get_center_address FAIL "); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 

	//	QL_SMS_LOG("ql_sms_get_center_address FAIL");
	}
    
	ql_sms_set_code_mode(QL_CS_GSM);
      //Send English text message start 
#if 0	  
	if(QL_SMS_SUCCESS == ql_sms_send_msg(nSim,"+972546262816","modem 4g send hello to yossi", GSM)){
		sprintf(str,"\r\n start SMS modem MSG send succssed"); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}else{
		sprintf(str,"\r\nSMS MSG FAILS to send "); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}
#endif	
	ql_sms_stor_info_s stor_info;
	if(QL_SMS_SUCCESS == ql_sms_get_storage_info(nSim,&stor_info)){
		//QL_SMS_LOG("ql_sms_get_storage_info OK");
		sprintf(str,"\r\n ql_sms_get_storage_info OK SM used=%u,SM total=%u,SM unread=%u,ME used=%u,ME total=%u,ME unread=%u, newSmsStorId=%u",
		stor_info.usedSlotSM,stor_info.totalSlotSM,stor_info.unReadRecordsSM,
			stor_info.usedSlotME,stor_info.totalSlotME,stor_info.unReadRecordsME,
			stor_info.newSmsStorId); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}else
	{
		sprintf(str,"\r\n ql_sms_get_storage_info FAIL"); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}

    //The first parameter specifies that SMS messages are read from SM
    ql_sms_set_storage(nSim,SM,SM,SM);
	while (1)
	{
		ql_rtos_task_sleep_s(2);
		if (sms_message_rx_event)
		{
			sms_message_rx_event = false;
			//Read one messages in SIM

			uint16_t msg_len = 512;
			char *msg = malloc(msg_len);
			if(msg == NULL){
				sprintf(str,"\r\n malloc ql_sms_msg_s fail"); 
				Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 

		//		QL_SMS_LOG("malloc ql_sms_msg_s fail");
				goto exit;
			}
			memset(msg ,0 ,msg_len);

			ql_sms_mem_info_t sms_mem = {0};
			ql_sms_get_storage(nSim, &sms_mem);
			QL_SMS_LOG("mem1=%d, mem2=%d, mem3=%d", sms_mem.mem1, sms_mem.mem2, sms_mem.mem3);
			if(msg)free(msg);

			//Read SMS messages as text
			ql_sms_recv_s *sms_recv = (ql_sms_recv_s *)calloc(1,sizeof(ql_sms_recv_s));
			if(sms_recv == NULL)
			{
				QL_SMS_LOG("calloc FAIL");
				goto exit;
			}
//			if(QL_SMS_SUCCESS == ql_sms_read_msg_ex(nSim,2, TEXT,sms_recv)){
	
			if(QL_SMS_SUCCESS == ql_sms_read_msg_ex(nSim,new_msg_index, TEXT,sms_recv))
			{
				sprintf(str,"\r\nindex=%d,os=%s,tooa=%u,status=%d,fo=0x%x,dcs=0x%x,scst=%d/%d/%d %d:%d:%d±%d,uid=%u,total=%u,seg=%u,dataLen=%d,data=%s",
					sms_recv->index,sms_recv->oa,sms_recv->tooa,sms_recv->status,sms_recv->fo,sms_recv->dcs,
					sms_recv->scts.uYear,sms_recv->scts.uMonth,sms_recv->scts.uDay,sms_recv->scts.uHour,sms_recv->scts.uMinute,sms_recv->scts.uSecond,sms_recv->scts.iZone,
					sms_recv->uid,sms_recv->msg_total,sms_recv->msg_seg,sms_recv->dataLen,sms_recv->data);
					Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
					ql_rtos_task_sleep_ms(50); // delay 2 sec
					strcat (oa_Tel, sms_recv->oa);
					SMSResult = ReadConfigFromSMS(sms_recv->data, sms_recv->dataLen);
					if (SMSResult==0)
					{
						sprintf(str,"\r\n wrong sms msg"); 
						Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
					}
					else
					{		
							
						if (SMSResult==1)
						{// enter to flash only if valid configuration is found
											//Send message ack to sender 
							PrepareAckConfig(SmsOut);			
							if(QL_SMS_SUCCESS == ql_sms_send_msg(nSim,oa_Tel,SmsOut, GSM))
							{
								sprintf(str,"\r\n start SMS modem MSG send succssed"); 
								Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
								ql_rtos_task_sleep_ms(10); // delay 2 sec
							}else
							{
								sprintf(str,"\r\nSMS MSG FAILS to send "); 
								Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
							}
							WriteInternalFlashConfig(); // 3/1/2023 store the configuration
							ql_rtos_task_sleep_ms(500); // delay 2 sec
							ConfigChanged = true;
						}
						else if (SMSResult==2) // y.f. only read configuration send ack 
						{
							PrepareAckConfig(SmsOut);			
							if(QL_SMS_SUCCESS == ql_sms_send_msg(nSim,oa_Tel,SmsOut, GSM))
							{
								sprintf(str,"\r\n start SMS modem MSG send succssed"); 
								Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
								ql_rtos_task_sleep_ms(10); // delay 2 sec
							}else
							{
								sprintf(str,"\r\nSMS MSG FAILS to send "); 
								Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
							}						
							
						}
						else if (SMSResult==3) // y.f. meter id ack 
						{
							PrepareMeterIdAck(SmsOut);
							if(QL_SMS_SUCCESS == ql_sms_send_msg(nSim,oa_Tel,SmsOut, GSM))
							{
								sprintf(str,"\r\n start SMS modem MSG send succssed"); 
								Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
								ql_rtos_task_sleep_ms(10); // delay 2 sec
							}else
							{
								sprintf(str,"\r\nSMS MSG FAILS to send "); 
								Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
							}	
						}
					}
			}else{
			
			Send2DebugUart((uint8_t *)"sms read failed ",20); // return to data mode 

			}
			
			if(sms_recv)free(sms_recv);
	//Delete message.
			if(QL_SMS_SUCCESS == ql_sms_delete_msg_ex(nSim, 0, QL_SMS_DEL_ALL)){
			QL_SMS_LOG("delete msg OK");
			}else{
				QL_SMS_LOG("delete sms FAIL");
			}
		}
	}

	//Read all message in SIM
#if 0
	ql_sms_set_storage(nSim,SM,SM,SM);//set sms storage as SIM.
	if(QL_SMS_SUCCESS == ql_sms_read_msg_list(nSim, TEXT)){
		if(ql_rtos_semaphore_wait(sms_list_sem, QL_WAIT_FOREVER)){
			QL_SMS_LOG("sms_list_sem time out");
		}
	}else{
		QL_SMS_LOG("get msg list FAIL");
	}
#endif
	
	goto exit;
exit:

	ql_rtos_task_delete(NULL);
}


QlOSStatus ql_sms_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;

	err = ql_rtos_task_create(&sms_task, 4096, APP_PRIORITY_NORMAL, "QsmsApp", sms_demo_task, NULL, 2);
	if(err != QL_OSI_SUCCESS)
	{
		QL_SMS_LOG("sms_task created failed, ret = 0x%x", err);
	}
	
	err = ql_rtos_semaphore_create(&sms_init_sem, 0);
	if(err != QL_OSI_SUCCESS)
	{
		QL_SMS_LOG("sms_init_sem created failed, ret = 0x%x", err);
	}

	err = ql_rtos_semaphore_create(&sms_list_sem, 0);
	if(err != QL_OSI_SUCCESS)
	{
		QL_SMS_LOG("sms_init_sem created failed, ret = 0x%x", err);
	}

	return err;
}

// this was taken from  gw_ra4m3 project 
uint16_t  ReadConfigFromSMS(uint8_t *pDdataIn, uint16_t Len)
{
	char *   position;
   	uint16_t bInd=0;
	position= strnstr ((char*)pDdataIn,(char *)SMS_IN_HEADER,MAX_SMS_SIZE);
    if (position)
    {
		RepeaterConfigStruct tmpRepeaterConfig;
		position = strnstr ((char*)pDdataIn,",2,",MAX_SMS_SIZE);
		if (position==0)
		{
			position = (strstr((char*)pDdataIn,",3,"));
			if (position ==0 ) 
			{
				position = (strstr((char*)pDdataIn,",4,")); // meter id req 
				if (position==0) return 0;
				else return 3;					
			}
			else  
				return 2 ; // this is read config just send config
		}
		position = position+3;
		// extract server ip Ex 31.22.22.154
		memcpy ((uint8_t*)& tmpRepeaterConfig,(uint8_t*)&RepeaterConfig,sizeof (RepeaterConfigStruct));
		memset ((uint8_t*)& tmpRepeaterConfig.GsmConfig,0,sizeof (GSMConfigStruct));// y.f. 3/1/23
		for (bInd = 0 ;bInd < 17 ; bInd++)
		{
			if ((char)(*position) ==',')
				break;
			if (((char)(*position) =='.') ||
			   ((*position <= 0x39) && (*position >=0x30)))
				tmpRepeaterConfig.GsmConfig.ServerIP[bInd]=*position++;
			else
			{
				if ((*position==0xa) || (*position==0xd))
				{
					position++;
				}
				else
				{// 14/12/22 ignore cr line feed
					PrintSMSError();
					return 0; // not valid char
				}
			}
	   }

	   // RepeaterConfig.GsmConfig.ServerIP  [bInd]=0;
	   position++;

	   for (bInd = 0 ;bInd < 6 ; bInd++)
	   {
			if ((char)(*position) ==',')
				break;
			if   ((*position <= 0x39) && (*position >=0x30))
				tmpRepeaterConfig.GsmConfig.Port[bInd]=*position++;
			else
			if ((*position==0xa) || (*position==0xd))
			{
				position++;
			}
			else
			{// 14/12/22 ignore cr line feed
				PrintSMSError();
				return 0; // not valid char
			}
	   }

	 //  tmpRepeaterConfig.GsmConfig.Port[bInd]=0;
		 position++;
	   for (bInd = 0 ;bInd < 20 ; bInd++) // max apn chars
	   {
		 if (*position ==',')
		 {
			 tmpRepeaterConfig.GsmConfig.APN[bInd] = 0;
			 memcpy((uint8_t*)&RepeaterConfig,(uint8_t*)&tmpRepeaterConfig,sizeof(RepeaterConfigStruct)); //4/1/23 only if everything is OK change RepeaterConfig
			return 1;
		 }
		 tmpRepeaterConfig.GsmConfig.APN[bInd] = *position++;
	   }
	}
   PrintSMSError();
   return 0; // not valid char
}


void PrintSMSError (void)
{
        char str[100];
        sprintf (( char*)&str, "sorry wrong config sms " );
       	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode  
		ql_rtos_task_sleep_ms(10); // delay 2 sec
}

void PrepareAckConfig(char *SMS_bufferTransmit)
{
		SMS_bufferTransmit[0] =0;
        strcat(SMS_bufferTransmit , "IP:");
        strcat(SMS_bufferTransmit , RepeaterConfig.GsmConfig.ServerIP);
        strcat(SMS_bufferTransmit, "\r\nPORT:");
        strcat(SMS_bufferTransmit, RepeaterConfig.GsmConfig.Port);
	
}
void PrepareMeterIdAck(char *SMS_bufferTransmit)
{
		SMS_bufferTransmit[0] =0;
        strcat(SMS_bufferTransmit , "MeterId:");
		sprintf (&SMS_bufferTransmit[8], "%llu",RepeaterConfig.ExtendedAddress);
}
