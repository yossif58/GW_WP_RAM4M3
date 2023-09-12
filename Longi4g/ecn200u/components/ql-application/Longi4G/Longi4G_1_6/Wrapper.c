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
#pragma pack(1)


const char LongiWrapperHeader[]          = {0,1,0,1,0,1 } ; 

/************************** local function defenitions *******************************/
uint32_t ExtractWrapper(uint8_t *pMsgIn, uint16_t len);
uint32_t SendLogin2Server(void );
uint32_t SendHeartBit2Server(void );
uint32_t SendWrapper2Server(uint8_t *pMsgIn, uint16_t len);
/*************************************************************************************/
extern uint32_t BuildLongiMeterMsg(uint8_t * pDatain, uint16_t Len);
extern void SendData2Debug(uint8_t * DataIn, uint16_t Len,DebugOriginating debugstr);
extern uint16_t DLMSCrcCalc(uint8_t *crcReg, uint16_t len); 
extern void SendData2Modem(uint8_t * DataIn, uint16_t Len);
extern RepeaterConfigStruct RepeaterConfig;
extern uint32_t ParseDlmsMsg(uint8_t * pDatain, uint16_t Len,uint16_t dstadd, uint8_t * pDataOut, uint16_t *DataOutLen );
extern void SendData2Meter(uint8_t * DataIn, uint16_t Len);
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
	WrapperStruct * pWrapper = (WrapperStruct*)pMsgIn;
	Headerlen = (uint16_t)((pWrapper->Header.Len[0])<<8 )+ pWrapper->Header.Len[1];
    if ((pWrapper->Apdu[0] == LOGIN_RESPONSE) || (pWrapper->Apdu[0] == HEARTBIT_RESPONSE))
    {
       dlmscrc = DLMSCrcCalc((unsigned char*)&pWrapper->Apdu[1],Headerlen -3);
	   pCRC = (uint16_t*)&pWrapper->Apdu[Headerlen-2];
 //        if (dlmscrc!=(uint16_t)(uint16_t*)&pWrapper->Apdu[Headerlen-2]) return 0;
           if (dlmscrc!=*pCRC) return 0;
       else return (pWrapper->Apdu[0]);
    }
 //   else  if ((pWrapper->Header.Version==0x100) && (pWrapper->Header.Destination_wPor==0x100)) 
	else  if (pWrapper->Header.Version==0x100)  // y.f. 27/06/23rsion 
    {
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
  WrapperOut.AddLen=0xc;
  long long address =  RepeaterConfig.ExtendedAddress; //ExtendedAddress;
  for(bInd=0;bInd < 12 ;bInd++)
  {
     WrapperOut.Add[11-bInd] = (address % 10) +0x30; 
     address /=10;
  }
  crc= DLMSCrcCalc((uint8_t *)&WrapperOut.DeviceType, (uint16_t)WrapperOut.AddLen +2);
  WrapperOut.Crc = ( crc >> 8)+ ((crc &0xff) << 8);
  SendWrapper2Server((uint8_t*)&WrapperOut,sizeof(WrapperHeartBitStruct));
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
  WrapperOut.AddLen =0xc; 
   long long address =  RepeaterConfig.ExtendedAddress; //ExtendedAddress;
  for(bInd=0;bInd < 12 ;bInd++)
  {
     WrapperOut.Add[11-bInd] = (uint8_t)((address % 10) +0x30); 
     address /=10;
  }
  WrapperOut.OptionByte=0;
  crc = DLMSCrcCalc((unsigned char*)&WrapperOut.DeviceType,15);
  WrapperOut.Crc = ( crc >> 8)+ ((crc &0xff) << 8);
  SendWrapper2Server((uint8_t*)&WrapperOut,sizeof(WrapperLoginStruct));
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
uint32_t SendWrapper2Server(uint8_t *pMsgIn, uint16_t len)
{
  WrapperStruct WrapperOut;
  WrapperOut.Header.Version  =          0x100;   // WrapperIn.Version;
  WrapperOut.Header.Destination_wPor  = 0x100;  //WrapperIn.Source_wPort;     // swap between src &dst 
  WrapperOut.Header.Source_wPort  =     0x100;  // WrapperIn.Destination_wPor; // swap betrween src &dst 
  WrapperOut.Header.Len[0] =         (uint8_t) (len & 0xff) ;
  WrapperOut.Header.Len[1] =         (uint8_t) ((len & 0xff00)>> 8) ;   
  memcpy ((uint8_t*)&WrapperOut.Apdu, pMsgIn, len);
  SendData2Modem((uint8_t*)&WrapperOut, len + sizeof(WrapperHeaderStruct));
  return 1;
}
