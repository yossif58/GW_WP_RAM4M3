/***********************************************************************************
*																					*
* FILE NAME:																		*
*   ModeBusProtocol.c																*
*																					*
* DESCRIPTION:																		*
*   This file contains all routines for the longi dlms interface                    *
* HISTORY																			*
*   Ferlandes Yossi																	*
* - CREATION      																	*
*   Yossi Ferlandes 2/5/23				    										*
*                                                                                   *
************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "utils.h"
#include "ql_api_osi.h"
#include "longi4g.h"
#define LONGI_WARAPPER_H
#include "LongiWrapper.h"
#include "socket_demo.h"
#pragma pack(1)
#ifdef LONGI_PUSH
extern ql_timer_t Push_timer ;
extern bool Push_timer_event;
#endif

extern uint16_t send_meter_timeout_cntr;
extern void Send2DebugUart(uint8_t *DataIn, uint16_t Len);
const char LongiWrapperHeader[]          = {0,1,0,1,0,1 } ; 
uint32_t sendlastgasp(void );
/************************** local function defenitions *******************************/
/*************************************************************************************/
extern uint32_t BuildLongiMeterMsg(uint8_t * pDatain, uint16_t Len);
extern void SendData2Debug(uint8_t * DataIn, uint16_t Len,DebugOriginating debugstr);
extern uint16_t DLMSCrcCalc(uint8_t *crcReg, uint16_t len); 
//extern void SendData2Modem(uint8_t * DataIn, uint16_t Len);
extern RepeaterConfigStruct RepeaterConfig;
extern uint32_t ParseDlmsMsg(uint8_t * pDatain, uint16_t Len,uint16_t dstadd, uint8_t * pDataOut, uint16_t *DataOutLen );
//extern void SendData2Meter(uint8_t * DataIn, uint16_t Len);
const char  aare_temp [] = {0, 1 ,0 ,1 ,0,0x30,0, 0x2B, 0x61 ,0x29 ,0xA1 ,0x9 ,0x6 ,0x7 ,0x60 ,0x85 ,0x74 ,0x5 ,0x8 ,0x1 ,0x1 ,0xA2 ,0x3 ,0x2 ,0x1 ,0x0 ,0xA3 ,0x5 ,0xA1 ,0x3 ,0x2 ,0x1 ,0x0, 0xBE ,0x10 ,0x4 ,0xE ,0x8 ,0x0,0x6 ,0x5F ,0x1F ,4 ,0 ,0 ,0x18 ,0x1D ,0 ,0xC8 ,0 ,7};

/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : ExtractWrapper                                               */
/* - DESCRIPTION     : this routine is called whenever a message is received from   */
/*                    Gprs/Tcp towards meter/dcu. This Routine Extract Wrapper      */
/*                    save it for response and send the apdu to meter/dcu           */
/* - INPUT           : Len - size of input data                                     */
/*                     pDataIn - pointer to input data buf                          */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 			                    			    */
/* - CREATION     24.01.2020     						                            */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t ExtractWrapper(uint8_t *pMsgIn, uint16_t len)
{
    uint16_t dlmscrc;
	uint16_t Headerlen;
	uint16_t *pCRC;
	char str[100];
	WrapperStruct * pWrapper = (WrapperStruct*)pMsgIn;
	Headerlen = (uint16_t)((pWrapper->Header.Len[0])<<8 )+ pWrapper->Header.Len[1];
    if ((pWrapper->Apdu[0] == LOGIN_RESPONSE) || (pWrapper->Apdu[0] == HEARTBIT_RESPONSE))
    {
       dlmscrc = DLMSCrcCalc((unsigned char*)&pWrapper->Apdu[1],Headerlen -3);
	   dlmscrc = ( dlmscrc >> 8)+ ((dlmscrc &0xff) << 8); // msb first 
	   pCRC = (uint16_t*)&pWrapper->Apdu[Headerlen-2];
 //        if (dlmscrc!=(uint16_t)(uint16_t*)&pWrapper->Apdu[Headerlen-2]) return 0;
 
           if (dlmscrc!=*pCRC)
		   {
			sprintf(str,"\r\ncrc err expected %d received %d ",dlmscrc, *pCRC);
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 		
			return 0;
		   }   
       else return (pWrapper->Apdu[0]);
    }
    else  if ((pWrapper->Header.Version==0x100) && (pWrapper->Header.Destination_wPor==0x100)) 
    {
#ifdef 	LONGI_PUSH
		ql_rtos_timer_start(Push_timer,20000,1); // 06/09/23 y.f. restartst meter push timer
#else
		send_meter_timeout_cntr =0;     // y.f. restartst meter periodic msg counter 
#endif	
      // send transparent no crc in msg to meter with local service type msg
        BuildLongiMeterMsg(pMsgIn,len);
        return DLMS_MSG;
    }
	return NOT_RCOGNAIZE_MSG;
}

/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : SendHeartBit2Server                                          */
/* - DESCRIPTION     : this routine build the hearbit wrapper message and send it   */
/* - INPUT           : None                                                         */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 						    */
/* - CREATION     31.07.2022     						    */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t SendHeartBit2Server(void )
{
  uint8_t bInd;
  uint16_t crc;
  WrapperHeartBitStruct WrapperOut;
  WrapperOut.ControlCode=0xc;
  WrapperOut.DeviceType=2;
  long long address =  RepeaterConfig.ExtendedAddress; //ExtendedAddress;
#if addlen12   
  for(bInd=0;bInd < 12 ;bInd++)
  {
     WrapperOut.Add[11-bInd] = (uint8_t)((address % 10) +0x30); 
     address /=10;
  }
  WrapperOut.AddLen =0xc; 
#else
 WrapperOut.AddLen = RepeaterConfig.AddressLen;	
 for(bInd=0;bInd < WrapperOut.AddLen ;bInd++)
  {
     WrapperOut.Add[WrapperOut.AddLen-1-bInd] = (uint8_t)((address % 10) +0x30); 
     address /=10;
  }	
#endif
  crc= DLMSCrcCalc((uint8_t *)&WrapperOut.DeviceType, (uint16_t)WrapperOut.AddLen +2);
  WrapperOut.Crc = ( crc >> 8)+ ((crc &0xff) << 8);
  SendWrapper2Server((uint8_t*)&WrapperOut,sizeof(WrapperHeartBitStruct)+WrapperOut.AddLen-12,0x100);
  return 1;
}

