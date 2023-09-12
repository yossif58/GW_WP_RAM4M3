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

#include "ql_log.h"
#include "ql_api_datacall.h"
#include "ql_http_client.h"

#define QL_HTTP_LOG_LEVEL           	QL_LOG_LEVEL_INFO
#define QL_HTTP_LOG(msg, ...)			QL_LOG(QL_HTTP_LOG_LEVEL, "ql_HTTP", msg, ##__VA_ARGS__)
#define QL_HTTP_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_HTTP", msg, ##__VA_ARGS__)


ql_task_t http_post_task = NULL;
static http_client_t  http_cli = 0;
static ql_sem_t  http_semp;

static void http_event_cb(http_client_t *client, int evt, int evt_code, void *arg)
{
	QL_HTTP_LOG("*client:%d, http_cli:%d", *client, http_cli);
	if(*client != http_cli)
		return;
	QL_HTTP_LOG("evt:%d, evt_code:%d", evt, evt_code);
	switch(evt){
	case HTTP_EVENT_SESSION_ESTABLISH:{
			if(evt_code != HTTP_SUCCESS){
				QL_HTTP_LOG("HTTP session create failed!!!!!");
				ql_rtos_semaphore_release(http_semp);
			}
		}
		break;
	case HTTP_EVENT_RESPONE_STATE_LINE:{
			if(evt_code == HTTP_SUCCESS){
				int resp_code = 0;
				int content_length = 0;
				int chunk_encode = 0;
				char *location = NULL;
				ql_httpc_getinfo(client, HTTP_INFO_RESPONSE_CODE, &resp_code);
				QL_HTTP_LOG("response code:%d", resp_code);
				ql_httpc_getinfo(client, HTTP_INFO_CHUNK_ENCODE, &chunk_encode);
				if(chunk_encode == 0){
					ql_httpc_getinfo(client, HTTP_INFO_CONTENT_LEN, &content_length);
					QL_HTTP_LOG("content_length:%d",content_length);
				}else{
					QL_HTTP_LOG("http chunk encode!!!");
				}

				if(resp_code >= 300 && resp_code < 400){
					ql_httpc_getinfo(client, HTTP_INFO_LOCATION, &location);
					QL_HTTP_LOG("redirect location:%s", location);
					free(location);
				}
			}
		}
		break;
	case HTTP_EVENT_SESSION_DISCONNECT:{
			if(evt_code == HTTP_SUCCESS){
				QL_HTTP_LOG("===http transfer end!!!!");
			}else{
				QL_HTTP_LOG("===http transfer occur exception!!!!!");
			}			
			ql_rtos_semaphore_release(http_semp);
		}
		break;
	}
}

