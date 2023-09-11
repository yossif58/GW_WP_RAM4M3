/**
  ******************************************************************************
  * @file    SPI/SPI_FullDuplex_ComIT/Inc/main.h
  * @author  MCD Application Team
  * @brief   Header for main.c module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
//#include "stm32f3_discovery.h"
#include "stdio.h"
#include "stdint.h"

#include <stdbool.h>
#pragma pack(1)

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Defines related to Clock configuration */
/* Uncomment to enable the adaquate Clock Source */

//#define EC200N temp remove for first test that was with ecn200s the old version
#define PROGRAME_BY_E2STUDIO
#define MAX_LOGGER_DAYS 15
#define MAX_DAYS_INBLOCK 15
//#define DEBUG_UART_MODE
#define DEBUG_UART_MODE3
#define DEBUG_UART_MODE4
#define MAX_UART_SIZE                    255
/* Size of Trasmission buffer */
//#define TXUARTBUFFERSIZE                    MAX_UART_SIZE
/* Size of Reception buffer */
//#define RXUARTBUFFERSIZE                    MAX_UART_SIZE
#define RESET_VALUE  0
#define MAGNET_DETECT      0x80
#define LOW_BATTERY_DETECT 0x40
#define TAMPER_DETECT      0x20
typedef enum
{
  NONE  =0,
  EVEN,
  ODD
}ParitType;

//#define CurrentFirmwareVersion 101


typedef enum
{
  MODEM_START_SEQUENCE =0,
  MODEM_STOP_SEQUENCE,
  MODEM_CONT_SEQUENCE
}ModemModeSequence;

typedef struct
{
  
  
  uint8_t Year;     /*!< Specifies the RTC Date Year.
                         This parameter must be a number between Min_Data = 0 and Max_Data = 99 */
  
  uint8_t Month;    /*!< Specifies the RTC Date Month (in BCD format).
                         This parameter can be a value of @ref RTC_Month_Date_Definitions */

  uint8_t Date;     /*!< Specifies the RTC Date.
                         This parameter must be a number between Min_Data = 1 and Max_Data = 31 */

  uint8_t Hours;            /*!< Specifies the RTC Time Hour.
                                 This parameter must be a number between Min_Data = 0 and Max_Data = 12 if the RTC_HourFormat_12 is selected.
                                 This parameter must be a number between Min_Data = 0 and Max_Data = 23 if the RTC_HourFormat_24 is selected */

   uint8_t Minutes;          /*!< Specifies the RTC Time Minutes.
                                 This parameter must be a number between Min_Data = 0 and Max_Data = 59 */

  uint8_t Seconds;          /*!< Specifies the RTC Time Seconds.*/
}TimeStruct;
  
typedef enum
{
  DLT_PROTOCOL  =0,
  DLMS_PROTOCOL
}Protocol_type;

typedef struct
{
 uint8_t Hour;
 uint8_t Min;
 uint8_t Sec;
}DailyUploadTimeStruct;
typedef enum
{
  CONFIG_DATA  =0,
  LOGGER_DATA
}WriteInternal_type;

typedef struct
{
  char           ServerIP[16];
  char           APN[20];
  uint8_t        Port[6];
}GSMConfigStruct;

typedef struct
{
  uint8_t       ServerAck;         // 0 = No Ack needed      
  uint8_t       MeterFactor;       // 0 = 0.1 Liter, 1 = 1 Liter , 2 = 10 Liter, 3 = 100 Liter , 4 = 1000 Liter 
  uint16_t      PushIntervalTime;  // In minutes Once a day = 60*24 = 1440  
}WaterMeterConfigStruct;

typedef struct
{
  long long   TotalConsumption;
  uint32_t    HoursConsumption[24];
  uint16_t    TmpHourConsmption;

}WaterMeterConsumtionStruct;

typedef struct
{
  uint32_t      BaudRate;
  ParitType     Parity;
  uint8_t       DataBit;
  uint8_t       StopBit;
}SerialStruct;

#if 0
typedef struct  {

  int8_t       Year;
  int8_t       Month;
  int8_t       Day;
  uint8_t      PF;
  int32_t      TotalConsumption;
  uint16_t     HourConsumption[24]; // y.f. 2 bytes per hour 16/8/2022
  int32_t      Pressure;
  int32_t      PrepaymentBalance;
  int8_t       EventRecord[2];
}DailyData;
#endif
typedef struct 
{
  Protocol_type           protocol;    // 
  long long               ExtendedAddress;
  long long               Moduladdress;
  uint8_t                 DeviceInitalizaion;
  SerialStruct            MeterSerial; // meter serial is used also for configuration RX "+++\r\n" enters to config mode DLT
                                       // RX "ATO\r\n" enters to meter mode defaulr equ meter mode
  SerialStruct            GSMSerial;
  SerialStruct            DebugSerial; // This serial is for interface to WP card T.B.D
  //uint8_t NewData;
  GSMConfigStruct            GsmConfig;
  int8_t                     Spare[20];
  uint16_t                   Crc;
 // int8_t                     Spare1[64];
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


/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */




/* Size of buffer */
#define SPIBUFFERSIZE                       (COUNTOF(aSPITxBuffer) - 1)

/* Exported macro ------------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
