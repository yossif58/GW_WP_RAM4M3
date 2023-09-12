/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
/*=================================================================

                        EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN              WHO         WHAT, WHERE, WHY
------------     -------     -------------------------------------------------------------------------------

=================================================================*/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_api_nw.h"

#include "ql_log.h"
#include "ql_api_datacall.h"
#include "sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/ip6_addr.h"

#include "lwip/netdb.h"
#include "lwip/netif.h"
#include "lwip/inet.h"
#include "lwip/tcp.h"

#include "ql_ssl.h"
#include "longi4g.h"
#include "longiWrapper.h"
#define QL_SOCKET_LOG_LEVEL	                QL_LOG_LEVEL_INFO
#define QL_SOCKET_LOG(msg, ...)			    QL_LOG(QL_SOCKET_LOG_LEVEL, "ql_Sock", msg, ##__VA_ARGS__)
#define QL_SOCKET_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_SSL", msg, ##__VA_ARGS__)

#define QL_SOCKET_DEMO_CHANGE_DEMO 0

#if QL_SOCKET_DEMO_CHANGE_DEMO
#define QL_SOCKET_DEMO_URL "www.baidu.com"
#else
#define QL_SOCKET_DEMO_URL "220.180.239.212"
#endif
#pragma pack(1)
#define TCP_SERVER_DOMAIN "220.180.239.212"
#define TCP_SERVER_PORT 8064
static void setstatdebug(uint8_t ind);
static ql_task_t socket_demo_task = NULL;
//static char send_buf[1024]={0};
//static int  send_len = 0;
static char recv_buf[1024]={0};
static int  recv_len = 0;
static uint32_t extractwrapperres=NOT_RCOGNAIZE_MSG;
int socket_fd = -1;
char str [1024];
extern RepeaterConfigStruct RepeaterConfig;
extern uint32_t ExtractWrapper(uint8_t *pMsgIn, uint16_t len);
extern uint32_t SendHeartBit2Server(void );
extern uint32_t sendlastgasp(void);
extern void Send2DebugUart(uint8_t *DataIn, uint16_t Len);
extern ql_timer_t Gsm_debounce_timer;
extern bool Gsm_timer_event;
extern uint16_t blinkFlag;
extern bool last_gasp;
extern bool ConfigChanged;
int connected = 0; // y.f. removed to upper level from socket_app_thread 
void SendData2Modem(uint8_t * DataIn, uint16_t Len);
//获取socket接收缓存的剩余大小
int ql_socket_get_free_size(int s,void *value)
{
    int ret = ERR_OK;
    if (s < 0||value==NULL)
    {
        return -1;
    }
    *((int32_t *)value) =lwip_getRecvBufSize(s)-lwip_getRecvAvailSize(s);
    QL_SOCKET_LOG("socket free size:%d", *((int32_t *)value));
    if(*((int32_t *)value)<0)
    {
        ret = ERR_VAL;
    }
    return ret;
}

//获取socket发送缓存中已发送但未收到回应的数据大小
int ql_socket_get_unacked_size(int s,void *value)
{
    int ret = ERR_OK;
    if (s < 0||value==NULL)
    {
        return -1;
    }
    *((int32_t *)value) =lwip_getSentSize(s)-lwip_getAckedSize(s);
    QL_SOCKET_LOG("socket unacked size:%d", *((int32_t *)value));
    if(*((int32_t *)value)<0)
    {
        ret = ERR_VAL;
    }
    return ret;
}

