/* Includes ------------------------------------------------------------------*/
#include "hal_data.h"
#include <stdio.h>
#include <string.h>
#include "main.h"
#include <stdbool.h>
//#include "DLTprotocol.h"
//#include "FLASH_Program.h"
#include "hal_data.h"
#include "utils.h"
#include "flash_hp_ep.h"
#include "uart_LTE.h"
#include "LongiWrapper.h"
#pragma pack(1)
//#define DEBUG_UART_MODE1
//	Modem handler states mechine
//===========================================
#ifdef LONGI_GW
extern  uint32_t SendLogin2Server(void );
extern  uint32_t SendHeartBit2Server(void );
#else
extern uint32_t PP_ExtractDlt(uint8_t *pMsgIn);
extern void PP_ReqConnectionFromServer(void);
#endif
extern bool write_internal_config_flag;
extern bool write_internal_logger_flag;
extern bool ToggleLedsReq;
//extern bool MagnetDetect;
extern bool powerup_start;

uint8_t GsmCompleateStatus(void);
void SetGsmCompleate(void);
void ClrGsmCompleate(void);
//void SMS_Handler(void);
uint8_t ConnectRetries=5;
static heartbit_count =0;
enum Connectiobn_State {
  MODEM_DISCONNECT_STATE = 0,   // Init State Communication not established with modem
  MODEM_AT0_STATE,
  MODEM_SET_CONFIGURATION_STATE, // communication estalished with modem
  MODEM_SET_SMS_CONFIGURATION_STATE,
  SIM_READY_STATE,
  MODEM_PROVIDER_STATE,
  MODEM_MAGNET_READ_SMS,
  MODEM_ACTIVATE_CONTEXID_STATE,
  MODEM_ACTIVATE_SET_CONTEXID_STATE,
  MODEM_QIOPEN_STATE,
  MODEM_CONNECT_STATE,          
  MODEM_DATA_STATE
 // SIM_EXIST_STATE,           // sim exist in modem 
 // PROVIDER_CONNECT_STATE,    // established connection with the GSM provider 
 // SERVER_CONNECT_STATE       // Tcp connection with server established
};
enum Response_State {
  NO_RESPONSE = 0,
  WRONG_RESPONSE ,
  GOOD_RESPONSE
};

#define SEND_RETRY_TIMES        10
extern uint32_t GsmTimer;
uint16_t SendRetry = SEND_RETRY_TIMES;
#ifdef DEBUG_UART_MODE3
void PrintSMSError (char *data);
#endif
//uint8_t tmpbufout[100]; // y.f. temp to be removed
void sendfreesms(uint8_t * pDdataIn);
uint16_t  ReadConfigFromSMS(uint8_t *pDdataIn, uint16_t Len);
void ReadSmsMsg(void);
void  Error_UartGSMHandler(void);
void RingHandler(void);
void SendSMSGPRS(void);
void DelSmsMsg(void);
void SendATD(void);
void SendDQIACT_SET2Modem(void);
uint8_t sendsmstest(uint8_t * pDdataIn,bool ack_test);
const char ATE0[]          = { "\r\nATE0\r\n" } ;           // A    stops echo from modem 
const char ATE0_RESP[]     = { "\r\nATE0\r\nOK\r\n" } ;     //    stops echo from modem 
const char SIM_READY[]     = { "\r\nAT+CPIN?\r\n"};   // B
const char SIM_RESP[]         = { "READY"};
const char CGREG[]        = { "\r\nAT+CGREG?\r\n" }; // C
const char CGREG_RESP[]   = { "\r\n+CGREG: 0,1\r\n"}; // 
const char CMGR[] = {"\r\nAT+CMGR=1\r\n"};

