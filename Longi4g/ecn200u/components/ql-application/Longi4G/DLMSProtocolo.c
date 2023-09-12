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
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "ql_api_osi.h"
#include "longi4g.h"
#include "longi_nor_flash.h"

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



uint32_t ParseLongiMeterMsg(uint8_t * pDatain, uint16_t Len, uint8_t * pDataOut, uint16_t *DataOutLen);
uint32_t BuildLongiMeterMsg(uint8_t * pDatain, uint16_t Len);

void SendReadMeter2Meter(void);

bool DLMS_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
void DLMS_ReadMeterId(void);
void Senddisconnect_msg(void);
void SendReadParam_msg(void);
bool CheckDLMSState(void);
uint16_t ExtractMeterNum(uint8_t *pDdataIn);
uint16_t ExtractServerParams(uint8_t *DataOut);
extern uint16_t DLMSCrcCalc(uint8_t *crcReg, uint16_t len); 
extern void SendData2Meter(uint8_t * DataIn, uint16_t Len);
extern RepeaterConfigStruct RepeaterConfig;
extern uint32_t SendWrapper2Server(uint8_t *pMsgIn, uint16_t len);
extern uint16_t DLMSCrcCalc(uint8_t *crcReg, uint16_t len); 

uint8_t meter_bufferTransmit[1024];
uint16_t dlmscrc = 0;
uint16_t tmpcrc =0;
const char readparam_msg  [] =        {0x7e,0xf0,0x13,0xc0,0x41,1,0,1,0,0x81,0x19,0,0x89,0xFF,2,0,0x16,0x1,0,5,0x7E};
const char readmeterid_msg [] =       {0x7e,0xf0,0x11,0xc0,1,0x41,0,1,0,0,0x60,1,0,0xFF,2,0,0x33,0x53,0x7E};
const char disconnect_msg [] =        {0x7E, 0xA0 ,0x7,0x3 ,0x41 ,0x53 ,0x56 ,0xa2 ,0x7E};
const char disconnect_res_msg [] =    {0x7E, 0xA0 ,0x7,0x41 ,0x3 ,0x1f ,0x26 ,0xec, 0x7E};

typedef enum
{
  DLMS_IDLE_STATE =0,
  DLMS_SNRM_STATE,
  DLMS_AARQ_STATE,
  DLMS_READ_METER_ID_STATE,
  DLMS_READ_PARAMETERS_STATE,
  DLMS_DATA_STATE,
 }DLMState;


typedef enum
{
  DST_METER = 3,
  DST_CLIENT = 0x41
}DstAddress;
const char METER_READ_RES[]   = {0xc4,1,0xc1,0}; // 11/1/2022 proper read reasponse expected
uint32_t ParseDlmsMsg(uint8_t * pDatain, uint16_t Len, DstAddress dst,uint8_t * pDatOut, uint16_t *DataOutLen);

uint8_t  *pDataOut;
uint16_t *DataoutLen;
long long  bb_tembel=0;
uint8_t tmpdataout[200];
DLMState DLMS_Protocol_State  ;
bool CheckDLMSState(void)
{
  if (DLMS_Protocol_State!=DLMS_DATA_STATE) return 1;
  else
    return 0;
}

bool DLMS_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event)
{
  static uint8_t DataOut[255];
  switch (DLMS_Protocol_State)
  {
  case DLMS_IDLE_STATE:
      break;
  case DLMS_SNRM_STATE:
    if (((bb_tembel++) > 3000000)&& (Event == TIME_OUT_EVENT))
    {
    // retry snrm 
        bb_tembel = 0;
		DLMS_Protocol_State=DLMS_READ_METER_ID_STATE;
		SendReadMeter2Meter();
    }
      break;
  case DLMS_AARQ_STATE:
      break;
  case DLMS_READ_METER_ID_STATE:
    
     if (((bb_tembel++) > 7000000)&& (Event == TIME_OUT_EVENT))
    {
   
        bb_tembel = 0;
        SendReadMeter2Meter(); 
    }
    else
	{
     if (Event == DATA_IN_EVENT)
	 {
	 
      if (ParseLongiMeterMsg(pDdataIn, Len,DataOut,DataoutLen)==LOCAL_SERVICE_MSG)
      {
        // extract meter number from msg
          if (ExtractMeterNum(DataOut ))
          {
            DLMS_Protocol_State = DLMS_READ_PARAMETERS_STATE;
            SendReadParam_msg();
            return true;
		  }
          else
          {
            DLMS_Protocol_State = DLMS_SNRM_STATE;
          }
 
      }
      else
      {
        DLMS_Protocol_State = DLMS_SNRM_STATE;
      }
	 }
	}
      break;
       case DLMS_READ_PARAMETERS_STATE:
    
       if (((bb_tembel++) > 7000000)&& (Event == TIME_OUT_EVENT))
        {
      // retry snrm 
          bb_tembel = 0;
          SendReadParam_msg();
      //    ToggleLed1();
	  
        }
        else if (Event == DATA_IN_EVENT)
        {
            if (ParseLongiMeterMsg(pDdataIn, Len,DataOut,DataoutLen)==LOCAL_SERVICE_MSG)
            {
              if (ExtractServerParams(DataOut))
              {
                DLMS_Protocol_State = DLMS_DATA_STATE;
               return false;
              }
            }
        }
	
  break;
  default:
  case DLMS_DATA_STATE:

      if (ParseLongiMeterMsg(pDdataIn, Len,DataOut,DataoutLen)==WRAPPER_MSG)  
      {
        SendWrapper2Server(DataOut, *DataoutLen); 
      }
      // t.b.d to add push and local service also in data state

      break;
  }
  return true;
}