static void socket_app_thread(void * arg)
{
	int ret = 0;
	int i = 0;
	int profile_idx = 1;
    ql_data_call_info_s info;
	char ip4_addr_str[16] = {0};
	fd_set read_fds;
	fd_set write_fds;
	fd_set exp_fds;
	unsigned long long  tempu32=0;
	int flags = 0;
	int fd_changed = 0;
	
	int closing = false;
	struct sockaddr_in local4, server_ipv4;	
	struct addrinfo *pres = NULL;	
	struct addrinfo *temp = NULL;
	uint8_t nSim = 0;
    ql_nw_data_count_info_s data_count_info = {0};
	ql_nw_reg_status_info_s nw_info = {0}; // y.f. 3/8/23 add to check network state 
#if QL_SOCKET_DEMO_CHANGE_DEMO
	ql_datacall_dns_info_s dns_pri = {0};
	ql_datacall_dns_info_s dns_sec = {0};
#endif	
loop_0_registration:
	ql_rtos_task_sleep_s(10);
	sprintf(str,"\r\n wait for network register done\r\n"); 
	blinkFlag = WAIT_FOR_REGISTRATION;
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	while((ret = ql_network_register_wait(nSim, 120)) != 0 && i < 10){
    	i++;
		ql_rtos_task_sleep_s(1);
	}
	if(ret == 0){
		i = 0;
		blinkFlag = CONNECION_2_PROVIDER;
		sprintf(str,"\r\n=====network registered!!!!=========\r\n"); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}else{
		sprintf(str,"\r\n network register failure!!!!!====\r\n"); 
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
		goto exit;
	}
	ql_set_data_call_asyn_mode(nSim, profile_idx, 0);

	sprintf(str,"\r\n start data call====\r\n"); 
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
//	ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "uninet", NULL, NULL, 0); 
	ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, &RepeaterConfig.GsmConfig.APN[0], NULL, NULL, 0);  // y.f. 18/6/23 
    sprintf(str,"\r\n data call result ====\r\n"); 
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	if(ret != 0){
	   sprintf(str,"\r\n data call failure ====\r\n"); 
       Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}
	memset(&info, 0x00, sizeof(ql_data_call_info_s));
	
	ret = ql_get_data_call_info(nSim, profile_idx, &info);
	if(ret != 0){
        sprintf(str,"\r\n ql_get_data_call_info ret: %d\r\n", ret); 
        Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
		ql_stop_data_call(nSim, profile_idx);
		goto exit;
	}
	inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
	inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
	inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
	sprintf(str,"info->profile_idx: %d", info.profile_idx);
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
	sprintf(str,"info->ip_version: %d", info.ip_version);
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
	sprintf(str,"info->v4.state: %d", info.v4.state); 
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
	sprintf(str,"info.v4.addr.ip: %s\r\n", ip4_addr_str);
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
	sprintf(str,"info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
	sprintf(str,"info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
	
	memset(&local4, 0x00, sizeof(struct sockaddr_in));
	local4.sin_family = AF_INET;
	local4.sin_port = 0;
	inet_aton(ip4addr_ntoa(&info.v4.addr.ip), &local4.sin_addr);	
loop:
    connected = 0; // y.f 21/6/23 
//	ret = getaddrinfo_with_pcid(TCP_SERVER_DOMAIN, NULL, NULL, &pres, (uint32)profile_idx);
	ret = getaddrinfo_with_pcid(&RepeaterConfig.GsmConfig.ServerIP[0], NULL, NULL, &pres, (uint32)profile_idx);
//	ret = getaddrinfo_with_pcid("31.154.79.218", NULL, NULL, &pres, (uint32)profile_idx);
	if (ret < 0 || pres == NULL) 
	{
		sprintf(str,"\r\n DNS getaddrinfo failed! ret=%d; pres=%p!\n",ret,pres); 
        Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
		goto exit;
	}

	FD_ZERO(&read_fds);
	FD_ZERO(&write_fds);
	FD_ZERO(&exp_fds);
	flags = 0;

	closing = false;
	i = 0;
    extractwrapperres = NOT_RCOGNAIZE_MSG;	
	for(temp = pres; temp != NULL; temp = temp->ai_next){
 		struct sockaddr_in * sin_res = (struct sockaddr_in *)temp->ai_addr;
		if(temp->ai_family == AF_INET){
			socket_fd = socket(temp->ai_family, SOCK_STREAM, 0);
			if(socket_fd < 0){
				continue;
			}

			ret = bind(socket_fd,(struct sockaddr *)&local4,sizeof(struct sockaddr));
			if(ret < 0){
				close(socket_fd);
				socket_fd = -1;
				continue;
			}
			
			flags |= O_NONBLOCK;
			fcntl(socket_fd, F_SETFL,flags);

			memset(&server_ipv4, 0x00, sizeof(struct sockaddr_in));
			
			server_ipv4.sin_family = temp->ai_family;
//			server_ipv4.sin_port = htons(2338);
			server_ipv4.sin_addr = sin_res->sin_addr;
		    server_ipv4.sin_port = htons((atoi((const char *)&RepeaterConfig.GsmConfig.Port)));  // y.f. 19/6/23 add remote address from repeater config/			ip4addr_aton(&RepeaterConfig.GsmConfig.ServerIP[0], (ip4_addr_t*)&(server_ipv4.sin_addr));	
			blinkFlag = CONNECTION_2_SERVER;			
			ret = connect(socket_fd, (struct sockaddr *)&server_ipv4, sizeof(server_ipv4));	
			tempu32 =  (long long unsigned )server_ipv4.sin_addr.s_addr;
			sprintf(str,"\r\nserver ip %llu ",tempu32 ); 
			
            Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 	
			ql_rtos_task_sleep_ms(200);
			tempu32 =  (long long unsigned )server_ipv4.sin_port;
			sprintf(str,"\r\nserver port %llu ",tempu32 ); 
			
            Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 	

			
			
			if(ret == 0){
				connected = 1;
			//	blinkFlag = SERVER_DATA_MODE;
				break;
			}else{
				if(lwip_get_error(socket_fd) != EINPROGRESS){
					close(socket_fd);
					socket_fd = -1;
					continue;
				}else{
					break;
				}
			}
		}
	}
	if(socket_fd < 0){
		goto exit;
	}
	if(connected == 1){
		FD_SET(socket_fd, &read_fds);	
		sprintf(str,"\r\n=====already connect to server  ====="); 
        Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
//		memset(send_buf, 0x00, 128);
//		send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
		
//		if(write(socket_fd, send_buf, send_len) != send_len){
//			FD_SET(socket_fd, &write_fds);	
//		}else{
//		i++;
//	}
			SendLogin2Server(); // y.f. 19/6/23
			ql_rtos_timer_start(Gsm_debounce_timer, 120000, 1); // start timerfor heartbit 

			i++;
	}else{
		FD_SET(socket_fd, &write_fds);	
		ql_rtos_timer_start(Gsm_debounce_timer, 600000, 1); // start timer for onother try to connect 
	}	
	FD_SET(socket_fd, &exp_fds);


	while(1)
	{   
		struct timeval t;
		t.tv_sec = 2;
		t.tv_usec = 0;
//		sprintf(str,"\r\n loop socket tread 3sec   \r\n");
//      Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
		ql_rtos_task_sleep_ms(2000);
		setstatdebug(30);
		FD_SET(socket_fd, &read_fds);	// y.f. add for test read in constant
		fd_changed = select(socket_fd+1, &read_fds, &write_fds, &exp_fds, &t);
		setstatdebug(0);
		if (ConfigChanged)
		{
			ConfigChanged = false;
			break; // y.f. 8/8/23 when changed config is detected restart communication with new server 
			
		}			
		if(fd_changed > 0)
		{
		setstatdebug(1);	

			if(FD_ISSET(socket_fd, &write_fds))
			{
				setstatdebug(2);			
				FD_CLR(socket_fd, &write_fds);
				if(connected== 0){
					int value = 0;
					int len = 0;
					setstatdebug(3);							
					len = sizeof(value);
					getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &value, &len);
						
					//QL_SOCKET_LOG("errno: %d", value);
					if(value == 0 || value == EISCONN ){
								setstatdebug(4);	
					sprintf(str,"\r\n=====connect to server  success====="); 						
					Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 		
        			ql_rtos_timer_start(Gsm_debounce_timer, 120000, 1); // start timerfor heartbit 
					i++;						
					connected = 1;		
					blinkFlag = SERVER_DATA_MODE;					
					FD_SET(socket_fd, &read_fds);	
					SendLogin2Server(); // y.f. 19/6/23
							setstatdebug(5);	
#if 0						
						QL_SOCKET_LOG("=====connect to \"220.180.239.212:8252\" success=====");
						connected = 1;						
						FD_SET(socket_fd, &read_fds);	

						memset(send_buf, 0x00, 128);
						send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
						write(socket_fd, send_buf, send_len);
						i++;
#endif						
					}else{
								setstatdebug(6);	
						sprintf(str,"\r\n=====connect to server failed====="); 						
					    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 	
					    uint32_t tempu32 =  (uint32_t)server_ipv4.sin_addr.s_addr;
		                sprintf(str,"\r\n serverip %lu : %d====",tempu32,(uint16_t )(server_ipv4.sin_port)); 	
                        Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 							
						close(socket_fd);
						socket_fd = -1;
						break;
					}
				}else{
							setstatdebug(7);	
#if 0					// y.f. don't send anything 
					memset(send_buf, 0x00, 128);
					send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
					write(socket_fd, send_buf, send_len);
					i++;
#endif					
				}						
			}else if(FD_ISSET(socket_fd, &read_fds))
			{
						setstatdebug(8);	
				FD_CLR(socket_fd, &read_fds);
			    memset(recv_buf,0x00, 128);
				recv_len = read(socket_fd, recv_buf, 128);
				if(recv_len > 0){
							setstatdebug(9);	
					blinkFlag = DATA_IN_SERVER;
#if 0					
					QL_SOCKET_LOG(">>>>Recv: %s", recv_buf);
					memset(send_buf, 0x00, 128);
					send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
					write(socket_fd, send_buf, send_len);
					i++;
#endif				
					setstatdebug(10);	
					Send2DebugUart((uint8_t *)recv_buf,recv_len); 
					extractwrapperres = ExtractWrapper((uint8_t *)recv_buf,recv_len);
					if ((extractwrapperres == LOGIN_RESPONSE) ||  (extractwrapperres == HEARTBIT_RESPONSE) || (extractwrapperres==DLMS_MSG))
					{
						sprintf(str,"\r\nstart heartbit timer pos 0");
						Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 		

						ql_rtos_timer_start(Gsm_debounce_timer, 120000, 1); // start y.f. timer for end of chars  
//						SendData2Modem((uint8_t *)recv_buf,recv_len);  // temp caution to be remoevd 3/7/23 
					}
					else	
					{
								setstatdebug(11);	
						sprintf(str,"\r\nNot proper DLMS msg received");
						Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 		
					}						
					FD_SET(socket_fd, &read_fds);
							setstatdebug(12);	
					if(ql_nw_get_data_count(nSim, &data_count_info) == 0)
					{
						sprintf(str,"data count: uplink %llu, downlink %llu", data_count_info.uplink_data_count, data_count_info.downlink_data_count);
						Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 		
					}
				}else if(recv_len == 0){
							setstatdebug(13);	
					if(closing == false){
								setstatdebug(14);	
						sprintf(str,"\r\n===passive close!!!!"); 						
						Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 		
						shutdown(socket_fd, SHUT_WR);
						closing = true;
						FD_SET(socket_fd, &read_fds);	
					}else{
								setstatdebug(15);	
						close(socket_fd);
						socket_fd = -1;
						sprintf(str,"\r\n===socket closed!!!!"); 						
						Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 						
						break;
					}
				}else{
							setstatdebug(16);	
					if(lwip_get_error(socket_fd) == EAGAIN){
								setstatdebug(17);	
						FD_SET(socket_fd, &read_fds);	
					}else{
								setstatdebug(18);	
						close(socket_fd);
						socket_fd = -1;
						sprintf(str,"\r\n=====break close socket lwip error====="); 						
					    Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 	
						
						break;
					}
				}
			}else if(FD_ISSET(socket_fd, &exp_fds)){
						setstatdebug(19);	
				FD_CLR(socket_fd, &exp_fds);
				close(socket_fd);
				socket_fd = -1;
				sprintf(str,"\r\n===exeption found!!!!"); 						
				Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 						
				break;
			}
	//				setstatdebug(20);	
		}
		else
		{
//			sprintf(str,"\r\n select timeout \r\n");
 //           Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
		}
	//			setstatdebug(21);	
		// check if heartbit timeout
		if (last_gasp)
		{
				last_gasp = false;
				sendlastgasp();
				ql_rtos_timer_start(Gsm_debounce_timer, 120000, 1); // start timerfor heartbit 
		}
		else	
		if((Gsm_timer_event)&& (connected== 1))
		{
					setstatdebug(22);	
//			sprintf(str,"\r\n send heartbit timeout \r\n");
//            Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
			Gsm_timer_event=false;
		//    sendaaretemp();  // temp for test 
	
		      SendHeartBit2Server();
		//	sendlastgasp();
		    ql_rtos_timer_start(Gsm_debounce_timer, 120000, 1); // start timerfor heartbit 
		}
		else if ((connected== 0) && (Gsm_timer_event)) // y.f. try connect again from loop:
		{	
				setstatdebug(23);
				sprintf(str,"\r\n start connection to server again every 10 min ");
				Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
				break;
		}
	}
	sprintf(str,"\r\n break from loop  \r\n");
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
		setstatdebug(24);	
    if(pres!=NULL)
    {
		setstatdebug(25);	
        freeaddrinfo(pres);
        pres = NULL;
    }
	ret = ql_nw_get_reg_status(nSim, &nw_info); // y.f check if network is stil registreted 
    sprintf(str,"ret: 0x%x, current data reg status=%d", ret, nw_info.data_reg.state);
    if((QL_NW_REG_STATE_HOME_NETWORK != nw_info.data_reg.state) && (QL_NW_REG_STATE_ROAMING != nw_info.data_reg.state))   
    {
       goto loop_0_registration;
    }
	goto loop;
exit:  
		setstatdebug(26);	
   ql_rtos_task_delete(socket_demo_task);	
   sprintf(str,"\r\n exit from socket task  \r\n");
   Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
   return;	
}




