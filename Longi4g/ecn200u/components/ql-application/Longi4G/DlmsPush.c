/************************************************************************************
*																					*
* FILE NAME:																		*
*   DLMSPuse.c																		*
*																					*
* DESCRIPTION:																		*
*   This file contains all routines for the dlms push protocol for longi meters     *
* HISTORY																			*
*   Ferlandes Yossi																	*
* - CREATION      																	*
*   Yossi Ferlandes 04/09/23  								            			*
*                                                                                   *
************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_osi_def.h"
#include "longi4g.h"
#include "socket_demo.h"
#include "LongiWrapper.h"
#pragma pack(1)
#ifdef LONGI_PUSH
/************from longi document expalin push mechanizem*********************

1. module send sync_msg_2_meter.
2. meter response with B3 with value 1 (Data to push or event to push)
7E F0 12 C5 01 41 00 01 04 B1 00 B3 00 B4 00 B5 00 43 77 7E
7E //frame head
F0 12 //frame type is 0x0F local service ,length is 18
C5 01 41 //write return
00 //data
01 //enum type
04 //enum type with four members
B1 //parameter modification coding sign
00 //module parameter with no modification
B3 //meter event or data report coding sign
00 //meter no event or data request initiative report
B4 //module local upgrading coding sigh
00 //no module upgrading file need upgrade
B5 //enable FTP upgrading
00 //
3. once module realize the B3 data set up,then send 2 meter read_push_data_2_meter
4. meter response with push data 2 module. 
(7E 00 20 C4 01 41 00 02 04 16 00 09 00 16 01 09 0E 00 01 00 01 00 01 00 06 C2 00 00 00 00 01 01 F1 7E) 
5. module send to server with destination 0x66 indicating push data.
00 01 00 01 00 66 00 4D 0F 00 00 00 00 0C 07 E7 09 03 07 0A 34 0E FF FF 4C 00 02 04 09 0E 50 43 52 35 34 30 30 30 30 30 30 30 30 30 09 06 00 04 19 09 00 FF 09 0C 07 E7 09 03 07 09 1D 0D FF FF 4C 00 04 81 80 00 00 00 00 00 00 00 00 00 00 13 00 00 00 00 00
6.and report, and report meter the push result at the same time. send push_data_ack.
If data push successfully, and meter still have further data need push, 
then meter reply with the specific push data together with the returned data frame. 
If data push successfully, and meter has no more data need push, then the meter reply with empty data frame, 
and reset the scheduled information interaction B3 to 0. 
if data push failed, then meter repeat the data sending process according to themself rule. 
*******************************************************************/



extern 	ql_timer_t Push_timer ;
typedef enum
{
  SCHEDULE_IDLE_STATE =0,
  SCHEDULE_READ_DATA_STATE,
  SCHEDULE_READ_PUSH_STATE,
  SCHEDULE_SEND_SERVER_PUSH_DATA
 }DLMS_SCHEDULE;

typedef enum
{
  PERIODIC_TIME_OUT =0,
  METER_DATA_EVENT,
 }DLMS_pusheventtype;


typedef enum
{
  NO_PUSH_IN_ACTION =0,
  PUSH_IN_ACTION,
 }DLMS_pushRetVal;
