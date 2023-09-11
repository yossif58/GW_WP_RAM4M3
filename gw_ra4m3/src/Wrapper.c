/**********************************************************************************
*																					*
* FILE NAME:																		*
*   ModeBusProtocol.c																			*
*																					*
* DESCRIPTION:																		*
*   This file contains all routines for the DLT 465 protocol                      *
* HISTORY																			*
*   Ferlandes Yossi																	*
* - CREATION      																	*
*   Yossi Ferlandes 19/11/2009														*
*                                                                                  *
***********************************************************************************/
#define LONGI_WARAPPER_H
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "main.h""
#include "hal_data.h"
#include "utils.h"
#include "LongiWrapper.h"
#pragma pack(1)

#define MAX_METERS_IN_LOOK_UP_TABLE 2000   // This value should be fine tuned in future
long long MeterLookUpTable[MAX_METERS_IN_LOOK_UP_TABLE];

extern RepeaterConfigStruct RepeaterConfig;
extern uint8_t ProtocolMode;
extern uint16_t DLMSCrcCalc(uint8_t *crcReg, uint16_t len); 
uint16_t DLMSCrcCalc_PCR160D(uint8_t *cp, int16_t len);
const char LongiWrapperHeader[]          = {0,1,0,1,0,1 } ; 
uint32_t SendLogin2Server(void );
uint32_t SendHeartBit2Server(void );
uint32_t SendWrapper2Server(uint8_t *pMsgIn, uint16_t len);
extern uint32_t BuildLongiMeterMsg(uint8_t * pDatain, uint16_t Len);
#ifdef DEBUG_UART_MODE4
char bufout[512];
#endif


/************************** local function defenitions *******************************/
//uint32_t ExtractWrapper(uint8_t *pMsgIn, uint16_t len);

/*************************************************************************************/
extern uint16_t UartMETERRxInd;
extern uint32_t ParseDlmsMsg(uint8_t * pDatain, uint16_t Len,uint16_t dstadd, uint8_t * pDataOut, uint16_t *DataOutLen );

static uint16_t MeterLookUpTableIndex;
long long MeterLookUpTable[MAX_METERS_IN_LOOK_UP_TABLE];

// y.f. 28/12/22 here we will put all routnes deals with coordinator connection to DCU in dlms wrapper methoud

/************************************************************************************/
/*                                                                                  */
/* - FUNCTION        : ExtractWrapper                                               */
/* - DESCRIPTION     : this routine is called whenever a message is received from   */
/*                    Gprs/Tcp towards meter/dcu. This Routine Extract Wrapper      */
/*                    save it for response and send the apdu to meter/dcu           */
/* - INPUT           : Len - size of input data                                     */
/*                     pDataIn - pointer to input data buf                          */
/* - OUTPUT          : none.                                                        */
/* - CHANGES         :                                          */
/* - CREATION     24.01.2020                                */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t ExtractWrapper(uint8_t *pMsgIn, uint16_t len)
{
    uint16_t dlmscrc,msgcrc,headerlen;
    WrapperStruct * pWrapper = (WrapperStruct*)pMsgIn;
    WrapperGWStruct *pGW_Wrapper = (WrapperGWStruct*)pMsgIn;
    if (pWrapper->Header.Version!=0x100) return 0; // not proper message
    if ((pWrapper->Apdu[0] == LOGIN_RESPONSE) || (pWrapper->Apdu[0] == HEARTBIT_RESPONSE))
    {
        headerlen = (pWrapper->Header.Len[0] << 8 )+ pWrapper->Header.Len[1];
       dlmscrc = DLMSCrcCalc((unsigned char*)&pWrapper->Apdu[1], headerlen -3);
       msgcrc = ((uint16_t)pWrapper->Apdu[headerlen-2]<< 8 )+(uint16_t)pWrapper->Apdu[headerlen-1];
       if (dlmscrc!=msgcrc ) return 0;
       else return (pWrapper->Apdu[0]);
    }
    else
    {
        if (pGW_Wrapper->DlmsGwPrefix== 0xe6)
             return DLMS_MSG_2_METER;
        else return DLMS_MSG_2_GW;
    }

}