int ql_socket_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
// y.f. temp enter to be removed for test only repeater with default value.
#if 0
    memset((uint8_t*)&RepeaterConfig.GsmConfig,0,sizeof(RepeaterConfig.GsmConfig));
    memcpy (RepeaterConfig.GsmConfig.APN ,"internet",8);
    memcpy (RepeaterConfig.GsmConfig.Port ,"2339",4);
    memcpy (RepeaterConfig.GsmConfig.ServerIP ,"31.154.79.218",13);
#endif

    err = ql_rtos_task_create(&socket_demo_task, 16*1024, APP_PRIORITY_ABOVE_NORMAL, "QsocketApp", socket_app_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		sprintf(str,"\r\nsocket_app init failed \r\n");
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}
	return err;
}

void SendData2Modem(uint8_t * DataIn, uint16_t Len)
{
	int ret = 0;
	fd_set read_fds;
#if 0	
	sprintf(str,"\r\n send data to modem  to send data to modem \r\n ");
	Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	ql_rtos_task_sleep_ms(20);
#endif	
	FD_SET(socket_fd, &read_fds);	
	if (connected ==1)
	{
	
		ret = write(socket_fd, (const void*)DataIn, Len);
		if(ret < 0)
		{
			sprintf(str,"failes to send data to modem \r\n");
			Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
		}
	}
	Send2DebugUart((uint8_t *) DataIn, Len);  

}

void setstatdebug(uint8_t ind)
{
#if 0
			sprintf(str,"\r\n ind = %d!!!!",ind); 						
			Send2DebugUart((uint8_t *)str,strlen((const char *)str));
			ql_rtos_task_sleep_ms(20);			
#endif			
}


