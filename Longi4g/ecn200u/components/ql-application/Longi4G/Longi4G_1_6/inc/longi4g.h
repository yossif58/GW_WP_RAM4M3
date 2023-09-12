
#ifndef __MAIN_LONGI_H
#define __MAIN_LONGI_H


#ifdef __cplusplus
extern "C" {
#endif


#include "ql_api_common.h"
#include "ql_app_feature_config.h"
#include "quec_pin_index.h"
#include "ql_gpio.h"


/*========================================================================
 *  Variable Definition
 *========================================================================*/

/*========================================================================
 *  function Definition
 *========================================================================*/
void ql_Longi4G_app_init(void);

#pragma pack(1)

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* User can use this section to tailor SPIx instance used and associated 
   resources */
#define DEBUG_ON  
#define LONGI     
/* Definition for USARTGSM clock resources */

typedef struct
{
  uint16_t      len;
  uint8_t       *rcvbuf;
}GsmQueueStruct;
typedef struct
{
  uint16_t      len;
  uint8_t       *rcvbuf;
}MeterQueueStruct;
 typedef enum
{
  NONE  =0,
  EVEN,
  ODD
}ParitType;


typedef enum
{
  DLT_PROTOCOL  =0,
  DLMS_PROTOCOL
}Protocol_type;
typedef struct
{
  uint32_t      BaudRate;
  ParitType     Parity;
  uint8_t       DataBit;
  uint8_t       StopBit;
}SerialStruct;

typedef struct
{
  char           ServerIP[16];
  char           APN[20];
  char           Port[6];
}GSMConfigStruct;

typedef struct 
{
  Protocol_type           protocol;    // 
  uint8_t                 AddressLen;
  long long               ExtendedAddress;
  
  SerialStruct            MeterSerial; // meter serial is used also for configuration RX "+++\r\n" enters to config mode DLT
                                       // RX "ATO\r\n" enters to meter mode defaulr equ meter mode
  SerialStruct            GSMSerial;
  SerialStruct            DebugSerial; // This serial is for interface to WP card T.B.D
  //uint8_t NewData;
  GSMConfigStruct         GsmConfig;
  uint8_t                 Spare[100];
  uint16_t                Crc;
 
} RepeaterConfigStruct;


typedef enum
{
  METER_DATA_MODE  =0,
  METER_CONFIG_MODE
}MeterPort_type;
typedef enum
{
  TIME_OUT_EVENT  =0,
  DATA_IN_EVENT
}EventType;
typedef enum
{
  RX_From_GSM  =0,
  RX_From_Meter,
  TX_2_METER,
  TX_2_GSM,
  DBG
 
}DebugOriginating;
typedef enum
{
  CONNECTION_2_METER = 0,
  READ_PARAM_FROM_METER,
  CONNECION_2_PROVIDER,
  CONNECTION_2_SERVER,
  SERVER_DATA_MODE,
  DATA_IN_METER,
  DATA_IN_SERVER
 
}LedsStatus;



#endif