static int http_write_response_data(http_client_t *client, void *arg, char *data, int size, unsigned char end)
{
    unsigned char *recbuff=NULL;
    QL_HTTP_LOG("recv size:%d", size);
    recbuff=malloc(size+1);
    if(NULL!=recbuff)
	{	
        memset(recbuff, 0, size+1);
        memcpy(recbuff, data, size);
    	QL_HTTP_LOG("recv data: %s", recbuff);
        free(recbuff);
        recbuff=NULL;
    }
	return size;
}
#if 1
static int http_read_request_data(http_client_t *client, void *arg, char *data, int size)
{
	QL_HTTP_LOG("size: %d,data: %s", size,  (char *)arg);
	strncpy(data, (char *)arg, size);
	return size;
}
#endif
static int sim_net_register(int nSim, int profile_idx)
{
	int ret = 0, i = 0;
	ql_data_call_info_s info;
	char ip4_addr_str[16] = {0};

	while((ret = ql_network_register_wait(nSim, 120)) != 0 && i < 10){
		i++;
		ql_rtos_task_sleep_ms(1000);
	}
	if(ret == 0) {
		i = 0;
		QL_HTTP_LOG("====network registered!!!!====");
	} else {
		QL_HTTP_LOG("====network register failure!!!!!====");
		return ret;
	}
	ql_set_data_call_asyn_mode(nSim, profile_idx, 0);
	ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "uninet", NULL, NULL, 0); 
	if(ret != 0) {
		QL_HTTP_LOG("====data call failure!!!!=====");
	}
	memset(&info, 0x00, sizeof(ql_data_call_info_s));
	
	ret = ql_get_data_call_info(nSim, profile_idx, &info);
	if(ret != 0) {
		QL_HTTP_LOG("ql_get_data_call_info ret: %d", ret);
		ql_stop_data_call(nSim, profile_idx);
		return ret;
	}
		
	QL_HTTP_LOG("info->profile_idx: %d", info.profile_idx);
	QL_HTTP_LOG("info->ip_version: %d", info.ip_version);
					
	QL_HTTP_LOG("info->v4.state: %d", info.v4.state); 
	inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
	QL_HTTP_LOG("info.v4.addr.ip: %s\r\n", ip4_addr_str);
		
	inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
	QL_HTTP_LOG("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);
		
	inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
	QL_HTTP_LOG("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);
	return ret;
}

//	ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_REQUEST_HEADER, "Content-type: application/json");
		//ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_REQUEST_HEADER, "Content-type: application/x-www-from-urlencoded");

static void http_post_app_thread(void * arg)
{
    int ret = 0;
    int profile_idx = 1;
	int run_num = 0;
	uint8_t nSim = 0;
	char url[] = "http://220.180.239.212:8300";
	char *post_body = "hardware=V5001";
	char *username = "admin";
	char *password = "admin";
	char author_info[64] = {0};
	snprintf(author_info, 64, "%s:%s", username, password);
	
	ql_rtos_task_sleep_s(5);
	QL_HTTP_LOG("========== http new demo start ==========");
	QL_HTTP_LOG("wait for network register done");
	ret = sim_net_register(nSim, profile_idx);
	if( ret != 0 ) {
		QL_HTTP_LOG("sim register net failure , ret= %d", ret);
		goto exit;
	}
	ql_rtos_semaphore_create(&http_semp, 0);
    
	do {
		QL_HTTP_LOG("==============http_client_post_test[%d]================\n",run_num+1);
		if(ql_httpc_new(&http_cli, http_event_cb, NULL) != HTTP_SUCCESS){
			QL_HTTP_LOG("http client create failed!!!!");
			break;
		}
		
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_SIM_ID, nSim);
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_PDPCID, profile_idx);
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_BASIC_AUTH, author_info);
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_WRITE_FUNC, http_write_response_data);
		
			
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_METHOD, HTTP_METHOD_POST);
		//ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_REQUEST_HEADER, "application/x-www-form-urlencoded");
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_READ_FUNC, http_read_request_data);
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_READ_DATA, post_body);
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_UPLOAD_LEN, strlen(post_body));
		 
		ql_httpc_setopt(&http_cli, HTTP_CLIENT_OPT_URL, (char *)url);
		if( ql_httpc_perform(&http_cli) == HTTP_SUCCESS ) {
			QL_HTTP_LOG("wait http perform end!!!!!!");
			ql_rtos_semaphore_wait(http_semp, QL_WAIT_FOREVER);
		} else {
			QL_HTTP_LOG("http perform failed!!!!!!!!!!");
		}
		
		ql_httpc_release(&http_cli);
		http_cli = 0;
		QL_HTTP_LOG("==============http_client_test_end[%d]================\n",run_num+1);
		run_num++;
		ql_rtos_task_sleep_s(5);
	}while(1);
exit:
    ql_rtos_semaphore_delete(http_semp);
	
  	ql_rtos_task_delete(http_post_task);	
    return;		
}

void ql_http_post_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;
    
    err = ql_rtos_task_create(&http_post_task, 4096, APP_PRIORITY_REALTIME, "QhttpPost", http_post_app_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
	{
		QL_HTTP_LOG("created task failed");
	}
}

