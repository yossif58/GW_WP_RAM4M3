/**********************************************************************************
*																					*
* FILE NAME:																		*
*   DLMSProtocol.c																			*
*																					*
* DESCRIPTION:																		*
*   This file contains all routines for the dlms protocol                         *
* HISTORY																			*
*   Ferlandes Yossi																	*
* - CREATION      																	*
*   Yossi Ferlandes 01/06/2021/2009														*
*                                                                                  *
***********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_osi_def.h"
#include "longi_nor_flash.h"
#include "ql_fs.h"
#include "longi4g.h"
#include "socket_demo.h"
#include "sms_demo.h"
#include "LongiWrapper.h"
//#include "utils.h"
#pragma pack(1)
typedef enum
{
  DLMS_IDLE_STATE =0,
 // DLMS_SNRM_STATE,
 // DLMS_AARQ_STATE,
  DLMS_READ_METER_ID_STATE,
  DLMS_READ_PARAMETERS_STATE,
  DLMS_DATA_STATE,
 }DLMState;
typedef enum
{
  PUSH_APDU =0,
  WRAPPER_APDU  = 0x10,
  LOCAL_SERVICE = 0xf0,
 
 }MeterFrameType;

typedef enum
{
  DST_METER = 3,
  DST_CLIENT = 0x41
}DstAddress;
typedef struct 
{
  uint8_t                 Stx;
  uint8_t                 Frametype;    // 
  uint8_t                 Len;    // 
  uint8_t                 info[200];
 
} LongiframeStruct;
typedef enum
{
  NOT_VALID_MSG =0,
  LOCAL_SERVICE_MSG,
  WRAPPER_MSG,
  DLT_MSG,
  HDLC_MSG,
  EVENT_REPORT,
  
}Lingimsgtype;
extern ql_queue_t DebugUartqueue;
extern ql_queue_t MeterUartqueue;
#ifdef LONGI_PUSH				
extern 	ql_timer_t Push_timer ;
extern uint32_t PushHandler(uint8_t *pInfo, uint16_t DataLen , uint8_t EventType);

#endif	
extern RepeaterConfigStruct RepeaterConfig;
extern uint16_t blinkFlag;
extern uint16_t MeterTimer;
//extern void Send2DebugUart(uint8_t *pDdataIn ,uint32_t len);
//extern void Send2DebugQueue(uint8_t *DataIn, uint16_t Len);
extern void SendData2Meter(uint8_t * DataIn, uint16_t Len,bool display);
//extern uint32_t SendWrapper2Server(uint8_t *pMsgIn, uint16_t len);
extern uint16_t DLMSCrcCalc(uint8_t *crcReg, uint16_t len); 
extern void Send2DebugUart(uint8_t *DataIn, uint16_t Len);



uint32_t ParseDlmsMsg(uint8_t * pDatain, uint16_t Len, DstAddress dst,uint8_t * pDatOut, uint16_t *DataOutLen);
uint32_t ParseLongiMeterMsg(uint8_t * pDatain, uint16_t Len, uint8_t * pDataOut, uint16_t *DataOutLen);
uint32_t BuildLongiMeterMsg(uint8_t * pDatain, uint16_t Len);
void SendReadMeter2Meter(void);
bool Meter_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
void DLMS_ReadMeterId(void);
void Senddisconnect_msg(void);
void SendReadParam_msg(void);
bool CheckDLMSState(void);
uint16_t ExtractMeterNum(uint8_t *pDdataIn);
uint16_t ExtractServerParams(uint8_t *DataOut);
uint8_t meter_bufferTransmit[1024];
uint16_t dlmscrc = 0;
uint16_t tmpcrc =0;
// aarq from client   00 01 00 30 00 01 00 38 60 36 A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 01 AC 0A 80 08 31 32 33 34 35 36 37 38 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 00 1E 5D FF FF
// aare from meter/gw 00 01 00 01 00 30 00 2B 61 29 A1 09 06 07 60 85 74 05 08 01 01 A2 03 02 01 00 A3 05 A1 03 02 01 00 BE 10 04 0E 08 00 06 5F 1F 04 00 00 18 1D 00 C8 00 07

const char readparam_msg  [] =        {0x7e,0xf0,0x13,0xc0,0x41,1,0,1,0,0x81,0x19,0,0x89,0xFF,2,0,0x16,0x1,0,5,0x7E};
const char readmeterid_msg [] =       {0x7e,0xf0,0x11,0xc0,1,0x41,0,1,0,0,0x60,1,0,0xFF,2,0,0x33,0x53,0x7E};
const char disconnect_msg [] =        {0x7E, 0xA0 ,0x7,0x3 ,0x41 ,0x53 ,0x56 ,0xa2 ,0x7E};
//const char disconnect_res_msg [] =    {0x7E, 0xA0 ,0x7,0x41 ,0x3 ,0x1f ,0x26 ,0xec, 0x7E};
//const char init_push_data_msg [] =    {0x7E ,0 ,0x11 ,0xC0 ,1 ,0x41 ,0 ,1 ,0 ,0x80 ,0x60 ,1 ,0x2 ,0xFF ,2 ,0 ,0xA2 ,0xA7 ,0x7E};
//const char push_result_data_msg [] =  {0x7E ,0 ,0x13 ,0xC3 ,1 ,0x41 ,0 ,0x1 ,0 ,0x80 ,0x60 ,1 ,7 ,0xFF ,2 ,1 ,3 ,0 ,0xC1 ,0xEA ,0x7E};
#ifndef LONGI_PUSH
const char sync_msg_2_meter[] =       {0x7E ,0xF0 ,0xA4 ,0xC1 ,0x01 ,0x41 ,0x00 ,0x01 ,0x00 ,0x81 ,0x19 ,0x00 ,0x8B ,0xFF ,0x02 ,0x00 
										,0x01 ,0x0D ,0xA1 ,0x14 ,0x46 ,0x57 ,0x30 ,0x30 ,0x32 ,0x36 ,0x31 ,0x56 ,0x30 ,0x39 ,0x28 ,0x31 ,0x34 ,0x30 ,0x35 ,0x30 ,0x35 ,0x31 
										,0x35 ,0x29 ,0xA2 ,0x12 ,0x46 ,0x50 ,0x36 ,0x30 ,0x39 ,0x32 ,0x35 ,0x56 ,0x30,0x35 ,0x52 ,0x32 ,0x30 ,0x34 ,0x4D ,0x30 ,0x30 ,0x32 
										,0xA3 ,0x0F ,0x38 ,0x36 ,0x38 ,0x39 ,0x39 ,0x38 ,0x30 ,0x33 ,0x38 ,0x31 ,0x36 ,0x34 ,0x36 ,0x36 ,0x37 ,0xA4 ,0x12 ,0x45 ,0x43 ,0x32 
										,0x35 ,0x45 ,0x55 ,0x58 ,0x47 ,0x41 ,0x52 ,0x30 ,0x38 ,0x41 ,0x30 ,0x33 ,0x4D ,0x31 ,0x47 ,0xA5 ,0x14 ,0x38 ,0x39 ,0x38 ,0x36 ,0x30 
										,0x31 ,0x31 ,0x38 ,0x38 ,0x30 ,0x31 ,0x35 ,0x31 ,0x34 ,0x34 ,0x38 ,0x35 ,0x36 ,0x37 ,0x32 ,0xA6 ,0x08 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35 
										,0x36 ,0x37 ,0x38 ,0xA7 ,0x12 ,0xA8 ,0x01 ,0x02 ,0x03 ,0x04 ,0xA9 ,0x00 ,0xAA ,0x02 ,0xAB ,0x02 ,0xAC ,0x05 ,0x43 ,0x4D ,0x4E ,0x45 
										,0x54 ,0xAD ,0x0C ,0x43 ,0x48 ,0x49 ,0x4E ,0x41 ,0x20 ,0x4D ,0x4F ,0x42 ,0x49,0x4C ,0x45 ,0x04 ,0x08 ,0x7E};
#endif
		

//uint8_t  *pDataOut;
uint16_t DataoutLen;
uint8_t DataOut[1024];
uint8_t tmpdataout[1024];
extern void Gsm_Init(void);
DLMState DLMS_Protocol_State  ;

bool CheckDLMSState(void)
{
  if (DLMS_Protocol_State==DLMS_DATA_STATE) return true;
  return false;
}

bool Meter_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event)
{
  uint32_t ret_value;
  uint8_t retry=3;
  char str[50];
  switch (DLMS_Protocol_State)
  {
  case DLMS_IDLE_STATE:
      break;
  case DLMS_READ_METER_ID_STATE:
    
    if (Event == TIME_OUT_EVENT)
    {
         DLMS_ReadMeterId();
    }
    else
	
     if (Event == DATA_IN_EVENT)
	 {
	    
		  if (ParseLongiMeterMsg(pDdataIn, Len,DataOut,&DataoutLen)==LOCAL_SERVICE_MSG)
		  {
			// extract meter number from msg
			  if (ExtractMeterNum(DataOut ))
			  {
				blinkFlag =READ_PARAM_FROM_METER;
				DLMS_Protocol_State = DLMS_READ_PARAMETERS_STATE;
				SendReadParam_msg();
				MeterTimer = 20;
				retry = 3;
				return true;
			  }else
			  {
				sprintf (str, "wrong meter num msg\r\n");
				Send2DebugUart((uint8_t *)str,22); // return to data mode 
//				ql_rtos_queue_release(DebugUartqueue, sizeof(str), (uint8 *)str, QL_NO_WAIT);
			  }
		  }
		  else
		  {				
				sprintf (str, "wrong longi msg from meter\r\n");
				Send2DebugUart((uint8_t *)str,28); // return to data mode 
//				ql_rtos_queue_release(DebugUartqueue,  sizeof(str), (uint8 *)str, QL_NO_WAIT);
		  }
		  DLMS_ReadMeterId();

	 }
      break;
       case DLMS_READ_PARAMETERS_STATE:
    
       if (Event == TIME_OUT_EVENT)
        {
		  if (retry)
		  {
			  if (--retry)
			  {
				MeterTimer = 20;
				SendReadParam_msg();
				return true;
			  }
		  }			  
		  DLMS_ReadMeterId(); // back to start read metger id after 3 retries
        }
        else if (Event == DATA_IN_EVENT)
        {
	//		sprintf (str, "before  ParseLongiMeterMsg \r\n");
    //        Send2DebugUart((uint8_t *)str,29); // return to data mode  
            if (ParseLongiMeterMsg(pDdataIn, Len,DataOut,&DataoutLen)==LOCAL_SERVICE_MSG)
            {
              if (ExtractServerParams(DataOut))
              {
                DLMS_Protocol_State = DLMS_DATA_STATE;
#ifdef LONGI_PUSH				
				ql_rtos_timer_start(Push_timer,20000,1);
#endif				
				ql_sms_app_init();    // y.f. 22/06/23 
				ql_socket_app_init(); // // y.f. 7/6//23 start gsm only after establishing conne	
				// Gsm_Init(); ction with meter 
 				blinkFlag = WAIT_FOR_REGISTRATION;
                return false;
              }
            }
			else
			{
		      memcpy(str, pDdataIn, Len);
			  Send2DebugUart((uint8_t *)str,Len); // return to data mode 
			}
        }
	
  break;
  default:
  case DLMS_DATA_STATE:
	  
	  ret_value = ParseLongiMeterMsg(pDdataIn, Len,DataOut,&DataoutLen);
      if (ret_value==WRAPPER_MSG)  
      {
		SendData2Modem(DataOut, DataoutLen);
		blinkFlag =DATA_IN_METER;
      }
#ifdef LONGI_PUSH	  
	  else if ((ret_value ==EVENT_REPORT)|| (ret_value ==LOCAL_SERVICE_MSG))
	  {// it can be response to schedulare or response to push req messge
		PushHandler(DataOut, DataoutLen , 1);		  
	  }
#endif    
      break;
  }
  return true;
}

void DLMS_ReadMeterId(void)
{
//#define testnometer 
#ifdef testnometer
          DLMS_Protocol_State = DLMS_DATA_STATE;
 				blinkFlag = WAIT_FOR_REGISTRATION;
				RepeaterConfig.ExtendedAddress = 8888777;
				RepeaterConfig.AddressLen =8;
				ql_rtos_task_sleep_s(10);
			  
				ql_rtos_task_sleep_s(2);
				ql_socket_app_init(); // // y.f. 7/6//23 start gsm only after establishing conne
                ql_sms_app_init();    // y.f. 22/06/23 				
				return;
#else
  char *str = malloc (50 *sizeof(char)) ;
  DLMS_Protocol_State=DLMS_READ_METER_ID_STATE;
//  sprintf (str, "send read meter id \r\n");
  //Send2DebugUart((uint8_t *)str, strlen(str));
 //   ql_rtos_queue_release(DebugUartqueue, 20, (uint8 *)str, QL_NO_WAIT);
  SendReadMeter2Meter();
  MeterTimer = 10;
  free (str);
#endif

  
}

void SendReadMeter2Meter(void)
{
 
    char *str = malloc (50 *sizeof(char)) ;
    sprintf (str, "send read meter id \r\n");
	ql_rtos_queue_release(DebugUartqueue, sizeof(str), (uint8 *)str, QL_NO_WAIT);
	SendData2Meter((uint8_t*)readmeterid_msg,sizeof(readmeterid_msg),0); // return to data mode 
	free (str);
}
void Senddisconnect_msg(void)
{
  memcpy (meter_bufferTransmit,disconnect_msg,sizeof(disconnect_msg));
  SendData2Meter(meter_bufferTransmit,sizeof(disconnect_msg),0); // return to data mode 
}


void SendReadParam_msg(void)
 
{ 
    char *str = malloc (50 *sizeof(char)) ;
    sprintf (str, "send read meter params\r\n");
    Send2DebugUart((uint8_t *)str,24); // return to data mode  	
 	memcpy (meter_bufferTransmit,readparam_msg,sizeof(readparam_msg));
	SendData2Meter(meter_bufferTransmit,sizeof(readparam_msg),0); // return to data mode 
	free (str);
}       
uint16_t ExtractServerParams(uint8_t * pDdataIn)
{

#if 0  
  uint16_t bind;
  uint8_t *pInfo;
  char str[100];
  // 10/7/23 temp to be decide who is the master meter or SMS return use sms 

  //  verify read response  
  // C4 01 41 00 read response ok 
  if (!((pDdataIn[3]== 0xc4 )  && (pDdataIn[6]== 0x0 ) && (pDdataIn[7]== 01) && (pDdataIn[9]== 01))) return 0;
  pInfo= &pDdataIn[11];
  // search for ':'
  bind = 0;
  memset ((uint8_t*)&RepeaterConfig.GsmConfig,0,sizeof(GSMConfigStruct));
  while ((pInfo[bind]!=0x3a) &&  (pDdataIn[10] >= bind))  // y.f. search for : before port num 
  {
    if (((pInfo[bind] < 0x3a ) && (pInfo[bind] >=0x30 )) || (pInfo[bind] == 0x2e)) // y.f. allowed digits or '.'
    {
      RepeaterConfig.GsmConfig.ServerIP[bind] = pInfo[bind];
      bind++;
    } 
    else
    { 
       sprintf (str, "  \r\n values are not valid meter does not contain valid ip");
       Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode  
       return 1; // y.f ignore params values are not valid meter does not contain valid ip   
    }
  }
  bind++;
  for (uint16_t portind = bind ; portind < pDdataIn[10] ; portind++)
  {
    if (!((pInfo[portind] < 0x3a ) && (pInfo[portind] >=0x30)))  // y.f. allowed digits only
    {
       sprintf (str, "  \r\n values are not valid meter does not contain valid port");
       Send2DebugUart((uint8_t *)str,strlen((const char*)str) ); // return to data mode  
       return 1;  // y.f ignore params  values are not valid meter does not contain valid port
    }
  }
  memcpy (RepeaterConfig.GsmConfig.Port ,&pInfo[bind], (pDdataIn[10]- bind));  
  memcpy (RepeaterConfig.GsmConfig.APN,  &pInfo[pDdataIn[10]+2],pInfo[pDdataIn[10]+1]);
  WriteInternalFlashConfig(); // store the configuration 
#endif  
  return 1;
}
      
uint16_t ExtractMeterNum(uint8_t * pDdataIn)
{
  uint16_t bind,tmpdigit;
  long long address=0;
  char str[50];
  //  verify read response  
  // C4 01 41 00 read response ok 
  if (!((pDdataIn[3]== 0xc4 ) && (pDdataIn[4]== 0x1 ) && (pDdataIn[6]== 0x0 ))) return 0;
  uint8_t addlen =0;
  for (bind = 0; bind < pDdataIn[8] ; bind++)
  {
    address= address*10;  
    tmpdigit = pDdataIn[9+bind] -0x30;
    if (tmpdigit < 10) // y.f 11/1/2022 drop Preliminary letters
    {
      addlen++;
      address+=tmpdigit;
    }
  }
  RepeaterConfig.AddressLen = addlen; // y.f. 21/1/22
  RepeaterConfig.ExtendedAddress=  address;
  WriteInternalFlashConfig(); // store the configuration 
  sprintf (str, " \r\n meter id read %llu",RepeaterConfig.ExtendedAddress);
  Send2DebugUart((uint8_t *)str,strlen((const char*)str)); // return to data mode  
  return 1;
}

/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : BuildLongiMeterMsg                                           */
/* - DESCRIPTION     : this routine takes wrapper message recived from server and   */
/*                     builds local service frame message towards meter             */
/* - INPUT           : pDatain. Len of string                                       */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 						    */
/* - CREATION     06/06/21      						    */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t BuildLongiMeterMsg(uint8_t * pDatain, uint16_t Len)
{

   LongiframeStruct *pMsg;
   pMsg = (LongiframeStruct*)tmpdataout;
   pMsg->Stx = 0x7e;
   pMsg->Len = Len +4;
   pMsg->Frametype=0x10;
   memcpy ((unsigned char *)pMsg->info, (unsigned char *) pDatain,Len); // Copy info  
   dlmscrc = DLMSCrcCalc((unsigned char*)&pMsg->Frametype,  pMsg->Len-2);
   tmpdataout[ pMsg->Len-1]= (dlmscrc >> 8) & 0xff;
   tmpdataout[ pMsg->Len]= dlmscrc & 0xff;
   tmpdataout[ pMsg->Len+1] = 0x7e;
   SendData2Meter (tmpdataout,pMsg->Len+2,1);
   return 1; 
}