//AT+QSMSL
//const char QSMSL[] = {"\r\nAT+QSMSL\r\n"};
const char CMGL[]    = {"\r\nAT+CMGL=REC UNREAD\r\n"};
const char CMGD[]   = {"\r\nAT+CMGD=1,4\r\n"};  // delete all messages after read
//const char TRANSPARENT_MODE[]   = {  "\r\nAT+QISWTMD=2\r\n"}; // switch to transparent mode 
const char CGDCONT[]     = { "AT+CGDCONT=1,IP,internet,32.154.79.218,0,0,0,0,0,0\r\n"}; // D XILIN SERVER 218.78.213.232 4059
const char CGDCONT_HEADER[]     = { "AT+CGDCONT=1,IP,"};
const char CGDCONT_FOOTER[]     = { ",0,0,0,0,0,0\r\n"};
//const char CGDCONT[]     = { "AT+CGDCONT=1,IP,publicip.net,31.154.79.218,0,0,0,0,0,0\r\n"}; // D XILIN SERVER 218.78.213.232 4059
//const char CGDCONT[]     = { "AT+CGDCONT=1,IP,internet,31.154.8.246,0,0,0,0,0,0\r\n"}; // D XILIN SERVER 218.78.213.232 4059
//const char CGDCONT_RESP[]  = { "OK\r\n"};
//const char QIOPEN []     = { "AT+QIOPEN=1,0,TCP,2.55.107.55,4058,0,2\r\n"}; // Open in transparent mode
//const char QIOPEN []     = { "AT+QIOPEN=1,0,TCP,31.154.8.246,2334,0,2\r\n"}; // Open in transparent mode
//const char QIOPEN []     =    { "AT+QIOPEN=1,0,TCP,32.154.79.218,2334,0,2\r\n"}; // Open in transparent mode
const char QIOPEN_HEADER [] = { "AT+QIOPEN=1,0,TCP,"}; // Open in transparent mode
const char QIOPEN_FOOTER [] = { ",0,2\r\n"}; 
//const char QIOPEN []     = { "AT+QIOPEN=1,0,TCP,31.154.79.218,3556,0,2\r\n"}; // dani's private port
const char QIOPEN_RESP[] = { "\r\nCONNECT\r\n"}; // TO BE CHECK 
const char QICLOSE[]     = { "\r\nAT+QICLOSE=1\r\n"};
const char OK[]          = { "\r\nOK\r\n"};  // to be checked again
//const char CGDCONT_RESP[]= { "\r\n+CGDCONT: 1,IP,internet,2.55.107.55,0,0,0,0,0,0\r\n"};
const char CGDCONT_RESP[]= { "\r\n+CGDCONT: 1,IP,internet,32.154.8.246,0,0,0,0,0,0\r\n"};
const char QISTAT[]      = { "\r\nAT+QISTAT?\r\n" };
const char CSQ[]         = { "\r\nAT+CSQ\r\n"  };
const char COMMAND_MODE[]  = {0x2b,0x2b,0x2b}; // +++ command mode 
const char QIACT_SET[]         = {"\r\nAT+QIACT=1\r\n"};
const char DQIACT_SET[]         = {"\r\nAT+QIDEACT=1\r\n"};
const char QIACT_REQ[]         = {"\r\nAT+QIACT?\r\n"};
const char NOCARRIERRESP[]     = {"\r\nNO CARRIER\r\n"};
const char ERRORRESP[]     =     {"\r\nERROR\r\n"};
const char CONNECTION_DLMS_HEADER_COMMAND[]  =     {0x4D,0x4F,0x44,0x45,0x4D,0x5F,0x49,0x44,0x3D,0x51};
const char CONNECTION_DLMS_FOOTER_COMMAND[]  =     {0x2c,0x4D,0x45,0x54,0x45,0x52,0x5f,0x49,0x44,0x3d,0x51};
//const char SERVER_DLMS_HEARTBIT[] =              { 0x7e, 0xA0, 7, 0x41, 3, 0x53, 0x56, 0xa2, 0x7e  }; y.f. 1/5/2022 change danni is send echo message  
const char SERVER_DLMS_HEARTBIT[] =               {  0x7E, 0xA0, 7, 0x41, 3, 0x1F, 0x26, 0xEC, 0x7E  };
const char SERVER_DLMS_ACK_COMMAND[] =            { 0x7E, 0xA0, 7, 0x41, 3, 0x1F, 0x26, 0xEC, 0x7E  };
const char PHON_HEADER[] = {"+CMT:"};
const char QIACT_CHECK_RESP[]         = {"\r\n+QIACT: 1,1,1"};

#ifdef LONGI_GW
const char CONNECTION_HEADER_COMMAND[]     =    {0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x12,0xa,0x2,0xc};
const char SERVER_LOGIN_HEADER_ACK_COMMAND[]      =    {0,1 ,0 ,1, 0, 1, 0, 0x12, 0xaa, 3 ,0xc}; // this message is ack command received for connection and for login commands
const char SERVER_HEARTBIT_HEADER_ACK_COMMAND[]   =    {0,1 ,0 ,1, 0, 1, 0, 0x11, 0xcc, 3 ,0xc}; // this message is ack command received for connection and for login commands

#else
const char CONNECTION_HEADER_COMMAND[]   = {0x23,0x52,0x23}; // this message is the first message after connect 
const char CONNECTION_FOOTER_COMMAND[]   = {0x2C,0x1}; // this message is the first message after connect 
const char HEARTBIT_COMMAND[]     = {0x23,0x24,0x23,0x15}; // this message is the periodic messag to keep socket alive.
const char SERVER_ACK_COMMAND[]   = {0x23,0x24,0x23}; // this message is ack command received for connection and for heartbit commands
#endif

//const char SMS_IN_HEADER[]   = {"+CMGR:"};
const char SMS_IN_HEADER[]   = {"+GSMS:"};
//const char SMS_IN_HEADER[]   = {"UNREAD"};
//const char SMS_IN_HEADER[]   = {"+CMT: "}; // 15/09
const char DATA_MODE[]     = { "\r\nATO\r\n"};
// const char MODEM_POWER_DOWN = { "AT+QPOWD\r\n"}; not to use use cfun=1,1 for reset 
const char DISCONNECT_CALL[]    = { "ATH/r/n"};
const char MODEM_RESET[]        = {"\r\nAT+CFUN=1,1\r\n"};        // y.f  7/3/21
const char SMS_FORMAT[]         = {"\r\nAT+CMGF=1\r\n"};          // y.f. 7/3/21 TEXT MODE
const char SMS_CHARSET[]        = {"\r\nAT+CSCS=GSM\r\n"};       // y.f. 7/3/21
const char SMS_DELIVERY_FORMAT[]=   {"\r\nAT+CNMI= 2,1,0,0,0\r\n"};  // y.f. 7/3/21
const char ATD[]               = {"\r\nAT&D1\r\n"};  // on to off in DTR turns to command mode  
const char SMS_TEST_SEND1[] =      {"\r\nAT+CMGS=+972546262816\r\n"};
const char yossiphon[] = {"+972546262816"};
//const char yossiphon[] =   {"+972522671485"};
const char SMS_TEST_SEND_HEADER[] = {"\r\nAT+CMGS="};
const char SMS_TEST_SEND_FOOTER[] = {"\r"};
const char CONTROL_Z = 0x1a;
const char SMS_RING_INDICATOR[] = {"\r\nAT+QCFG=urc/ri/smsincoming,pulse\r\n"};;

