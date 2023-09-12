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

#define QL_SOCKET_LOG_LEVEL	                QL_LOG_LEVEL_INFO
#define QL_SOCKET_LOG(msg, ...)			    QL_LOG(QL_SOCKET_LOG_LEVEL, "ql_Sock", msg, ##__VA_ARGS__)
#define QL_SOCKET_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_SSL", msg, ##__VA_ARGS__)

#define QL_SOCKET_DEMO_CHANGE_DEMO 0

#if QL_SOCKET_DEMO_CHANGE_DEMO
#define QL_SOCKET_DEMO_URL "www.baidu.com"
#else
#define QL_SOCKET_DEMO_URL "220.180.239.212"
#endif
extern RepeaterConfigStruct RepeaterConfig;

static ql_task_t socket_demo_task = NULL;
static char send_buf[128]={0};
static int  send_len = 0;
static char recv_buf[128]={0};
static int  recv_len = 0;

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
	int socket_fd = -1;
	int flags = 0;
	int fd_changed = 0;
	int connected = 0;
	int closing = false;
	struct sockaddr_in local4, server_ipv4;	
	struct addrinfo *pres = NULL;	
	struct addrinfo *temp = NULL;
	uint8_t nSim = 0;
	ql_nw_data_count_info_s data_count_info = {0};
#if QL_SOCKET_DEMO_CHANGE_DEMO
	ql_datacall_dns_info_s dns_pri = {0};
	ql_datacall_dns_info_s dns_sec = {0};