/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : ParseLongiMeterMsg                                           */
/* - DESCRIPTION     : this routine parse a longi dlms HDLC PROFILE   message       */
/* - INPUT           : pDatain. Len of string                                       */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 						    */
/* - CREATION     06/06/21      						    */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t ParseLongiMeterMsg(uint8_t * pDatain, uint16_t Len, uint8_t * pDataOut, uint16_t *DataOutLen)
{
   uint16_t bInd;
   LongiframeStruct *pMsg;
   uint16_t dlmscrc;
   char str[100];
  // uint16_t msgcrcHigh,msgcrcLow;
  uint16_t Msgsize=0;
  sprintf (str, "STart routine \r\n");
  Send2DebugUart((uint8_t *)str,16); // return to data mode  
  for (bInd = 0 ; bInd < Len; bInd++)
  { // search for STX 
 
    if (pDatain[bInd] == 0x7e) break;
  }
  if(bInd>=Len) 
  {
	  sprintf (str, "STX not found\r\n");
	  Send2DebugUart((uint8_t *)str,16); // return to data mode  
	  return 0; // STX not found;
  }
  pMsg = (LongiframeStruct*)(pDatain+bInd);
  switch (pMsg->Frametype & 0xf0) 
  {
	case LOCAL_SERVICE:
	  // if (((pMsg->Frametype) & 0xf0) == LOCAL_SERVICE) // local service
		Msgsize = (((pMsg->Frametype)&0x7) << 4)+pMsg->Len;
		*DataOutLen = Msgsize+2;
		sprintf (str, "local service found\r\n");
		Send2DebugUart((uint8_t *)str,21); // return to data mode  
		memcpy ((unsigned char *)pDataOut, (unsigned char *) pMsg,Msgsize+2);
		sprintf (str, "copy to dataout\r\n");
		Send2DebugUart((uint8_t *)str,17); // return to data mode  
		
		if (pDataOut[Msgsize+1] != 0x7e) 
		{	
			sprintf (str, "etx not found \r\n");
			Send2DebugUart((uint8_t *)str,16); // return to data mode  
			return 0;
		}
		sprintf (str, "etx found wait for crc\r\n");
		Send2DebugUart((uint8_t *)str,16); // return to data mode  

		 dlmscrc = DLMSCrcCalc((unsigned char*)&pMsg->Frametype, Msgsize-2);
		tmpcrc = pDataOut[Msgsize-1]*0x100  + pDataOut[Msgsize];
		if (tmpcrc == dlmscrc) 
		{
			sprintf (str, "crc ok\r\n");
			Send2DebugUart((uint8_t *)str,9); // return to data mode  
			return LOCAL_SERVICE_MSG;
		}
		else
		{
			sprintf (str, "crc err \r\n");
			Send2DebugUart((uint8_t *)str,10); // return to data mode  
			return (0);
		}
	  break;
	case WRAPPER_APDU:
			Msgsize = (((pMsg->Frametype)&0x7) << 4)+pMsg->Len;
			*DataOutLen = Msgsize-4;
			memcpy ((unsigned char *)pDataOut, (unsigned char *) &pMsg->info,Msgsize-4);
			return WRAPPER_MSG;
			// no crc in wrapper
		break;
#ifdef LONGI_PUSH			
	case PUSH_APDU:
			Msgsize = (((pMsg->Frametype)&0x7) << 4)+pMsg->Len;
			*DataOutLen = Msgsize-4;
			memcpy ((unsigned char *)pDataOut, (unsigned char *) &pMsg->info,Msgsize-4);
			return EVENT_REPORT;
	break;
#endif	
	
  }
  return 0xff;
 
}
#ifndef LONGI_PUSH
void SendSync_msg(void)
{
    char *str = malloc (1024 *sizeof(char)) ;
    sprintf (str, "sync periodic msg to meter   \r\n");
	ql_rtos_queue_release(DebugUartqueue, sizeof(str), (uint8 *)str, QL_NO_WAIT);
	SendData2Meter((uint8_t*)sync_msg_2_meter,sizeof(sync_msg_2_meter),0); // return to data mode 
	free (str);	
	
}
#endif


                           
                           