void Gsm_Init(void);
void Gsm_Handler(uint8_t * pDdataIn, uint32_t Len , EventType Event);
void SetSMSconfig(uint8_t smsstate);
void StartOpenConnection(void);
void StopConnection(void);
void SendData(uint8_t * DataIn, uint16_t Len);
// y.f. 7/2/22 no heart bit in water pulser void SendHeartBit(void);
void SendQIOPEN2Modem(void);
bool CheckProvider(void);
//void SendCONNECT2Modem(void);
void SendACK2Modem(void);
void SendQICLOSE2Modem(void);
void SendCOMMAND_MODE2Modem(void);
void SendDLMSACK2Modem(void);
void SendCGREG2Modem(void);
void SendATE02Modem(void);
void SendQIOPEN2Modem(void);
void SendQIACT_SET2Modem(void);
void SendQIACT_REQ2Modem(void);
extern void SendData2Modem(uint8_t * DataIn, uint16_t Len);
void    modem_SW_reset(void);
void SendConfig2Modem(void);
//void sendsmstest(uint8_t *pInData,bool ack_test);
void Senddatamode(void); // 9/3/2021
void SendSimReady2Modem(void);
bool CheckSimReadyResp(uint8_t* pDdataIn, uint16_t Len);
bool CheckATE0Resp(uint8_t* pDdataIn,uint16_t Len);
bool CheckCGDCONT_Resp(uint8_t* pDdataIn,uint16_t Len);
bool CheckCGREGResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIOPENResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIOPENResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIACT_REQResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIACT_SETResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckACKResp(uint8_t*pDdataIn,uint16_t Len);
bool CheckSMSResp(uint8_t*pDdataIn,uint16_t Len);

