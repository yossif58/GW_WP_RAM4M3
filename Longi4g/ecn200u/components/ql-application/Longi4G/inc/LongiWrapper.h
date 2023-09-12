/****************************************************************************************
* _LongiWrapper_H 																			   	*
* DESCRIPTION:																		   	*
*   This file contains ALL 	longi wrapper protocol packet exported routints and definitions .  
*
*																					   	*
* HISTORY																			 	*
*   Ferlandes Yossi   20/07/2022     Created										  		*
****************************************************************************************/
#ifndef _Longi_Wrapper_H
#define _Longi_Wrapper_H

#undef PUBLIC
#ifdef LONGI_WARAPPER_H
#define  PUBLIC
#else
#define  PUBLIC  extern
#endif

#pragma pack(1)

#define LOGIN_RESPONSE    0xaa
#define HEARTBIT_RESPONSE 0xcc
#define  DLMS_MSG          0x1
#define NOT_RCOGNAIZE_MSG  0

/********** sample login process for LONGI gprs meters
136 2022-07-19 15:54:16.243 SOCKET Add a socket: 192.168.14.254:54847, socket counter: 2 
137 2022-07-19 15:54:16.791 RECEIVE 00 01 00 01 00 01 00 12 0A 02 0C 32 31 30 36 30 30 34 30 32 39 34 31 00 5A 4C 
138 2022-07-19 15:54:16.792 SEND    00 01 00 01 00 01 00 12 AA 03 0C 32 31 30 36 30 30 34 30 32 39 34 31 00 70 04 
139 2022-07-19 15:54:16.793 SOCKET Meter No. 210600402941 is on line! 
140 2022-07-19 15:54:48.020 RECEIVE 00 01 00 01 00 01 00 11 0C 01 0C 32 31 31 32 30 30 35 30 34 33 37 37 40 02 
141 2022-07-19 15:54:48.022 SEND    00 01 00 01 00 01 00 11 CC 03 0C 32 31 31 32 30 30 35 30 34 33 37 37 94 FF 
142 2022-07-19 15:57:22.199 SOCKET Server socket is with session! 
143 2022-07-19 15:57:22.276 SEND    00 01 00 11 00 01 00 47 E6 03 0C 35 34 30 30 30 30 30 30 30 33 34 31 60 36 A1 09 06 07 60 85 74 05 08 01 01 8A 02 07 80 8B 07 60 85 74 05 08 02 01 AC 0A 80 08 31 32 33 34 35 36 37 38 BE 10 04 0E 01 00 00 00 06 5F 1F 04 00 00 7E 1F 04 B0

********/

typedef struct
{
  uint16_t       Version;
  uint16_t       Source_wPort;
  uint16_t       Destination_wPor;
  uint8_t        Len[2];
}WrapperHeaderStruct;

typedef struct
{
  WrapperHeaderStruct       Header;
  uint8_t       Apdu[1024];
}WrapperStruct;
#define MAX_NUMBER_DIGITS  13
typedef struct
{
  WrapperHeaderStruct       Header;
  uint8_t       DlmsGwPrefix;    // Can Be E6/E7
  uint8_t       AddLen[2];
  uint8_t       add[MAX_NUMBER_DIGITS];
  uint8_t       Apdu[1024];
}WrapperGWStruct;


typedef struct
{
  uint8_t  ControlCode;
  uint8_t  DeviceType;
  uint8_t  AddLen;
  uint8_t  Add[12];
  uint8_t  OptionByte;
  uint16_t Crc;
}WrapperLoginStruct;

typedef struct
{
  uint8_t  ControlCode;
  uint8_t  DeviceType;
  uint8_t  AddLen;
  uint8_t  Add[12];
  uint8_t  OptionByte;
  uint16_t Crc;
}WrapperParamStruct;

typedef struct
{
  uint8_t  ControlCode;
  uint8_t  DeviceType;
  uint8_t  AddLen;
  uint8_t  Add[12];
  uint16_t Crc;
}WrapperHeartBitStruct;

typedef enum
{
  NO_ADDRESS = 0,
  DCU_ADDRESS,
  METER_ADDRESS,
  MASTER_STATION_ADDRESS
}Wrapper_NET_address;

uint32_t ExtractWrapper(uint8_t *pMsgIn, uint16_t len);
uint32_t SendLogin2Server(void );
uint32_t SendHeartBit2Server(void );
uint32_t SendWrapper2Server(uint8_t *pMsgIn, uint16_t len,uint16_t dst);
uint32_t sendaaretemp(void);


#endif