static uint8_t scedule_state = SCHEDULE_IDLE_STATE;
extern void SendData2Meter(uint8_t * DataIn, uint16_t Len,bool display);
const char sync_msg_2_meter[]       =  {  0x7E ,0xF0 ,0xA4 ,0xC1 ,0x01 ,0x41 ,0x00 ,0x01 ,0x00 ,0x81 ,0x19 ,0x00 ,0x8B ,0xFF ,0x02 ,0x00 
										,0x01 ,0x0D ,0xA1 ,0x14 ,0x46 ,0x57 ,0x30 ,0x30 ,0x32 ,0x36 ,0x31 ,0x56 ,0x30 ,0x39 ,0x28 ,0x31 
										,0x34 ,0x30 ,0x35 ,0x30 ,0x35 ,0x31 ,0x35 ,0x29 ,0xA2 ,0x12 ,0x46 ,0x50 ,0x36 ,0x30 ,0x39 ,0x32 
										,0x35 ,0x56 ,0x30,0x35 ,0x52 ,0x32 ,0x30 ,0x34 ,0x4D ,0x30 ,0x30 ,0x32 	,0xA3 ,0x0F ,0x38 ,0x36 
										,0x38 ,0x39 ,0x39 ,0x38 ,0x30 ,0x33 ,0x38 ,0x31 ,0x36 ,0x34 ,0x36 ,0x36 ,0x37 ,0xA4 ,0x12 ,0x45 
										,0x43 ,0x32 ,0x35 ,0x45 ,0x55 ,0x58 ,0x47 ,0x41 ,0x52 ,0x30 ,0x38 ,0x41 ,0x30 ,0x33 ,0x4D ,0x31 
										,0x47 ,0xA5 ,0x14 ,0x38 ,0x39 ,0x38 ,0x36 ,0x30 ,0x31 ,0x31 ,0x38 ,0x38 ,0x30 ,0x31 ,0x35 ,0x31 
										,0x34 ,0x34 ,0x38 ,0x35 ,0x36 ,0x37 ,0x32 ,0xA6 ,0x08 ,0x31 ,0x32 ,0x33 ,0x34 ,0x35	,0x36 ,0x37 
										,0x38 ,0xA7 ,0x12 ,0xA8 ,0x01 ,0x02 ,0x03 ,0x04 ,0xA9 ,0x00 ,0xAA ,0x02 ,0xAB ,0x02 ,0xAC ,0x05 
										,0x43 ,0x4D ,0x4E ,0x45 ,0x54 ,0xAD ,0x0C ,0x43 ,0x48 ,0x49 ,0x4E ,0x41 ,0x20 ,0x4D ,0x4F ,0x42 
										,0x49 ,0x4C ,0x45 ,0x04 ,0x08 ,0x7E};
//const char schedule_resp_from_meter =[] { 0x7E,0xF0,0x12,0xC5,0x01,0x41,0x00,0x01,0x04,0xB1,0x00,0xB3,0x00,0xB4,0x00,0xB5,0x00,0x43,0x77,0x7E}; // only for example 										
const char read_push_data_2_meter[] = {  0x7E ,0x0 ,0x11 ,0xC0 ,0x1 ,0x41 ,0x0 ,0x1 ,0x0 ,0x80 ,0x60 ,0x1 ,0x2 ,0xFF ,0x2 ,0x0
										,0xA2 ,0xA7 ,0x7E};										
const char push_data_ack[]            = { 0x7E ,0x00 ,0x13 ,0xC3 ,0x01 ,0x41 ,0x00 ,0x01 ,0x00 ,0x80 ,0x60 ,0x01 ,0x07 ,0xFF ,0x02 ,0x01 ,0x03 ,0x00 ,0xC1 ,0xEA ,0x7E};
const char event_data_header[]        = {0,1,0,1,0,0x66};
typedef struct 
{
	uint8_t                 Stx;
	uint8_t                 Frametype;    // 
	uint8_t                 Len;  
	uint8_t                 executive_operation[3];
	uint8_t                 data;
	uint8_t				    enum_type;
	uint8_t					enum_four_members;
	uint8_t					B1;
	uint8_t					B1_Result;
	uint8_t					B3;
	uint8_t					B3_Result;  // event/data available
	uint8_t					B4;
	uint8_t				    B4_Result;   //module upgrading file 
	uint8_t					B5;
	uint8_t					B5_Result;  //enable FTP upgrading

 
} LongiScheduleRespStruct;

