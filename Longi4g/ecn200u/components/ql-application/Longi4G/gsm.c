/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ql_api_osi.h"
#include "ql_log.h"
#include "longi4g.h"
#include "ql_osi_def.h"
#include "ql_fs.h"
#include "longi_nor_flash.h"
#include "LongiWrapper.h"
#include "socket_demo.h"
#include "utils.h"
#pragma pack(1)

//	Modem handler states mechine
//===========================================


enum Response_State {
  NO_RESPONSE = 0,
  WRONG_RESPONSE ,
  GOOD_RESPONSE
};

#define SEND_RETRY_TIMES        5
//extern uint16_t GsmTimer;
uint16_t SendRetry = SEND_RETRY_TIMES;  
void sendfreesms(uint8_t * pDdataIn);
bool ReadConfigFromSMS(char *pDdataIn, uint16_t Len);
void ReadSmsMsg(void);
void SendSMSGPRS(void);
void DelSmsMsg(void);
void SendATD(void);
void sendCMEVerbouse(void);
char str[255];
uint8_t sendsmstest(uint8_t * pDdataIn,bool ack_test);
const char CMERR[]         = {"r\n AT+CMEE=2\r\n"};
const char ATE0[]          = { "\r\nATE0\r\n" } ;           // A    stops echo from modem 
const char ATE0_RESP[]     = { "\r\nATE0\r\nOK\r\n" } ;  
const char CREG[]          = { "\r\nAT+CREG=?\r\n" } ;   //    
const char SIM_READY[]     = { "\r\nAT+CPIN?\r\n"};   // B
const char SIM_RESP[]         = { "READY"};
//const char SIM_RESP[]      = {"\r\n+CPIN: READY\r\n"}; // y.f. change to full response 11/6/23 
const char CGREG[]        = { "\r\nAT+CGREG?\r\n" }; // C
const char CGREG_RESP[]   = { "\r\n+CGREG: 0,1\r\n"}; // 
const char CMGR[] = {"\r\nAT+CMGR=1\r\n"};
const char CMGD[]   = {"\r\nAT+CMGD=1\r\n"};  // delete all messages after read  
//const char CGDCONT[]     = { "AT+CGDCONT=1,IP,internet,32.154.79.218,0,0,0,0,0,0\r\n"}; // D XILIN SERVER 218.78.213.232 4059
//const char CGDCONT[]     = { "AT+CGDCONT=1,IP,internet\r\n"}; // 
const char QICSGP[]      = { "\r\nAT+QICSGP=1,1,internet,,,0\r\n"};
//const char QICSGP[]      = { "\r\nAT+QICSGP=?\r\n"};
const char CGDCONT_HEADER[]     = { "AT+CGDCONT=1,IP,"};
const char CGDCONT_FOOTER[]     = { ",0,0,0,0,0,0\r\n"};
const char QIOPEN_HEADER [] = { "AT+QIOPEN=1,0,TCP,"}; // Open in transparent mode
const char QIOPEN_FOOTER [] = { ",0,2\r\n"}; 
const char QIOPEN_RESP[] = { "\r\nCONNECT\r\n"}; // TO BE CHECK 
const char QICLOSE[]     = { "\r\nAT+QICLOSE=0\r\n"};
const char OK[]          = { "\r\nOK\r\n"};  // to be checked again
const char CGDCONT_RESP[]= { "\r\n+CGDCONT: 1,IP,internet,32.154.8.246,0,0,0,0,0,0\r\n"};
const char QISTAT[]      = { "\r\nAT+QISTAT?\r\n" };
const char CSQ[]         = { "\r\nAT+CSQ\r\n"  };
const char COMMAND_MODE[]  = {0x2b,0x2b,0x2b}; // +++ command mode 
const char QIACT_SET[]         = {"\r\nAT+QIACT=1\r\n"};
const char QIACT_REQ[]         = {"\r\nAT+QIACT?\r\n"};
const char NOCARRIERRESP[]     = {"\r\nNO CARRIER\r\n"};
const char ERRORRESP[]     =     {"\r\nERROR\r\n"};
const char CONNECTION_DLMS_HEADER_COMMAND[]  =     {0x4D,0x4F,0x44,0x45,0x4D,0x5F,0x49,0x44,0x3D,0x51};
const char CONNECTION_DLMS_FOOTER_COMMAND[]  =     {0x2c,0x4D,0x45,0x54,0x45,0x52,0x5f,0x49,0x44,0x3d,0x51};
const char SERVER_DLMS_HEARTBIT[] =               {  0x7E, 0xA0, 7, 0x41, 3, 0x1F, 0x26, 0xEC, 0x7E  };
const char SERVER_DLMS_ACK_COMMAND[] =            { 0x7E, 0xA0, 7, 0x41, 3, 0x1F, 0x26, 0xEC, 0x7E  };
const char PHON_HEADER[] = {"+CMT:"};
const char QIACT_CHECK_RESP[]         = {"\r\n+QIACT: 1,1,1"};
const char CONNECTION_HEADER_COMMAND[]     =    {0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x12,0xa,0x2,0xc};
const char SERVER_LOGIN_HEADER_ACK_COMMAND[]      =    {0,1 ,0 ,1, 0, 1, 0, 0x12, 0xaa, 3 ,0xc}; // this message is ack command received for connection and for login commands
const char SERVER_HEARTBIT_HEADER_ACK_COMMAND[]   =    {0,1 ,0 ,1, 0, 1, 0, 0x11, 0xcc, 3 ,0xc}; // this message is ack command received for connection and for login commands
const char SMS_IN_HEADER[]   = {"+GSMS:"};
const char DATA_MODE[]     = { "\r\nATO\r\n"};
const char DISCONNECT_CALL[]    = { "ATH/r/n"};
const char MODEM_RESET[]        = {"\r\nAT+CFUN=1,1\r\n"};        // y.f  7/3/21
const char SMS_FORMAT[]         = {"\r\nAT+CMGF=1\r\n"};          // y.f. 7/3/21 TEXT MODE
const char SMS_CHARSET[]        = {"\r\nAT+CSCS=GSM\r\n"};       // y.f. 7/3/21
const char SMS_DELIVERY_FORMAT[]= {"\r\nAT+CNMI= 2,1,0,0,0\r\n"};  // y.f. 7/3/21
const char ATD[]               = {"\r\nAT&D1\r\n"};  // on to off in DTR turns to command mode  
const char yossiphon[] = {"+972546262816"};
const char SMS_TEST_SEND_HEADER[] = {"\r\nAT+CMGS="};
const char SMS_TEST_SEND_FOOTER[] = {"\r\n"};
const char CONTROL_Z = 0x1a;
const char SMS_RING_INDICATOR[] = {"\r\nAT+QCFG=urc/ri/smsincoming,pulse\r\n"};;
void SMS_Process(void);
void Gsm_Init(void);
void Gsm_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
void SetSMSconfig(uint8_t smsstate);
void SendData(uint8_t * DataIn, uint16_t Len);
void SendQIOPEN2Modem(void);
bool CheckProvider(void);
extern  uint32_t SendLogin2Server(void );
extern  uint32_t SendHeartBit2Server(void );
void SendQICLOSE2Modem(void);
void SendCOMMAND_MODE2Modem(void);
void SendCGREG2Modem(void);
void SendATE02Modem(void);
void SendQIOPEN2Modem(void);
void SendQIACT_SET2Modem(void);
void SendQIACT_REQ2Modem(void);
void SendSimReg2Modem(void);
//extern void SendData2Modem(uint8_t * DataIn, uint16_t Len);
extern void Send2DebugUart(uint8_t *pDdataIn ,uint32_t len);
extern void clrMeterEvent(void);
void    modem_SW_reset(void);
void SendConfig2Modem(void);
void CheckInSMS(uint8_t *pDdataIn , uint16_t Len );
//void sendsmstest(uint8_t *pInData,bool ack_test);
void Senddatamode(void); // 9/3/2021
void SendSimReady2Modem(void);
bool CheckSimReadyResp(uint8_t* pDdataIn, uint16_t Len);
bool CheckATE0Resp(uint8_t* pDdataIn,uint16_t Len);
bool CheckCGDCONT_Resp(uint8_t* pDdataIn,uint16_t Len);
bool CheckDLMSHearBit(uint8_t* pDdataIn, uint16_t Len);
bool CheckCGREGResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIOPENResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIOPENResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIACT_REQResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckQIACT_SETResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckSMSResp(uint8_t*pDdataIn,uint16_t Len);
bool CheckNOCARRIERResp(uint8_t* pDdataIn, uint16_t Len);
bool CheckErrorResp(uint8_t* pDdataIn,uint16_t Len);
bool CheckCREGResp(uint8_t* pDdataIn,uint16_t Len);
uint8_t ConnectionState   = MODEM_DISCONNECT_STATE;   //
static uint8_t AT_bufferTransmit[256];
static uint8_t SMS_ON = 0;  // y.f. 9/3/2021 add a break to transparent mode to read sms 
static uint8_t sms_config =0;
extern long long 	ExtendedAddress;
extern uint32_t ExtractWrapper(uint8_t *pMsgIn, uint16_t len);
extern RepeaterConfigStruct RepeaterConfig;
extern uint16_t blinkFlag;
//extern uint16_t GsmTimer;
extern void SendData2Modem(uint8_t * DataIn, uint16_t Len);
extern ql_timer_t Gsm_debounce_timer;
/*************
AT+CREG=? AT+CCID AT+CGATT=1 AT+CGDCONT=1,"IP","internet" AT+CGACT=1,1
******************/
void Gsm_Init(void)
{
	ql_socket_app_init();	
#if 0
// y.f 18/06/23 no AT commands inthis project use quectel api instead 
  ConnectionState=MODEM_DISCONNECT_STATE;
  sprintf(str,"MODEM_DISCONNECT_STATE\r\n");
  Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
  SendRetry = SEND_RETRY_TIMES;  
  ql_rtos_timer_start(Gsm_debounce_timer, 20, 1);  
#endif  
}
#if 0
void Gsm_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event)
{
	char *   position;  
	if  (Event==DATA_IN_EVENT)
	{
		CheckInSMS(pDdataIn , Len );
	}
  switch(ConnectionState)
  {
    case MODEM_DISCONNECT_STATE:
        if (Event == TIME_OUT_EVENT)
        {
          SendCOMMAND_MODE2Modem();         
          ConnectionState = MODEM_AT0_STATE;
          sprintf(str,"MODEM_AT0_STATE\r\n");
	      Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
          blinkFlag = CONNECION_2_PROVIDER;
  //        ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  
//		  sendCMEVerbouse(); // AT+CMEE=2
//		  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  
          SendRetry = SEND_RETRY_TIMES;
 
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
	        sendCMEVerbouse(); // AT+CMEE=2
    		ql_rtos_task_sleep_ms(2000);  // y.f. wake every 1500 ms
            SendRetry = SEND_RETRY_TIMES*100;
            ConnectionState = SIM_READY_STATE;// temp y.f 11/6/23 to be restotre to original just for test 
			sprintf(str,"SIM_READY_STATE\r\n"); 
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			SendSimReady2Modem();
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
			SendRetry = SEND_RETRY_TIMES*100;
			ConnectionState = SIM_REGISTER_STATE;// temp y.f 11/6/23 to be restotre to original just for test 
			sprintf(str,"SIM_REGISTER_STATE\r\n"); 
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			SendSimReg2Modem();


          }
        }     
    
    break;
	case SIM_REGISTER_STATE:
	    if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
              modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendSimReg2Modem();
        }
		else if  (Event==DATA_IN_EVENT)
		{
			if (CheckCREGResp(pDdataIn, Len))
			{
							  
				SendRetry = SEND_RETRY_TIMES*100;
				ql_rtos_timer_start(Gsm_debounce_timer, 100000, 1); // start y.f. timer for end of chars  
				ConnectionState = MODEM_PROVIDER_STATE;
				sprintf(str,"MODEM_PROVIDER_STATE\r\n");
				Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
				SendCGREG2Modem();
				
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
			SendRetry = SEND_RETRY_TIMES*10;
            ConnectionState = MODEM_SET_CONFIGURATION_STATE;
		    sprintf(str,"MODEM_SET_CONFIGURATION_STATE\r\n");
		    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			SendConfig2Modem();
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
            SendRetry = 2;
            ConnectionState = MODEM_SET_SMS_CONFIGURATION_STATE;
		    sprintf(str,"MODEM_SET_SMS_CONFIGURATION_STATE\r\n");
		    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
            sms_config = 0;
            SetSMSconfig(sms_config);
          }
        }
    break;

    case MODEM_SET_SMS_CONFIGURATION_STATE:
        if (Event == TIME_OUT_EVENT)
        {
             SetSMSconfig(sms_config);
             ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  
        }
        else
          if  (Event==DATA_IN_EVENT)
          {
            position = strstr((char*)pDdataIn,(char*)OK);
            if (position)
            {
              if (sms_config < 5)
              {
                SetSMSconfig(sms_config);
                sms_config++;
                if ( sms_config==5)
                {
    				SendRetry = SEND_RETRY_TIMES*100;
					blinkFlag = CONNECTION_2_SERVER;
					ConnectionState = MODEM_ACTIVATE_CONTEXID_STATE;
					sprintf(str,"MODEM_ACTIVATE_CONTEXID_STATE\r\n");
					Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
					SendQIACT_REQ2Modem();
                }
              }
            }
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
            SendRetry = SEND_RETRY_TIMES*10;
            ConnectionState = MODEM_QIOPEN_STATE;
            sprintf(str,"MODEM_QIOPEN_STATE\r\n");
		    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			
            SendQIOPEN2Modem();
          }
          else
          { // need to activate contexid
            SendRetry = SEND_RETRY_TIMES*10;
            ConnectionState = MODEM_ACTIVATE_SET_CONTEXID_STATE;
            sprintf(str,"MODEM_ACTIVATE_SET_CONTEXID_STATE\r\n");
		    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			
            SendQIACT_SET2Modem();
            
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
            sprintf(str,"MODEM_QIOPEN_STATE\r\n");
		    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
            SendQIOPEN2Modem();
          }
        }
    break;
 
  case MODEM_QIOPEN_STATE:
         if (Event == TIME_OUT_EVENT)
        {
            if ((--SendRetry)==0)
            {
             SendQICLOSE2Modem();
			 ql_rtos_task_sleep_ms(50);
             modem_SW_reset(); // need to reset modem it does not response 
            }
            else SendQIOPEN2Modem();
        }
        else if  (Event==DATA_IN_EVENT)
        {
          
          if (CheckQIOPENResp(pDdataIn, Len))
          {
            SendRetry = SEND_RETRY_TIMES*2;
               
            ConnectionState = MODEM_DATA_STATE;
		    sprintf(str,"MODEM_DATA_STATE\r\n");
		    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
            SendLogin2Server();
            ql_rtos_timer_start(Gsm_debounce_timer, 10000, 1); // start y.f. timer for end of chars  

          }
          else if (CheckErrorResp(pDdataIn, Len))
          { // It seems contexid is already used try close and open again
             ReadSmsMsg();
             ql_rtos_timer_start(Gsm_debounce_timer, 10000, 1); // start y.f. timer for end of chars  
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
          SendLogin2Server();
          ql_rtos_timer_start(Gsm_debounce_timer, 10000, 1); // start y.f. timer for end of chars  

        }
        else if  (Event==DATA_IN_EVENT)
        {
          if (ExtractWrapper(pDdataIn,Len) == LOGIN_RESPONSE)
          { // server ack to connection message from meter
            SendRetry = SEND_RETRY_TIMES;
            ConnectionState = MODEM_DATA_STATE;
		    sprintf(str,"MODEM_DATA_STATE\r\n");
		    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
            SendHeartBit2Server();
            SMS_Process();
          }
          else if(CheckNOCARRIERResp(pDdataIn, Len))
          {
             // end of communication 
             modem_SW_reset(); // need to reset modem server does not response 
          }   
        }
    break;
    
    
    case MODEM_DATA_STATE:
        if (Event == TIME_OUT_EVENT) 
        {
           if ((--SendRetry)==0) 
           {
             modem_SW_reset(); // need to reset modem server does not response 
           }
           SendHeartBit2Server();
           SMS_Process();
        }
        else if  (Event==DATA_IN_EVENT)
        {
          if (CheckNOCARRIERResp(pDdataIn, Len))
          {
                            // end of communication 
               modem_SW_reset(); // need to reset modem no carrier 

          }    
         
            
		   if ((CheckDLMSHearBit(pDdataIn, Len)) ||
				(ExtractWrapper(pDdataIn, Len)))
		
            {
                SendRetry = SEND_RETRY_TIMES*2;
                ql_rtos_timer_start(Gsm_debounce_timer, 110000, 1); // start y.f. timer for end of chars  
				blinkFlag = DATA_IN_SERVER;
			               
            }
  
          
        }
      // check for disconnect or +++ to return to command mode
      break;
  }
 
}
#endif