bool CheckNOCARRIERResp(uint8_t* pDdataIn, uint16_t Len);
bool CheckErrorResp(uint8_t* pDdataIn,uint16_t Len);
bool sms_config_received =false;
uint8_t ConnectionState   = MODEM_DISCONNECT_STATE;   //
static uint8_t AT_bufferTransmit[256];
//static uint8_t SMS_ON = 0;  // y.f. 9/3/2021 add a break to transparent mode to read sms
static uint8_t sms_config =0;
//extern void DelayYos(uint32_t Timems);
extern RepeaterConfigStruct RepeaterConfig;
extern uint16_t ReadConfigFromSMS(uint8_t*pDdataIn, uint16_t Len);
//extern void SetLed(ioport_port_pin_t pin_num, bool STATE);
extern void  ToggleLed1(void);
#ifdef DEBUG_ON
extern void SendData2Debug(uint8_t * DataIn, uint16_t Len,DebugOriginating debug);
#endif
uint8_t GsmCompleateStatus(void)
{
    return ConnectRetries;
}
void ClrGsmCompleate(void)
{
  ConnectRetries =4;
}
void SetGsmCompleate(void)
{
  ConnectRetries=0;
  GsmTimer=0;
  ConnectionState=MODEM_DISCONNECT_STATE;
  ToggleLedsReq = false;

}
void Gsm_Init(void)
{
  ConnectionState=MODEM_DISCONNECT_STATE;
  SendRetry = SEND_RETRY_TIMES;  
  GsmTimer = 200; // wait 2 seconds and then check if provider is connected
  ToggleLedsReq = false;
  // Start gsmtimer to check if already connect to provider 
}
void Gsm_Handler(uint8_t * pDdataIn, uint32_t Len , EventType Event)
{
    char *   position;
    char Str[100];


    position= strstr((char*)pDdataIn,(char *)SMS_IN_HEADER);
    if (position)
    {// sms can come in evry state
        sms_config_received =true;
#ifdef DEBUG_UART_MODE9
  //      sprintf (( char*)&Str, "RECEIVED  SMS FROM USER\r\n" );
  //      position[70]=0;
  //      strcat (( char*)&Str,( char*)position);
        //     R_SCI_UART_Write (&Debug_Uart1_ctrl, Str,80);
              R_SCI_UART_Write (&Debug_Uart1_ctrl, "debugGW RECEIVED  SMS FROM USER\r\n" ,37);
              R_BSP_SoftwareDelay (500, 1000); // delay 2 sec
#endif
        DelSmsMsg();
      uint16_t resultsms;
      resultsms = ReadConfigFromSMS(position, Len);
      if (resultsms ==1)
      {// enter to flash only if valid configuration is found
        WriteInternalFlashConfig(); // 3/1/2023 store the configuration
        R_BSP_SoftwareDelay (500, 1000); // delay 2 sec
        memcpy ((unsigned char*) &RepeaterConfig, (uint8_t*) FLASH_HP_DF_BLOCK_1, sizeof(RepeaterConfigStruct)); // 4/1/23 readback to be sure it was changed
        //write_internal_config_flag = true; // 14/6/22 temp use flag instead of direct write internal
        sendsmstest(pDdataIn,true); //18/12/22 send ack configuration to user
       /* Issue a software reset to reset the MCU. */
        if (ConnectionState == MODEM_MAGNET_READ_SMS) SendRetry =1; // y.f. 18/12/22 stop wait for sms go to nest state
        // HAL_NVIC_SystemReset(); // after configuration read from sms reset mcu is needed
      }
      else if (resultsms ==2)
      {
          sendsmstest(pDdataIn,true); //18/12/22 send ack configuration to user
      }
      else
      {
#ifdef DEBUG_UART_MODE3
        R_SCI_UART_Write (&Debug_Uart1_ctrl, "debugGW not valid sms\r\n",22);
#endif
      }
    }
    else
    {
        if (strstr((char*)pDdataIn,"+CMGR"))
        { // other sms should be simple delete
          DelSmsMsg();
        }
    }

  switch(ConnectionState)
  {
    case MODEM_DISCONNECT_STATE:
        if (Event == TIME_OUT_EVENT)
        {
          if ( ConnectRetries)
          {
              ConnectRetries--;
              SendCOMMAND_MODE2Modem();
              ConnectionState = MODEM_AT0_STATE;
              GsmTimer = 200;
              SendRetry = SEND_RETRY_TIMES;
          }
       }
      
      break;
  case MODEM_AT0_STATE:
        if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendATE02Modem();
        }
        else if  (Event==DATA_IN_EVENT)
        {
          if (CheckATE0Resp(pDdataIn, Len))
          {
            SendRetry = SEND_RETRY_TIMES;
            ConnectionState = MODEM_SET_CONFIGURATION_STATE;
            SendConfig2Modem();
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state SET CONFIG\r\n" ,38);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif
          }
          else
          {
              sms_config = 0; // for test to removed
          }
        }
    break;
  case MODEM_SET_CONFIGURATION_STATE:
    
        if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendConfig2Modem();
        }
        else if  (Event==DATA_IN_EVENT)
        {
          if (CheckCGDCONT_Resp(pDdataIn, Len))
          {
            SendRetry = SEND_RETRY_TIMES;
            ConnectionState = MODEM_SET_SMS_CONFIGURATION_STATE;
            sms_config = 0;
            SetSMSconfig(sms_config);
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state SET SMS CONFIG\r\n" ,41);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif

          }
        }
    break;

    case MODEM_SET_SMS_CONFIGURATION_STATE:
        if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
             {
               modem_SW_reset(); // need to reset modem it does not response
             }
            else
            {
                sms_config =0;
                SetSMSconfig(sms_config);
            }
            GsmTimer = 200;
        }
        else
          if  (Event==DATA_IN_EVENT)
          {
            position = strstr(pDdataIn,(char*)OK);
            if (position)
            {
              if (sms_config < 5)
              {
                SetSMSconfig(sms_config);
                sms_config++;
                if ( sms_config==5)
                {
                  SendRetry = SEND_RETRY_TIMES;
                  ConnectionState = SIM_READY_STATE;
                  SendSimReady2Modem();
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state sim ready\r\n" ,34);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif

                }
              }
            }
          }
 

    break;
    case SIM_READY_STATE:
    if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendSimReady2Modem();
        }
    else
       if  (Event==DATA_IN_EVENT)
        {
          if (CheckSimReadyResp(pDdataIn, Len))
          {
       
            SendRetry = SEND_RETRY_TIMES;
            ConnectionState = MODEM_PROVIDER_STATE;
            SendCGREG2Modem();
//            HAL_UART_Transmit(&UartDEBUGHandle,"\r\Sim Ready",11,5000 != HAL_OK);
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state MODEM PROVIDER\r\n" ,40);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif
          }
        }     
    
    break;

    case MODEM_PROVIDER_STATE:
        if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendCGREG2Modem();
        }
        else if  (Event==DATA_IN_EVENT)
        {
        
          if (CheckCGREGResp(pDdataIn, Len))
          {
    //           sendsmstest("yossi test",false ); // to be removed for test only
  //            R_BSP_SoftwareDelay (100, 1000); // delay 1 sec temp to be removed
      //        if (powerup_start)
              {
               //   MagnetDetect  = false;
         //         powerup_start = false;
                  GsmTimer = 100; // 2 min wait befor going to to be removed
                  ConnectionState = MODEM_MAGNET_READ_SMS;
#ifdef DEBUG_UART_MODE3
                 R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state MODEM_MAGNET_READ_SMS\r\n" ,51);
#endif
       //         sendsmstest("wait for new config",false);  //to be removed
      //            ReadSmsMsg();
                  SendRetry = 4; // y.f. send read sms befor close socket
          //        sendfreesms("START SCAN FOR SMS CONFIG");

              }
#if 0
              else
              {
//                sendsmstest(pDdataIn,false); // for test only to be removd
                SendRetry = SEND_RETRY_TIMES;
 //               sendsmstest("yossi test",false ); // to be removed for test only
 //               R_BSP_SoftwareDelay (100, 1000); // delay 1 sec temp to be removed
   //             GsmTimer = 1200; // 2 min wait befor going to to be removed
                ConnectionState = MODEM_ACTIVATE_CONTEXID_STATE;
                SendQIACT_REQ2Modem();
#ifdef DEBUG_UART_MODE4
                R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state MODEM ACTIVATE CONTEXT ID\r\n" ,51);

#endif
              }
#endif
          }

          
        }
    
    break;
    case  MODEM_MAGNET_READ_SMS:
        if (Event == TIME_OUT_EVENT)
         {
             if ((--SendRetry)==0)
             {
                 // no sms continue to connect

                 SendRetry = SEND_RETRY_TIMES;
                 GsmTimer = 1200; // 2 min wait befor going to to be removed
                 ConnectionState = MODEM_ACTIVATE_CONTEXID_STATE;
                 SendQIACT_REQ2Modem();
#ifdef DEBUG_UART_MODE4
                 R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state MODEM ACTIVATE CONTEXT ID\r\n" ,51);
 #endif
             }
             else
             {
              //   GsmTimer = 10000; // 2 min wait befor going to to be removed
                 ReadSmsMsg();
                 GsmTimer = 1000; // 2 min wait befor going to to be removed
             }
         }
        else if (sms_config_received)
        {
            sms_config_received =false;
            SendRetry = SEND_RETRY_TIMES;
            GsmTimer = 300; // 2 min wait befor going to to be removed
            ConnectionState = MODEM_ACTIVATE_CONTEXID_STATE;
            SendQIACT_REQ2Modem();
        }
        break;
  case MODEM_ACTIVATE_CONTEXID_STATE:
           if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendQIACT_REQ2Modem();
        }
        else if  (Event==DATA_IN_EVENT)
        {
          
          if (CheckQIACT_REQResp(pDdataIn, Len))
          {// Contex id already exist go to connection state 
            SendRetry = SEND_RETRY_TIMES;
            ConnectionState = MODEM_QIOPEN_STATE;
            SendQIOPEN2Modem();
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state MODEM QIOPEN\r\n" ,38);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif

          }
          else
          { // need to activate contexid
              //     SendRetry = SEND_RETRY_TIMES*10;
            SendRetry = SEND_RETRY_TIMES;
            ConnectionState = MODEM_ACTIVATE_SET_CONTEXID_STATE;
            SendQIACT_SET2Modem();
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state MODEM ACTIVATE CONTEXT ID SET\r\n" ,55);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif
            
          }
        }
    break;
     case MODEM_ACTIVATE_SET_CONTEXID_STATE:
           if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendQIACT_SET2Modem();
        }
        else if  (Event==DATA_IN_EVENT)
        {
          
          if (CheckQIACT_SETResp(pDdataIn, Len))
          {
            SendRetry = SEND_RETRY_TIMES;
            ConnectionState = MODEM_QIOPEN_STATE;
            SendQIOPEN2Modem();
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state MODE QIOPEN\r\n" ,35);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif
//            HAL_UART_Transmit(&UartDEBUGHandle,"\r\Contexid Activated",20,5000 != HAL_OK);

          }
        }
    break;
 
  case MODEM_QIOPEN_STATE:
         if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
             SendQICLOSE2Modem();
             R_BSP_SoftwareDelay (1000, 1000); // delay 2 sec
             //DelayYos(1000000);
             modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendQIOPEN2Modem();
        }
        else if  (Event==DATA_IN_EVENT)
        {
          if (CheckQIOPENResp(pDdataIn, Len))
          {

            ConnectionState = MODEM_CONNECT_STATE;

            // 15/1/23  LOGIN TO server as gw
            SendLogin2Server();
            GsmTimer = 3000;
            SendRetry = SEND_RETRY_TIMES;

#ifdef DEBUG_UART_MODE4
 //           R_BSP_SoftwareDelay(100, 1000); //

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state CONNECT\r\n" ,35);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed

#endif

          }
          else if (CheckErrorResp(pDdataIn, Len))
          { // It seems contexid is already used try close and open again
#ifdef DEBUG_UART_MODE4

            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW DACIVATE AGAIN\r\n" ,25);

#endif

       // temp removed just wait for time out and send qiopen again
       // temp removed 15/09/22       ReadSmsMsg();
             GsmTimer = 510;
             SendRetry = 1; // y.f. send read sms befor close socket 
          }
        }
    break;
    case MODEM_CONNECT_STATE: // state only for dlt 
         if (Event == TIME_OUT_EVENT)
        {
          
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else
            {  // y.f. 7/2/2022 SendCONNECT2Modem();
                // 15/1/23  LOGIN TO server as gw
                SendLogin2Server();
                GsmTimer = 510;
            }
        }
        else if  (Event==DATA_IN_EVENT)
        {

            if (ExtractWrapper(pDdataIn,Len) == LOGIN_RESPONSE)
            { // server ack to connection message from meter
                SendRetry = SEND_RETRY_TIMES;
                ConnectionState = MODEM_DATA_STATE;
                heartbit_count =0;
                SendHeartBit2Server();
                GsmTimer = 12000;
                ToggleLedsReq = true;
                SetLed (PULSER_LED1, true); // to be removed only for test
                SetLed (PULSER_LED2, true);

#ifdef DEBUG_UART_MODE4
            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW connection state DATA\r\n" ,30);
#endif

          }
          else if(CheckNOCARRIERResp(pDdataIn, Len))
          {
             ToggleLedsReq = false;
                          // end of communication 
             modem_SW_reset(); // need to reset modem server does not response 
          }   
          else
          {
#ifdef DEBUG_UART_MODE4
            R_SCI_UART_Write(&Debug_Uart1_ctrl,"debugGW not proper message rx\r\n" ,40);
#endif

          }

        }
    break;
    
    
    case MODEM_DATA_STATE:
        if (Event == TIME_OUT_EVENT) 
        {
           if ( heartbit_count < 10)
           {
               SendHeartBit2Server();
               GsmTimer = 12000; // 2 min heart bit
               heartbit_count++;
           }
           else
           { // it seems that server is not responding try reconnect
               modem_SW_reset(); // need to reset modem it does not response
           }
        }
        else if  (Event==DATA_IN_EVENT)
        {
          if (CheckNOCARRIERResp(pDdataIn, Len))
          {
                            // end of communication 
                  modem_SW_reset(); // need to reset modem no carrier
          }
          else
          switch(ExtractWrapper(pDdataIn,Len))
          {
              case HEARTBIT_RESPONSE:
                  heartbit_count = 0 ;
                  break;
              case DLMS_MSG_2_METER:
                  // send transparent to coordinator
                  R_SCI_UART_Write (&Debug_Uart1_ctrl, pDdataIn, Len);
                  break;
              case DLMS_MSG_2_GW:
                  // t b d y.f. 8/3/23
                  break;
              default: // ignore not proper or other
                  break;
          }
         }
      break;
  }
 
}