#endif	
	
	while (1)
	{
		switch(ConnectionState)
		{	
			case MODEM_DISCONNECT_STATE:
			    ql_rtos_task_sleep_s(10);
				sprintf(str,"\r\n== socket demo start wait for network register done==========\r\n"); 
				Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
					
				while((ret = ql_network_register_wait(nSim, 120)) != 0 && i < 10){
					i++;
					ql_rtos_task_sleep_s(1);
				}
				if(ret == 0){
					i = 0;
					sprintf(str,"\r\n=====network registered!!!!=========\r\n"); 
					Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
				}else{
					sprintf(str,"\r\nnetwork register failure!!!!!====\r\n"); 
					Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
					break;   //y.f. 19/6/23  goto exit;
				}

				ql_set_data_call_asyn_mode(nSim, profile_idx, 0);
				sprintf(str,"\r\===start data call=======\r\n"); 
				Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
				ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, &RepeaterConfig.GsmConfig.APN[0], NULL, NULL, 0);  // y.f. 18/6/23 
				sprintf(str,"\r\n===data call result:%d", ret"); 
				Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 
				if(ret != 0){
					sprintf(str,"\r\n===data call failure!!!!\r\n"); 
					Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 
				}
				memset(&info, 0x00, sizeof(ql_data_call_info_s));
				ret = ql_get_data_call_info(nSim, profile_idx, &info);
				if(ret != 0){
					QL_SOCKET_LOG("ql_get_data_call_info ret: %d", ret);
					ql_stop_data_call(nSim, profile_idx);
					break; // y.f. 19/6/23 goto exit;
				}
				memset(&local4, 0x00, sizeof(struct sockaddr_in));
				local4.sin_family = AF_INET;
				local4.sin_port = 0;
				inet_aton(ip4addr_ntoa(&info.v4.addr.ip), &local4.sin_addr);	
			loop:
				ret = getaddrinfo_with_pcid(QL_SOCKET_DEMO_URL, NULL, NULL, &pres, (uint32)profile_idx);
				if (ret < 0 || pres == NULL) 
				{
					QL_SOCKET_LOG("DNS getaddrinfo failed! ret=%d; pres=%p!\n",ret,pres);
					goto exit;
				}

				FD_ZERO(&read_fds);
				FD_ZERO(&write_fds);
				FD_ZERO(&exp_fds);
				flags = 0;
				connected = 0;
				closing = false;
				i = 0;
				for(temp = pres; temp != NULL; temp = temp->ai_next){
				//	struct sockaddr_in * sin_res = (struct sockaddr_in *)temp->ai_addr;
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
			#if 0
			// y.f. add remote server from repeater config 			
						server_ipv4.sin_family = temp->ai_family;
						server_ipv4.sin_port = htons(8252);
						server_ipv4.sin_addr = sin_res->sin_addr;
			#endif
						server_ipv4.sin_family = temp->ai_family;
			//			server_ipv4.sin_port = htons(8252);
			//			server_ipv4.sin_addr = sin_res->sin_addr;
						server_ipv4.sin_port = htons((atoi((const char *)&RepeaterConfig.GsmConfig.Port)));
						ip4addr_aton(&RepeaterConfig.GsmConfig.ServerIP[0], (ip4_addr_t*)&(server_ipv4.sin_addr));		

						ret = connect(socket_fd, (struct sockaddr *)&server_ipv4, sizeof(server_ipv4));	
						
						if(ret == 0){
							connected = 1;
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
					ConnectionState = MODEM_CONNECT_STATE;	
					sprintf(str,"\r\n========connect to server \r\n"); 
					Send2DebugUart((uint8_t *)str,strlen((const char *)str)); 
					SendLogin2Server(); // y.f. 19/6/23
#if 0					
					memset(send_buf, 0x00, 128);
					send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
					
					if(write(socket_fd, send_buf, send_len) != send_len){
						FD_SET(socket_fd, &write_fds);	
					}else{
						i++;
					}
				}else{
					FD_SET(socket_fd, &write_fds);	
				}	
				FD_SET(socket_fd, &exp_fds);
			break;  // end 
			case MODEM_CONNECT_STATE:
			break;
			case MODEM_DATA_STATE:
			
				while(1)
				{
					fd_changed = select(socket_fd+1, &read_fds, &write_fds, &exp_fds, NULL);
					if(fd_changed > 0){
						if(FD_ISSET(socket_fd, &write_fds)){
							FD_CLR(socket_fd, &write_fds);
							if(connected== 0){
								int value = 0;
								int len = 0;
									
								len = sizeof(value);
								getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, &value, &len);
									
								QL_SOCKET_LOG("errno: %d", value);
								if(value == 0 || value == EISCONN ){
									QL_SOCKET_LOG("=====connect to \"220.180.239.212:8252\" success=====");
									connected = 1;						
									FD_SET(socket_fd, &read_fds);	

									memset(send_buf, 0x00, 128);
									send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
									write(socket_fd, send_buf, send_len);
									i++;
								}else{
									QL_SOCKET_LOG("=====connect to \"220.180.239.212:8252\" failed=====");
									close(socket_fd);
									socket_fd = -1;
									break;
								}
							}else{
								memset(send_buf, 0x00, 128);
								send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
								write(socket_fd, send_buf, send_len);
								i++;
							}						
						}else if(FD_ISSET(socket_fd, &read_fds)){
							FD_CLR(socket_fd, &read_fds);
							memset(recv_buf,0x00, 128);
							recv_len = read(socket_fd, recv_buf, 128);
							if(recv_len > 0){
								QL_SOCKET_LOG(">>>>Recv: %s", recv_buf);
								memset(send_buf, 0x00, 128);
								send_len = snprintf(send_buf, 128,"%d%s%d\r\n",i,"startAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAend",i);
								write(socket_fd, send_buf, send_len);
								i++;
								FD_SET(socket_fd, &read_fds);
								if(ql_nw_get_data_count(nSim, &data_count_info) == 0)
								{
									QL_SOCKET_LOG("data count: uplink %llu, downlink %llu", data_count_info.uplink_data_count, data_count_info.downlink_data_count);
								}
							}else if(recv_len == 0){
								if(closing == false){
									QL_SOCKET_LOG("===passive close!!!!");
									shutdown(socket_fd, SHUT_WR);
									closing = true;
									FD_SET(socket_fd, &read_fds);	
								}else{
									close(socket_fd);
									socket_fd = -1;
									break;
								}
							}else{
								if(lwip_get_error(socket_fd) == EAGAIN){
									FD_SET(socket_fd, &read_fds);	
								}else{
									close(socket_fd);
									socket_fd = -1;
									break;
								}
							}
						}else if(FD_ISSET(socket_fd, &exp_fds)){
							FD_CLR(socket_fd, &exp_fds);
							close(socket_fd);
							socket_fd = -1;
							break;
						}
						
					}
				}
				break;
				if(pres!=NULL)
				{
					freeaddrinfo(pres);
					pres = NULL;
				}
				goto loop;
			}
		}
exit:  
   ql_rtos_task_delete(socket_demo_task);	
	
   return;	
}

int ql_socket_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	err = ql_rtos_task_create(&socket_demo_task, 16*1024, APP_PRIORITY_ABOVE_NORMAL, "QsocketApp", socket_app_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_SOCKET_LOG("socket_app init failed");
	}

	return err;
}
void SendData2Modem(uint8_t * DataIn, uint16_t Len)
{
	int ret = 0;
	ret = write(sock_fd, (const void*)DataIn, Len, 0);
	if(ret < 0)
	{
		sprintf(str,"failes to send data to modem \r\n");
		Send2DebugUart((uint8_t *)str,strlen((const char *)str)); // return to data mode 
	}

}