void SendSimReady2Modem(void)
{
 
  memcpy (AT_bufferTransmit,SIM_READY,sizeof(SIM_READY));
  SendData2Modem(AT_bufferTransmit,sizeof(SIM_READY)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 3000, 1); // start y.f. timer for end of chars  

}
bool CheckCREGResp(uint8_t* pDdataIn,uint16_t Len)
{
	if (strstr((char*)pDdataIn,OK)) 
	   return true;
		else
	  return false;	
}
void SendSimReg2Modem(void)
{
  memcpy (AT_bufferTransmit,CREG,sizeof(CREG));
  SendData2Modem(AT_bufferTransmit,sizeof(CREG)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 3000, 1); // start y.f. timer for end of chars  
	
}
bool CheckSimReadyResp(uint8_t* pDdataIn, uint16_t Len)
{
	 char *ret;
  //   ret = strstr((char*)pDdataIn,(char*)SIM_RESP);
     ret = strstr((char*)pDdataIn,(char*)SIM_RESP);
	 if (ret == NULL )
	 {    
		  return false;
	 }
     else return true;
}

bool CheckDLMSHearBit(uint8_t* pDdataIn, uint16_t Len)
{
     if (ExtractWrapper(pDdataIn,Len) == HEARTBIT_RESPONSE)
       return true;
     return false;
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

void SendCOMMAND_MODE2Modem(void)
{
  memcpy (AT_bufferTransmit,COMMAND_MODE,sizeof(COMMAND_MODE));
  SendData2Modem(AT_bufferTransmit,sizeof(COMMAND_MODE)); // return to data mode 
  ql_rtos_timer_start(Gsm_debounce_timer, 3000, 1); // start y.f. timer for end of chars  

}

void SMS_Process(void)
{
   switch  (SMS_ON)
   {
      case 0:
        ql_rtos_timer_start(Gsm_debounce_timer, 1200, 1); // start y.f. timer for end of chars  
        SMS_ON++;
        break;
      case 1:
        SendCOMMAND_MODE2Modem(); // turn off transparent mode and go back to command mode for sms read
        ql_rtos_timer_start(Gsm_debounce_timer, 3400, 1); // start y.f. timer for end of chars  

        SendRetry = 100;
        SMS_ON++;
        break;
      case 2:
        ReadSmsMsg();
        ql_rtos_timer_start(Gsm_debounce_timer, 5100, 1); // start y.f. timer for end of chars  

        SMS_ON++;
        break;
      case 3:
          SendATE02Modem(); // 5/1/2022 y.f. avoid echo 
          SMS_ON++;
          ql_rtos_timer_start(Gsm_debounce_timer, 1100, 1); // start y.f. timer for end of chars  

          break;
      case 4:    
        Senddatamode();
        ql_rtos_timer_start(Gsm_debounce_timer, 120000, 1); // start y.f. timer for end of chars  
        SMS_ON = 0;
        SendRetry = 10;
        break;
   }
 
}   
   

void SendQIOPEN2Modem(void)
{
  
  AT_bufferTransmit[0]=0;
  strcat((char*)AT_bufferTransmit , QIOPEN_HEADER);
  strcat((char*)AT_bufferTransmit , RepeaterConfig.GsmConfig.ServerIP);
  strcat((char*)AT_bufferTransmit,(char*)",");
  strcat((char*)AT_bufferTransmit , RepeaterConfig.GsmConfig.Port);
  strcat((char*)AT_bufferTransmit , QIOPEN_FOOTER);
  SendData2Modem(AT_bufferTransmit,strlen((char*)AT_bufferTransmit));
  ql_rtos_timer_start(Gsm_debounce_timer, 20000, 1); // start y.f. timer for end of chars  
 
}
void SendQICLOSE2Modem(void)
{

  memcpy (AT_bufferTransmit,QICLOSE,sizeof(QICLOSE));
  SendData2Modem(AT_bufferTransmit,sizeof(QICLOSE)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 20000, 1); // start y.f. timer for end of chars  
   
 
}

void SendConfig2Modem(void)
{
 

  // 8/6/23 temp to be removed and used the original RepeaterConfig  
  memcpy (AT_bufferTransmit,QICSGP,sizeof(QICSGP));
  SendData2Modem(AT_bufferTransmit,sizeof(QICSGP)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 4000, 1); // start y.f. timer for end of chars  
#if 0
  AT_bufferTransmit[0]=0;
  strcat((char*)AT_bufferTransmit , CGDCONT_HEADER);
  strcat((char*)AT_bufferTransmit , CGDCONT);

 
  strcat((char*)AT_bufferTransmit , RepeaterConfig.GsmConfig.APN);
  strcat((char*)AT_bufferTransmit,(char*)",");
  strcat((char*)AT_bufferTransmit , RepeaterConfig.GsmConfig.ServerIP);

  strcat((char*)AT_bufferTransmit , CGDCONT_FOOTER);
  SendData2Modem(AT_bufferTransmit,strlen((char*)AT_bufferTransmit));

  ql_rtos_timer_start(Gsm_debounce_timer, 3000, 1); // start y.f. timer for end of chars  
#endif
}
void SendATE02Modem(void)
{
  memcpy (AT_bufferTransmit,ATE0,sizeof(ATE0));
  SendData2Modem(AT_bufferTransmit,sizeof(ATE0)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  

}
void SendCGREG2Modem(void)
{
  memcpy (AT_bufferTransmit,CGREG,sizeof(CGREG));
  SendData2Modem(AT_bufferTransmit,sizeof(CGREG)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  

}
void SendQIACT_REQ2Modem(void)
{
  memcpy (AT_bufferTransmit,QIACT_REQ,sizeof(QIACT_REQ));
  SendData2Modem(AT_bufferTransmit,sizeof(QIACT_REQ)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  
}
void SendQIACT_SET2Modem(void)
{
  memcpy (AT_bufferTransmit,QIACT_SET,sizeof(QIACT_SET));
  SendData2Modem(AT_bufferTransmit,sizeof(QIACT_SET)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  

}
void ReadSmsMsg(void)
{
  memcpy (AT_bufferTransmit,CMGR,sizeof(CMGR));
  SendData2Modem(AT_bufferTransmit,sizeof(CMGR)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  

}
void DelSmsMsg(void)
{
  memcpy (AT_bufferTransmit,CMGD,sizeof(CMGD));
  SendData2Modem(AT_bufferTransmit,sizeof(CMGD)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  

}
void SendATD(void)
{
  memcpy (AT_bufferTransmit,ATD,sizeof(ATD));
  SendData2Modem(AT_bufferTransmit,sizeof(ATD)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  
}
void Senddatamode(void)
{
  memcpy (AT_bufferTransmit,DATA_MODE,sizeof(DATA_MODE));
  SendData2Modem(AT_bufferTransmit,sizeof(DATA_MODE)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  
  
}
void sendCMEVerbouse(void)
{
  memcpy (AT_bufferTransmit,CMERR,sizeof(CMERR));
  SendData2Modem(AT_bufferTransmit,sizeof(CMERR)-1);
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  	
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
  SendCOMMAND_MODE2Modem();   
 // y.f. removed memcpy (AT_bufferTransmit,MODEM_RESET,sizeof(MODEM_RESET));
 // y.f. removed SendData2Modem(AT_bufferTransmit,sizeof(MODEM_RESET)-1);
  SendRetry = SEND_RETRY_TIMES;
  ConnectionState = MODEM_DISCONNECT_STATE;
  sprintf(str,"MODEM_DISCONNECT_STATE\r\n");
  Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
  ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  

}  

void SetSMSconfig(uint8_t smsstate)
{
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
      case 4:
        break;
        
     }
	 ql_rtos_timer_start(Gsm_debounce_timer, 2000, 1); // start y.f. timer for end of chars  

  //GsmTimer = 200; 
}
  uint8_t sendsmstest(uint8_t * pDdataIn,bool ack_test)
  {

    // retrieve caller phon number 
    uint8_t bInd=0,tmp=0;
    uint8_t SmsPhon[20];
    AT_bufferTransmit[0]=0;
    strcat((char*)AT_bufferTransmit , SMS_TEST_SEND_HEADER);
    memset(SmsPhon,0,sizeof(SmsPhon));
    while (((pDdataIn[bInd]!= 0x22)|| (pDdataIn[bInd+1]!= 0x2b)) && (bInd <50)) // find first '"+'
           bInd++;
     bInd++;
   
    if (ack_test)
    {
      if (bInd>=50) return 0;
      while ((pDdataIn[bInd]!= 0x22)&& (bInd <140))  // find second '"' sample "+972546262888"
           SmsPhon[tmp++] = pDdataIn[bInd++];
    
      strcat((char*)AT_bufferTransmit , (char*)SmsPhon);
    }
    else
    {
      strcat((char*)AT_bufferTransmit , (char*)yossiphon);
    }
    strcat((char*)AT_bufferTransmit , SMS_TEST_SEND_FOOTER);
    if (ack_test)
      strcat ((char*)AT_bufferTransmit,"retrofit ack configuration");
    else
      strcat ((char*)AT_bufferTransmit,"          Start Modem");
    strcat((char*)AT_bufferTransmit,(char*)&CONTROL_Z);
    SendData2Modem(AT_bufferTransmit,strlen((char*)AT_bufferTransmit));
 	ql_rtos_task_sleep_ms(50);
    return 1;
 }
  void sendfreesms(uint8_t * pDdataIn)
  {
    // retrieve caller phon number 
    uint8_t SmsPhon[20];
    AT_bufferTransmit[0]=0;
    strcat((char*)AT_bufferTransmit , SMS_TEST_SEND_HEADER);
    memset(SmsPhon,0,sizeof(SmsPhon));
    strcat((char*)AT_bufferTransmit , (char*)yossiphon);
    strcat((char*)AT_bufferTransmit , SMS_TEST_SEND_FOOTER);
    strcat ((char*)AT_bufferTransmit,"retrofit ack configuration");
    strcat ((char*)AT_bufferTransmit,(char*)pDdataIn);
    strcat((char*)AT_bufferTransmit,(char*)&CONTROL_Z);
    SendData2Modem(AT_bufferTransmit,strlen((char*)AT_bufferTransmit));
	ql_rtos_task_sleep_ms(50);

 }


bool  ReadConfigFromSMS(char *pDdataIn, uint16_t Len)
{
    char *position;   
    uint16_t bInd=0;
    position = (strstr((char*)pDdataIn,",2,"));
    position = position+3;
    // extract server ip Ex 31.22.22.154
    for (bInd = 0 ;bInd < 17 ; bInd++)
    {
        if ((char)(*position) ==',')
            break; 
        if (((char)(*position) =='.') ||
           ((*position <= 0x39) && (*position >=0x30)))
            RepeaterConfig.GsmConfig.ServerIP[bInd]=*position++; 
        else  
          return 0; // not valid char 
   } 
  
   RepeaterConfig.GsmConfig.ServerIP[bInd]=0; 
   position++;

   for (bInd = 0 ;bInd < 6 ; bInd++)
   {
        if ((char)(*position) ==',')
            break; 
        if   ((*position <= 0x39) && (*position >=0x30))
              RepeaterConfig.GsmConfig.Port[bInd]=*position++; 
        else 
          return 0; // not valid char 
   } 
 
   RepeaterConfig.GsmConfig.Port[bInd]=0; 
     position++;
   for (bInd = 0 ;bInd < 20 ; bInd++) // max apn chars
   {
     if (*position ==',') 
     {
        RepeaterConfig.GsmConfig.APN[bInd] = 0;
        return 1;
     }
     RepeaterConfig.GsmConfig.APN[bInd] = *position++;
   }
//   RepeaterConfig.GsmConfig.APN[bInd] = 0;
   return 0;
}    
  


void CheckInSMS(uint8_t *pDdataIn , uint16_t Len )
{
			char *   position;   
		position=strstr((char*)pDdataIn,(char *)SMS_IN_HEADER);
		if (position!=NULL)
		{
		  ql_rtos_task_sleep_ms(50);
		  DelSmsMsg();
		  if (ReadConfigFromSMS(position, Len))
		  {// enter to flash only if valid configuration is found 
			sendsmstest(pDdataIn,true);
			WriteInternalFlashConfig(); // store the configuration
	   //     HAL_NVIC_SystemReset(); // after configuration read from sms reset mcu is needed
		  }
		}
		else
		if (strstr((char*)pDdataIn,"+CMGR"))
		{ // other sms should be simple delete 
		  DelSmsMsg();
		}
}