void SendSimReady2Modem(void)
{
 
  memcpy (AT_bufferTransmit,SIM_READY,sizeof(SIM_READY));
  SendData2Modem(AT_bufferTransmit,sizeof(SIM_READY));



   GsmTimer = 200;
}

bool CheckSimReadyResp(uint8_t* pDdataIn, uint16_t Len)
{
     if (strstr((char*)pDdataIn,(char*)SIM_RESP))
       return true;
     return false;
}
void SendACK2Modem(void)
{
  memcpy (AT_bufferTransmit,SERVER_ACK_COMMAND,sizeof(SERVER_ACK_COMMAND));
  SendData2Modem(AT_bufferTransmit,sizeof(SERVER_ACK_COMMAND));
 // GsmTimer = 2400; // 2 min wait befor sending heartbit
}


bool CheckSMSResp(uint8_t*pDdataIn,uint16_t Len)
{
  if (strstr((char*)pDdataIn,(char*)SMS_IN_HEADER))
 
      return true;
  else
      return false;
   
}

bool CheckNOCARRIERResp(uint8_t* pDdataIn, uint16_t Len)
{
     if (strstr((char*)pDdataIn,(char*)NOCARRIERRESP))
       return true;
     return false;
}
bool CheckErrorResp(uint8_t* pDdataIn,uint16_t Len)
{
   if (strstr((char*)pDdataIn,(char*)ERRORRESP))
       return true;
     return false; 
}

                    
bool CheckCGDCONT_Resp(uint8_t* pDdataIn,uint16_t Len)
{
    if (strstr((char*)pDdataIn,(char*)OK))

       return true;
     return false;

  
}