/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : SendLogin2Server                                             */
/* - DESCRIPTION     : this routine build the login wrapper message and send it     */
/* - INPUT           : None                                                         */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 				                    		    */
/* - CREATION     31.07.2022     					                        	    */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t SendLogin2Server(void )
{
  uint8_t bInd;
  uint16_t crc;
  WrapperLoginStruct WrapperOut;
  WrapperOut.ControlCode=0xa;
  WrapperOut.DeviceType=2;
  
   long long address =  RepeaterConfig.ExtendedAddress; //ExtendedAddress;
#if addlen12   
  for(bInd=0;bInd < 12 ;bInd++)
  {
     WrapperOut.Add[11-bInd] = (uint8_t)((address % 10) +0x30); 
     address /=10;
  }
  WrapperOut.AddLen =0xc; 
#else
 WrapperOut.AddLen = RepeaterConfig.AddressLen;	
 for(bInd=0;bInd < WrapperOut.AddLen ;bInd++)
  {
     WrapperOut.Add[WrapperOut.AddLen-1-bInd] = (uint8_t)((address % 10) +0x30); 
     address /=10;
  }	
#endif	
  WrapperOut.OptionByte=0;
  crc = DLMSCrcCalc((unsigned char*)&WrapperOut.DeviceType,(uint16_t)WrapperOut.AddLen +3);
  WrapperOut.Crc = ( crc >> 8)+ ((crc &0xff) << 8);
  SendWrapper2Server((uint8_t*)&WrapperOut,sizeof(WrapperLoginStruct)+WrapperOut.AddLen-12,0x100);
  return 1;
}
/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : SendWrapper2Server                                           */
/* - DESCRIPTION     : this routine build the wrapper message and send it to Server */
/*                     Data to modem.                                               */
/* - INPUT           : Len - size of input data                                     */
/*                     pDataIn - pointer to input data buf                          */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 						    */
/* - CREATION     24.01.2020     						    */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t SendWrapper2Server(uint8_t *pMsgIn, uint16_t len,uint16_t dst)
{
  WrapperStruct WrapperOut;
  WrapperOut.Header.Version  =          0x100;   // WrapperIn.Version;
  WrapperOut.Header.Destination_wPor  =  dst;  //WrapperIn.Source_wPort;     // swap between src &dst 
  WrapperOut.Header.Source_wPort  =     0x100;  // WrapperIn.Destination_wPor; // swap betrween src &dst 
#if 0  
  WrapperOut.Header.Len[0] =         (uint8_t) (len & 0xff) ;
  WrapperOut.Header.Len[1] =         (uint8_t) ((len & 0xff00)>> 8) ;   
#endif
  WrapperOut.Header.Len[1] =         (uint8_t) (len & 0xff) ;  // y.f. 21/06/23 change to msb first
  WrapperOut.Header.Len[0] =         (uint8_t) ((len & 0xff00)>> 8) ;   

  memcpy ((uint8_t*)&WrapperOut.Apdu, pMsgIn, len);
  SendData2Modem((uint8_t*)&WrapperOut, len + sizeof(WrapperHeaderStruct));
  return 1;
}
/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : sendaaretemp                                                 */
/* - DESCRIPTION     : this routine build the wrapper message and send it to Server */
/*                     Data to modem.                                               */
/* - INPUT           : Len - size of input data                                     */
/*                     pDataIn - pointer to input data buf                          */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 				                    		    */
/* - CREATION     4.07.2023     				                        		    */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t sendaaretemp(void)
{
	SendData2Modem((uint8_t*)&aare_temp, sizeof(aare_temp));
	return 1;
}



/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : sendlastgasp                                                 */
/* - DESCRIPTION     : this routine send last gasp to server indicates power off    */
/* - INPUT           : None                                                         */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                 						                        */
/* - CREATION     17.07.2023     						                            */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t sendlastgasp(void )
{
  uint8_t bInd;
  uint16_t crc;
  char str[30];
	sprintf(str,"\r\n last GASP INTERRUPT");
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 		
 // return 1;
  WrapperHeartBitStruct WrapperOut;
  WrapperOut.ControlCode=0xf;
  WrapperOut.DeviceType=2;
  long long address =  RepeaterConfig.ExtendedAddress; //ExtendedAddress;

  WrapperOut.AddLen = RepeaterConfig.AddressLen;	
  for(bInd=0;bInd < WrapperOut.AddLen ;bInd++)
  {
     WrapperOut.Add[WrapperOut.AddLen-1-bInd] = (uint8_t)((address % 10) +0x30); 
     address /=10;
  }	
  crc= DLMSCrcCalc((uint8_t *)&WrapperOut.DeviceType, (uint16_t)WrapperOut.AddLen +2);
  WrapperOut.Crc = ( crc >> 8)+ ((crc &0xff) << 8);
  SendWrapper2Server((uint8_t*)&WrapperOut,sizeof(WrapperHeartBitStruct)+WrapperOut.AddLen-12,0x100);
  return 1;
}