void HandleRXRFGwWrapperFromMeter(uint8_t * pDatain,uint16_t Len)
{
#if 0
/* y.f. 29/12/22 sample of AARE Response from meter number 0159000123456 msg rx from mrter send to DCU
1.   RX from Meter
   -------------
  
00 01 00 01 00 11 00 2B 
61 29 A1 09 06 07 60 85 74 05 08 01 01 A2 03 02 01
00 A3 05 A1 03 02 01 00 BE 10 04 0E 08 00 06 5F 1F 04 00 00 18 1D 00 DD 00 07  


  
2.   TX to DCU
   ---------
00 01 00 01 00 11 00 3B E7 00 0D 30
31 35 39 30 30 30 31 32 33 34 35 36 61 29 A1 09 06 07 60 85 74 05 08 01 01 A2 03 02 01
00 A3 05 A1 03 02 01 00 BE 10 04 0E 08 00 06 5F 1F 04 00 00 18 1D 00 DD 00 07
  
  
  
00 01 00 01 00 11 00 3B   // header : vesrion 2 bytes src 2 bytes dst 2 byes data len 2 bytes (0x48)
E7                        // GW TOWARDS DCU DLMS PREFIX  
00 0D                     // METER ADD LEN   
30 31 35 39 30 30 30 31 32 33 34 35 36                                          // ascii meter num
61 29 A1 09 06 07 60 85 74 05 08 01 01 A2 03 02 01
00 A3 05 A1 03 02 01 00 BE 10 04 0E 08 00 06 5F 1F 04 00 00 18 1D 00 DD 00 07   // apdu
  

  --------------------
search in MeterLookUpTable[0159000123456]   the rf address  23456  
  **************/
  
  
#endif

                                    ToggleLeds();
    SendData2Modem (pDatain, Len);

#ifdef DEBUG_UART_MODE4

    memcpy(&bufout,"send to server",14);
    memcpy(&bufout[15],pDatain ,Len);
    bufout[15+Len] = 0xd;
    bufout[16+Len] = 0xa;
    R_SCI_UART_Write(&Debug_Uart1_ctrl,bufout ,16+Len);
#endif
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
  WrapperOut.Header.Version=0x100;
  WrapperOut.Header.Source_wPort    = 0x100;
  WrapperOut.Header.Destination_wPor= 0x100;
  WrapperOut.Header.Len[1] = 0x11;
  WrapperOut.Header.Len[0] = 0x0;
  WrapperOut.ControlCode=0xc;
  WrapperOut.DeviceType=1;  // DCU/GW
  WrapperOut.AddLen=0xc;
  long long address =  RepeaterConfig.ExtendedAddress; //ExtendedAddress;
  for(bInd=0;bInd < 12 ;bInd++)
  {
     WrapperOut.Add[11-bInd] = (address % 10) +0x30; 
     address /=10;
  }
  crc= DLMSCrcCalc((unsigned char*)&WrapperOut.DeviceType, WrapperOut.AddLen +2);
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
/* - CHANGES         :                 						    */
/* - CREATION     31.07.2022     						    */
/*   Ferlandes Yossi                                                                */
/*                                                                                  */
/************************************************************************************/
uint32_t SendLogin2Server(void )
{
  uint8_t bInd;
  uint16_t crc;
  WrapperLoginStruct WrapperOut;
  WrapperOut.Header.Version=0x100;
  WrapperOut.Header.Source_wPort    = 0x100;
  WrapperOut.Header.Destination_wPor= 0x100;
  WrapperOut.Header.Len[1] = 0x12;
  WrapperOut.Header.Len[0] = 0x0;

  WrapperOut.ControlCode=0xa;
  WrapperOut.DeviceType=1; // means dcu/gw
  WrapperOut.AddLen=0xc;
  long long address =  RepeaterConfig.ExtendedAddress; //ExtendedAddress;
  for(bInd=0;bInd < 12 ;bInd++)
  {
     WrapperOut.Add[11-bInd] = (address % 10) +0x30; 
     address /=10;
  }
  WrapperOut.OptionByte=0;
  crc = DLMSCrcCalc((unsigned char*)&WrapperOut.DeviceType, WrapperOut.AddLen +3);
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
  WrapperOut.Header.Len[0] =            len>>8;
  WrapperOut.Header.Len[1] =            len & 0xff     ;
  memcpy ((uint8_t*)&WrapperOut, pMsgIn, len);
  SendData2Modem(&WrapperOut,len);

#ifdef DEBUG_UART_MODE4
            R_BSP_SoftwareDelay(1000, 1000); // delay 1 sec temp to be removed
            memcpy(&bufout,"send to server ",14);
            memcpy(&bufout[15],(uint8_t*)&WrapperOut ,len);
            bufout[14+len] = 0xd;
            bufout[15+len] = 0xa;
            R_SCI_UART_Write(&Debug_Uart1_ctrl,bufout ,15+len);
            R_BSP_SoftwareDelay(1000, 1000); // delay 1 sec temp to be removed

#endif
  return 1;

}