bool CheckATE0Resp(uint8_t* pDdataIn,uint16_t Len)
{
 
 if  (strstr((char*)pDdataIn,(char*)OK))
 
  
  return true;
 else
  return false;
  
}
bool CheckCGREGResp(uint8_t* pDdataIn,uint16_t Len)
{
 // if (memcmp(pDdataIn,(uint8_t*)CGREG_RESP, sizeof(CGREG_RESP)-1))
  if (strstr((char*)pDdataIn,(char *)CGREG_RESP))
  return true;
 else
  return false;

}


bool CheckQIOPENResp(uint8_t* pDdataIn,uint16_t Len)
{
  if (strstr((char*)pDdataIn,(char*)QIOPEN_RESP))
  {//good response go to next state
 
    return true;
  } 
  return false;
}

void StartOpenConnection(void)
{
 // SendAtCommand(AT_open);
}

void StopConnection(void)
{
 // SendAtCommand(AT_close);
}

void SendCOMMAND_MODE2Modem(void)
{
//  HAL_GPIO_WritePin(MODEM_DTR_PORT, MODEM_DTR_PIN,GPIO_PIN_RESET);  // DTR ON TO OFF=COMMAND MODE
//#if 0
 // temp removed y.f. 15/6/2022  DelayYos(7000000);
  memcpy (AT_bufferTransmit,COMMAND_MODE,sizeof(COMMAND_MODE));
  SendData2Modem(AT_bufferTransmit,sizeof(COMMAND_MODE)); // return to data mode 
  R_BSP_SoftwareDelay (2000, 1000); // delay 2 sec



//  DelayYos(7000000);
//#endif
  

}
#if 0
// not in use in pulser
void SMS_Handler(void)
{

   switch  (SMS_ON)
   {
      case 0:
//        HAL_UART_Transmit(&UartDEBUGHandle,"\r\n\modem enter to command mode\r\n",31,5000 != HAL_OK);
        GsmTimer = 120; // at least 1 sec befor +++
        SMS_ON++;
        break;
      case 1:
        SendCOMMAND_MODE2Modem(); // turn off transparent mode and go back to command mode for sms read
        GsmTimer = 340; // 4000; //340;
        SendRetry = SEND_RETRY_TIMES;
        SMS_ON++;
        break;
      case 2:
        ReadSmsMsg();
        GsmTimer = 510; //4000;//510;
        SMS_ON++;
        break;
      case 3:
          SendATE02Modem(); // 5/1/2022 y.f. avoid echo 
          SMS_ON++;
          GsmTimer = 1100; //4000;//510;
          break;
      case 4:    
        Senddatamode();
        GsmTimer = 1085; // 2 min wait befor sending heartbit
        SMS_ON = 0;
        SendRetry = SEND_RETRY_TIMES;
//        HAL_UART_Transmit(&UartDEBUGHandle,"\r\nmodem back to data mode\r\n",27,5000 != HAL_OK);
        break;
   }
 
   
   
}
#endif

void SendDLMSACK2Modem(void)
{
  memcpy (AT_bufferTransmit,SERVER_DLMS_ACK_COMMAND,sizeof(SERVER_DLMS_ACK_COMMAND));
  SendData2Modem(AT_bufferTransmit,sizeof(SERVER_DLMS_ACK_COMMAND));
 // GsmTimer = 1200; // 2 min wait befor going to
}


void SendQIOPEN2Modem(void)
{
  
  AT_bufferTransmit[0]=0;
  strcat((char*)&AT_bufferTransmit , QIOPEN_HEADER);
  strcat((char*)&AT_bufferTransmit , RepeaterConfig.GsmConfig.ServerIP);
  strcat((char*)&AT_bufferTransmit,(char*)",");
  strcat((char*)&AT_bufferTransmit , RepeaterConfig.GsmConfig.Port);
  strcat((char*)&AT_bufferTransmit , QIOPEN_FOOTER);
  SendData2Modem(AT_bufferTransmit,strlen(AT_bufferTransmit));
  GsmTimer = 300; // wait 2 seconds and then check if provider is conn
  
  
//  memcpy (AT_bufferTransmit,QIOPEN,sizeof(QIOPEN));
//  SendData2Modem(AT_bufferTransmit,sizeof(QIOPEN)-1);
//  GsmTimer = 3000; // wait 30 seconds
 
}
void SendQICLOSE2Modem(void)
{
//  HAL_UART_Transmit(&UartDEBUGHandle,"\r\close socket",14,5000 != HAL_OK);

  memcpy (AT_bufferTransmit,QICLOSE,sizeof(QICLOSE));
  SendData2Modem(AT_bufferTransmit,sizeof(QICLOSE)-1);
  GsmTimer = 200; // wait 30 seconds
 
}