typedef struct 
{
  uint8_t                 Stx;
  uint8_t                 Frametype;    // 
  uint8_t                 Len;  
  uint8_t                 executive_operation[4];
  uint8_t                 push_info[9];
  uint8_t                 info_len;
  uint8_t                 info[20];
 
} LongiPushRespStruct;

void SendSync_msg(void);
void SendPushAck(void);
bool ParseScheduleResp(uint8_t *pInfo, uint16_t DataLen);
void SendReadPushData(void);
bool ParsePushResp(uint8_t *pDdataIn, uint16_t DataLen, uint8_t *pInfoDataOut, uint16_t *len);
//#define testpush
uint32_t PushHandler(uint8_t *pInfo, uint16_t DataLen , uint8_t EventType)
{
	uint8_t  DataOut[512];
	uint16_t datoutlen;
	
	DLMS_pushRetVal ret = NO_PUSH_IN_ACTION; 
	if (EventType == PERIODIC_TIME_OUT)
	{
		SendSync_msg();
		scedule_state = SCHEDULE_READ_DATA_STATE;
		ql_rtos_timer_start(Push_timer,20000,1);
		return PUSH_IN_ACTION;
	}
	switch (scedule_state)
	{
		case SCHEDULE_IDLE_STATE:
		default:
		// in this case only timeout event is expected
		break;
		case SCHEDULE_READ_DATA_STATE:
			if (ParseScheduleResp(pInfo,DataLen))
			{
				SendReadPushData();
				scedule_state=SCHEDULE_READ_PUSH_STATE;
				ql_rtos_timer_start(Push_timer,20000,1);
				return PUSH_IN_ACTION;
			}
		break;
		case SCHEDULE_READ_PUSH_STATE:
			 if (ParsePushResp(pInfo, DataLen, DataOut,&datoutlen))
			 {
				 // start info string of push was founded 
				SendData2Modem(DataOut,datoutlen);
				SendPushAck();
			 }
		break;
	}
	scedule_state = SCHEDULE_IDLE_STATE;
	ql_rtos_timer_start(Push_timer,20000,1);
	return ret;
}

void SendReadPushData(void)
{
	SendData2Meter((uint8_t*)read_push_data_2_meter,sizeof(read_push_data_2_meter),0); // return to data mode 
}


void SendPushAck(void)
{
	SendData2Meter((uint8_t*)push_data_ack,sizeof(push_data_ack),0); // return to data mode 
}
void SendSync_msg(void)
{
	SendData2Meter((uint8_t*)sync_msg_2_meter,sizeof(sync_msg_2_meter),0); // return to data mode 
}
bool ParseScheduleResp(uint8_t *pInfo, uint16_t DataLen)
{
	LongiScheduleRespStruct *pPushData = (LongiScheduleRespStruct*)pInfo;
	if 	((pPushData->B3_Result)|| (pPushData->B3_Result))
		return true;
	else
		return false;
}


bool ParsePushResp(uint8_t *pDdataIn, uint16_t DataLen, uint8_t *pInfoDataOut,uint16_t *dataoutlen)
{
	
	uint8_t *position = pDdataIn ;
   	uint16_t bInd,bInd_1;
	
	for (bInd = 0 ;  bInd< DataLen ;bInd++)
	{
		for (bInd_1 = 0 ;  bInd_1< 6 ;bInd_1++)
		{
			if (pDdataIn[bInd+bInd_1] != event_data_header[bInd_1]) break;
		}
		if (bInd_1==6)	
		{			
			// string was found 
			uint16_t Msgsize = pDdataIn[bInd+7] + (position[bInd+6] << 8) + 8;
			memcpy (pInfoDataOut,(uint8_t*)&pDdataIn[bInd],Msgsize);
			*dataoutlen   = Msgsize;
		    return true;
		}
	
	}
#ifdef testpush	
	// temp fot test to be removed 	
	
	memcpy (pInfoDataOut,(uint8_t*)&pDdataIn[34],85);
	*dataoutlen   = 85;
	return true;
#endif	


	return false;
}
#endif