void DLMS_ReadMeterId(void)
{
  DLMS_Protocol_State=DLMS_READ_METER_ID_STATE;
  SendReadMeter2Meter();
 
}

void SendReadMeter2Meter(void)
{
  memcpy (meter_bufferTransmit,readmeterid_msg,sizeof(readmeterid_msg));
  SendData2Meter(meter_bufferTransmit,sizeof(readmeterid_msg)); // return to data mode 
}
void Senddisconnect_msg(void)
{
  memcpy (meter_bufferTransmit,disconnect_msg,sizeof(disconnect_msg));
  SendData2Meter(meter_bufferTransmit,sizeof(disconnect_msg)); // return to data mode 
}


void SendReadParam_msg(void)
{
   memcpy (meter_bufferTransmit,readparam_msg,sizeof(readparam_msg));
   SendData2Meter(meter_bufferTransmit,sizeof(readparam_msg)); // return to data mode 
 
}       
uint16_t ExtractServerParams(uint8_t * pDdataIn)
{
  uint16_t bind;
  uint8_t *pInfo;
  //  verify read response  
  // C4 01 41 00 read response ok 
  if (!((pDdataIn[3]== 0xc4 ) && (pDdataIn[4]== 0x1 ) && (pDdataIn[6]== 0x0 ) && (pDdataIn[7]== 01) && (pDdataIn[9]== 01))) return 0;
  pInfo= &pDdataIn[11];
  // search for ':'
  bind = 0;
  memset ((uint8_t*)&RepeaterConfig.GsmConfig,0,sizeof(GSMConfigStruct));
  while ((pInfo[bind]!=0x3a) &&  (pDdataIn[10] >= bind))
  {
    RepeaterConfig.GsmConfig.ServerIP[bind] = pInfo[bind];
    bind++;
  }
  bind++;
  memcpy (RepeaterConfig.GsmConfig.Port ,&pInfo[bind], (pDdataIn[10]- bind));  
  memcpy (RepeaterConfig.GsmConfig.APN,  &pInfo[pDdataIn[10]+2],pInfo[pDdataIn[10]+1]);
  WriteInternalFlashConfig(); // store the configuration 
  return 1;
}
      
uint16_t ExtractMeterNum(uint8_t * pDdataIn)
{
  uint16_t bind,tmpdigit;
  long long address=0;
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
   tmpdataout[ pMsg->Len-1]=(uint8_t) dlmscrc >> 8;
   tmpdataout[ pMsg->Len]= dlmscrc & 0xff;
   tmpdataout[ pMsg->Len+1] = 0x7e;
   SendData2Meter (tmpdataout,pMsg->Len+2);
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
  // uint16_t msgcrcHigh,msgcrcLow;
  uint16_t Msgsize=0;
  pMsg = (LongiframeStruct*) pDatain;
  for (bInd = 0 ; bInd < Len; bInd++)
  { // search for STX 
    pMsg = (LongiframeStruct*)(pDatain+bInd);
    if (pMsg->Stx == 0x7e) break;
  }
  if (((pMsg->Frametype) & 0xf0) == 0xf0) // local service
  {
    Msgsize = (((pMsg->Frametype)&0x7) << 4)+pMsg->Len;
    *DataOutLen = Msgsize+2;
    memcpy ((unsigned char *)pDataOut, (unsigned char *) pMsg,Msgsize+2);
    if (pDataOut[Msgsize+1] != 0x7e) return 0;
     dlmscrc = DLMSCrcCalc((unsigned char*)&pMsg->Frametype, Msgsize-2);
    tmpcrc = pDataOut[Msgsize-1]*0x100  + pDataOut[Msgsize];
    if (tmpcrc == dlmscrc) return LOCAL_SERVICE_MSG;
    else return (0);
  }
  else if  (((pMsg->Frametype) & 0xf0) == 0x10) // wrapper msg
  {
    Msgsize = (((pMsg->Frametype)&0x7) << 4)+pMsg->Len;
    *DataOutLen = Msgsize-4;
    memcpy ((unsigned char *)pDataOut, (unsigned char *) &pMsg->info,Msgsize-4);
    return WRAPPER_MSG;
    // no crc in wrapper
  }
    return 0;
 
}



                           
                           