void SendConfig2Modem(void)
{
  AT_bufferTransmit[0]=0;
  strcat((char*)&AT_bufferTransmit , CGDCONT_HEADER);
  strcat((char*)&AT_bufferTransmit , RepeaterConfig.GsmConfig.APN);
  strcat((char*)&AT_bufferTransmit,(char*)",");
  strcat((char*)&AT_bufferTransmit , RepeaterConfig.GsmConfig.ServerIP);
  strcat((char*)&AT_bufferTransmit , CGDCONT_FOOTER);
  SendData2Modem(AT_bufferTransmit,strlen(AT_bufferTransmit));
  GsmTimer = 200; // wait 2 seconds and then check if provider is connected
}
void SendATE02Modem(void)
{
  memcpy (AT_bufferTransmit,ATE0,sizeof(ATE0));
  SendData2Modem(AT_bufferTransmit,sizeof(ATE0)-1);
  GsmTimer = 200; // wait 2 seconds and then check if provider is connected
}
void SendCGREG2Modem(void)
{
  memcpy (AT_bufferTransmit,CGREG,sizeof(CGREG));
  SendData2Modem(AT_bufferTransmit,sizeof(CGREG)-1);
  GsmTimer = 300; // wait 2 seconds and then check if provider is connected
}
void SendQIACT_REQ2Modem(void)
{
  memcpy (AT_bufferTransmit,QIACT_REQ,sizeof(QIACT_REQ));
  SendData2Modem(AT_bufferTransmit,sizeof(QIACT_REQ)-1);
  GsmTimer = 300; // wait 2 seconds and then check if provider is connected
}
void SendQIACT_SET2Modem(void)
{
  memcpy (AT_bufferTransmit,QIACT_SET,sizeof(QIACT_SET));
  SendData2Modem(AT_bufferTransmit,sizeof(QIACT_SET)-1);
  GsmTimer = 200; // wait 2 seconds and then check if provider is connected
}
void SendDQIACT_SET2Modem(void)
{
  memcpy (AT_bufferTransmit,DQIACT_SET,sizeof(DQIACT_SET));
  SendData2Modem(AT_bufferTransmit,sizeof(DQIACT_SET)-1);
  GsmTimer = 200; // wait 2 seconds and then check if provider is connected
}


void ReadSmsMsg(void)
{
#if 0
  memcpy (AT_bufferTransmit,CMGR,sizeof(CMGR));
  SendData2Modem(AT_bufferTransmit,sizeof(CMGR)-1);
#endif

  memcpy (AT_bufferTransmit,CMGL,sizeof(CMGL));
  SendData2Modem(AT_bufferTransmit,sizeof(CMGL)-1);


 // y.f. removed 14/09/22 GsmTimer = 200; // wait 2 seconds
}

void DelSmsMsg(void)
{
  memcpy (AT_bufferTransmit,CMGD,sizeof(CMGD));
  SendData2Modem(AT_bufferTransmit,sizeof(CMGD)-1);
 // y.f. 13/9/ temp removed  GsmTimer = 200; // wait 2 seconds
}
void SendATD(void)
{
  memcpy (AT_bufferTransmit,ATD,sizeof(ATD));
  SendData2Modem(AT_bufferTransmit,sizeof(ATD)-1);
 // removed 14/09/22  GsmTimer = 200; // wait 2 seconds
}
void Senddatamode(void)
{
  //HAL_GPIO_WritePin(MODEM_DTR_PORT, MODEM_DTR_PIN,GPIO_PIN_SET);  // DTR OFF TO ON = DATA MODE


  memcpy (AT_bufferTransmit,DATA_MODE,sizeof(DATA_MODE));
  SendData2Modem(AT_bufferTransmit,sizeof(DATA_MODE)-1);
 // y.f. 14/09/22 removed  GsmTimer = 200; // wait 2 seconds and then check if provider is connected
  
}

bool CheckQIACT_SETResp(uint8_t* pDdataIn,uint16_t Len)
{
   if (strstr((char*)pDdataIn,OK)) 
   return true;
    else
  return false;
}
bool CheckQIACT_REQResp(uint8_t* pDdataIn,uint16_t Len)
{
  if (strstr((char*)pDdataIn,(char*)QIACT_CHECK_RESP)) 
  return true;
    else
  return false;
}


void    modem_SW_reset(void)
{
 // HAL_NVIC_DisableIRQ(EXTI9_5_IRQn);  
  SendCOMMAND_MODE2Modem();   
  memcpy (AT_bufferTransmit,MODEM_RESET,sizeof(MODEM_RESET));
  SendData2Modem(AT_bufferTransmit,sizeof(MODEM_RESET)-1);
  SendRetry = SEND_RETRY_TIMES;
  ConnectionState = MODEM_DISCONNECT_STATE;
//#ifdef DEBUG_UART_MODE
//            R_SCI_UART_Write(&Debug_Uart1_ctrl,"connection state DISCONNECT\r\n" ,29);
//            R_BSP_SoftwareDelay(1000, bsp_delay_units); // delay 1 sec temp to be removed
//
//#endif
  GsmTimer = 200; // wait 2 seconds and then check if provider is connected
//  HAL_UART_Transmit(&UartDEBUGHandle,"\r\Modem Disconnect",18,5000 != HAL_OK);

}  

void SetSMSconfig(uint8_t smsstate)
{
  //SendSMSGPRS();
      switch (smsstate)
      {
      case 0:
          memcpy (AT_bufferTransmit,SMS_FORMAT,sizeof(SMS_FORMAT));
          SendData2Modem(AT_bufferTransmit,sizeof(SMS_FORMAT)-1);
        break;
      case 1:
         memcpy (AT_bufferTransmit,SMS_CHARSET,sizeof(SMS_CHARSET));
        SendData2Modem(AT_bufferTransmit,sizeof(SMS_CHARSET)-1);
        break;
      case 2:
          memcpy (AT_bufferTransmit,SMS_DELIVERY_FORMAT,sizeof(SMS_DELIVERY_FORMAT));
          SendData2Modem(AT_bufferTransmit,sizeof(SMS_DELIVERY_FORMAT)-1);
        break;
      case 3:
        SendATD();
        break;
      case 4:
     //     DelSmsMsg(); // y.f. 14/12/22

     //    memcpy (AT_bufferTransmit,SMS_RING_INDICATOR,sizeof(SMS_RING_INDICATOR));
     //    SendData2Modem(AT_bufferTransmit,sizeof(SMS_RING_INDICATOR)-1);
        break;
        
     }
  GsmTimer = 200;
}
  uint8_t sendsmstest(uint8_t * pDdataIn,bool ack_test)
  {

    // retrieve caller phon number 
    uint16_t bInd=0,tmp=0;
    uint8_t SmsPhon[20];
    AT_bufferTransmit[0]=0;
    strcat((char*)&AT_bufferTransmit , SMS_TEST_SEND_HEADER);
    memset(SmsPhon,0,sizeof(SmsPhon));
    while (((pDdataIn[bInd]!= 0x22)|| (pDdataIn[bInd+1]!= 0x2b)) && (bInd <50)) // find first '"+'
           bInd++;
     bInd++;
   
    if (ack_test)
    {
      if (bInd>=50) return 0;
      while ((pDdataIn[bInd]!= 0x22)&& (bInd <140))  // find second '"' sample "+972546262888"
           SmsPhon[tmp++] = pDdataIn[bInd++];
    
      strcat((char*)&AT_bufferTransmit , (char*)SmsPhon);
    }
    else
    {
      strcat((char*)&AT_bufferTransmit , (char*)yossiphon);
    }
    strcat((char*)&AT_bufferTransmit , SMS_TEST_SEND_FOOTER);
    if (ack_test)
    {// Y.F. 4/1/23 add ip & port
        strcat((char*)&AT_bufferTransmit , "IP:");
        strcat((char*)&AT_bufferTransmit , RepeaterConfig.GsmConfig.ServerIP);
        strcat((char*)&AT_bufferTransmit, "\r\nPORT:");
        strcat((char*)&AT_bufferTransmit , RepeaterConfig.GsmConfig.Port);

    }
    //  strcat (AT_bufferTransmit,"retrofit ack configuration");
    else
      strcat (AT_bufferTransmit,"          Start Modem");
    strcat((char*)&AT_bufferTransmit,(char*)&CONTROL_Z);
    SendData2Modem(AT_bufferTransmit,strlen(AT_bufferTransmit));
    R_BSP_SoftwareDelay (2000, 1000); // delay 20 sec to be sure all message is sent

//    DelayYos(5000000);
    return 1;
 }
  void sendfreesms(uint8_t * pDdataIn)
  {

    // retrieve caller phon number 
    uint8_t SmsPhon[20];
    AT_bufferTransmit[0]=0;
    strcat((char*)&AT_bufferTransmit , SMS_TEST_SEND_HEADER);
    memset(SmsPhon,0,sizeof(SmsPhon));
    strcat((char*)&AT_bufferTransmit , (char*)yossiphon);
    strcat((char*)&AT_bufferTransmit , SMS_TEST_SEND_FOOTER);
//    strcat (AT_bufferTransmit,"retrofit ack configuration");
    strcat (AT_bufferTransmit,pDdataIn);
    strcat((char*)&AT_bufferTransmit,(char*)&CONTROL_Z);
    SendData2Modem(AT_bufferTransmit,strlen(AT_bufferTransmit));
//    DelayYos(5000000);
    R_BSP_SoftwareDelay (2000, 1000); // delay 2 sec

 }

void RingHandler(void)
{
   if (ConnectionState>=MODEM_PROVIDER_STATE)
       //  HAL_NVIC_SystemReset(); // after configuration read from sms reset mcu is needed
           __NVIC_SystemReset;
}

void  Error_UartGSMHandler(void)
{
  while (1)
  {
  }
}

uint16_t  ReadConfigFromSMS(uint8_t *pDdataIn, uint16_t Len)
{
    char *position;
    uint16_t bInd=0;
    RepeaterConfigStruct tmpRepeaterConfig;
    position = (strstr((char*)pDdataIn,",2,"));
    if (position==0)
    {
        position = (strstr((char*)pDdataIn,",3,"));
        if (position ==0 ) return 0;  // not found start of string
        else  return 2 ; // this is read config just send config
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
                PrintSMSError(position);
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
            PrintSMSError(position);
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
   PrintSMSError(position);
   return 0; // not valid char

}

#ifdef DEBUG_UART_MODE3
void PrintSMSError (char *data)
{
        char Str[50];

        sprintf (( char*)&Str, "This is The place We failed:" );
        data[70]=0;
        strcat (( char*)&Str,( char*)data);
        R_SCI_UART_Write (&Debug_Uart1_ctrl, Str,80);
        R_BSP_SoftwareDelay (500, 1000); // delay 2 sec
}
#endif
