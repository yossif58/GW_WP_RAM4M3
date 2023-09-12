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
#include "ql_mqttclient.h"

#include "ql_ssl.h"

#define QL_MQTT_LOG_LEVEL	            QL_LOG_LEVEL_INFO
#define QL_MQTT_LOG(msg, ...)			QL_LOG(QL_MQTT_LOG_LEVEL, "ql_MQTT", msg, ##__VA_ARGS__)
#define QL_MQTT_LOG_PUSH(msg, ...)	    QL_LOG_PUSH("ql_MQTT", msg, ##__VA_ARGS__)
static ql_task_t mqtt_task = NULL;

#define MQTT_CLIENT_IDENTITY        "quectel_01"
#define MQTT_CLIENT_USER            ""
#define MQTT_CLIENT_PASS            ""


#define MQTT_CLIENT_ONENET_PRODUCTID             "417661"
#define MQTT_CLIENT_ONENET_ACCESSKEY             "aggblpDBWoKg2CGdbxp/CVKt9xXWcV162hJ1+Bhzvw4="
#define MQTT_CLIENT_ONENET_DEVICENAME            "test_led"
#define MQTT_CLIENT_ONENET_VERSION               "2018-10-31"



#define MQTT_CLIENT_QUECTEL_URL                  "mqtt://220.180.239.212:8306"
#define MQTT_CLIENT_ONENET_URL                   "mqtt://mqtts.heclouds.com:1883" //onenet 的ip地址

#define MQTT_CLIENT_QUECTEL_SSL_URL              "mqtts://220.180.239.212:8307"
#define MQTT_CLIENT_ONENET_SSL_URL               "mqtts://mqttstls.heclouds.com:8883"//onenet 的ip地址

// publist 的内容
#define MQTT_PUB_MSG0 "{\"id\": 000000,\"dp\": {\"temperatrue\": [{\"v\": 0.001,}],\"power\": [{\"v\": 0.001,}]}}"
#define MQTT_PUB_MSG1 "{\"id\": 111111,\"dp\": {\"temperatrue\": [{\"v\": 1.000,}],\"power\": [{\"v\": 1.001,}]}}"
#define MQTT_PUB_MSG2 "{\"id\": 222222,\"dp\": {\"temperatrue\": [{\"v\": 2.000,}],\"power\": [{\"v\": 2.002,}]}}"

static ql_sem_t  mqtt_semp;
static int  mqtt_connected = 0;

static void mqtt_state_exception_cb(mqtt_client_t *client)
{
	QL_MQTT_LOG("mqtt session abnormal disconnect");
	mqtt_connected = 0;
}

static void mqtt_connect_result_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_e status)
{
	QL_MQTT_LOG("status: %d", status);
	if(status == 0){
		mqtt_connected = 1;
	}
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_requst_result_cb(mqtt_client_t *client, void *arg,int err)
{
	QL_MQTT_LOG("err: %d", err);
	
	ql_rtos_semaphore_release(mqtt_semp);
}

static void mqtt_inpub_data_cb(mqtt_client_t *client, void *arg, int pkt_id, const char *topic, const unsigned char *payload, unsigned short payload_len)
{
	QL_MQTT_LOG("topic: %s", topic);
	QL_MQTT_LOG("payload: %s", payload);
}

static void mqtt_disconnect_result_cb(mqtt_client_t *client, void *arg,int err){
	QL_MQTT_LOG("err: %d", err);
	
	ql_rtos_semaphore_release(mqtt_semp);
}
static void mqtt_app_thread(void * arg)
{
	int ret = 0;
	int i = 0, run_num = 1;
	int profile_idx = 1;
    ql_data_call_info_s info;
	char ip4_addr_str[16] = {0};
	mqtt_client_t  mqtt_cli;
	uint8_t nSim = 0;
	uint16_t sim_cid;
    struct mqtt_connect_client_info_t  client_info = {0};
    char *token = NULL;
    int is_user_onenet = 1;
	ql_rtos_semaphore_create(&mqtt_semp, 0);
	ql_rtos_task_sleep_s(10);

    char* client_id = (char*)malloc(256);
    char* client_user = (char*)malloc(256);
    char* client_pass = (char*)malloc(256);
    
	QL_MQTT_LOG("========== mqtt demo start ==========");
	QL_MQTT_LOG("wait for network register done");
		
	while((ret = ql_network_register_wait(nSim, 120)) != 0 && i < 10){
    	i++;
		ql_rtos_task_sleep_s(1);
	}
	if(ret == 0){
		i = 0;
		QL_MQTT_LOG("====network registered!!!!====");
	}else{
		QL_MQTT_LOG("====network register failure!!!!!====");
		goto exit;
	}

	ql_set_data_call_asyn_mode(nSim, profile_idx, 0);

	QL_MQTT_LOG("===start data call====");
	ret=ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IP, "uninet", NULL, NULL, 0); 
	QL_MQTT_LOG("===data call result:%d", ret);
	if(ret != 0){
		QL_MQTT_LOG("====data call failure!!!!=====");
	}
	memset(&info, 0x00, sizeof(ql_data_call_info_s));
	
	ret = ql_get_data_call_info(nSim, profile_idx, &info);
	if(ret != 0){
		QL_MQTT_LOG("ql_get_data_call_info ret: %d", ret);
		ql_stop_data_call(nSim, profile_idx);
		goto exit;
	}
    QL_MQTT_LOG("info->profile_idx: %d", info.profile_idx);
	QL_MQTT_LOG("info->ip_version: %d", info.ip_version);
            
	QL_MQTT_LOG("info->v4.state: %d", info.v4.state); 
	inet_ntop(AF_INET, &info.v4.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
	QL_MQTT_LOG("info.v4.addr.ip: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
	QL_MQTT_LOG("info.v4.addr.pri_dns: %s\r\n", ip4_addr_str);

	inet_ntop(AF_INET, &info.v4.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
	QL_MQTT_LOG("info.v4.addr.sec_dns: %s\r\n", ip4_addr_str);

	
	while(run_num <= 100)
	{	
        int test_num = 0;
        int case_id = 0;
        if(is_user_onenet != 1)
        {   
            case_id = run_num%2;
        }

		int ret = MQTTCLIENT_SUCCESS;
		
		QL_MQTT_LOG("==============mqtt_client_test[%d]================\n",run_num);

		if(QL_DATACALL_SUCCESS != ql_bind_sim_and_profile(nSim, profile_idx, &sim_cid))
		{
			QL_MQTT_LOG("nSim or profile_idx is invalid!!!!");
			break;
		}
		
		if(ql_mqtt_client_init(&mqtt_cli, sim_cid) != MQTTCLIENT_SUCCESS){
			QL_MQTT_LOG("mqtt client init failed!!!!");
			break;
		}

		QL_MQTT_LOG("mqtt_cli:%d", mqtt_cli);

        if(is_user_onenet == 1)
        {
    		client_info.keep_alive = 60;
    		client_info.pkt_timeout = 5;
    		client_info.retry_times = 3;
    		client_info.clean_session = 1;
    		client_info.will_qos = 0;
    		client_info.will_retain = 0;
    		client_info.will_topic = NULL;
    		client_info.will_msg = NULL;
            
    		client_info.client_id = client_id;
    		client_info.client_user = client_user;
    		client_info.client_pass = client_pass;
            
            memset((void*)client_info.client_user,0,256);
            memset((void*)client_info.client_id,0,256);
            memset((void*)client_info.client_pass,0,256);
             /*token过期时间(单位为s),请按照实际具体需求计算token过期时间,本例中为当前时刻的一年后过期*/
            signed long long expire_time = 24 * 60 * 60 * 365;
            token = ql_mqtt_onenet_generate_auth_token(expire_time,MQTT_CLIENT_ONENET_PRODUCTID,MQTT_CLIENT_ONENET_DEVICENAME,MQTT_CLIENT_ONENET_VERSION,MQTT_CLIENT_ONENET_ACCESSKEY);
            
		    if(NULL != token) {
		    	memcpy((void*)client_info.client_pass, token, strlen(token));
                memcpy((void*)client_info.client_user,MQTT_CLIENT_ONENET_PRODUCTID,strlen(MQTT_CLIENT_ONENET_PRODUCTID));
                memcpy((void*)client_info.client_id,MQTT_CLIENT_ONENET_DEVICENAME,strlen(MQTT_CLIENT_ONENET_DEVICENAME));
                free(token);
                token = NULL;
		    }
            else 
            {
                break;
            }
		    QL_MQTT_LOG("clientid_str=%s", client_info.client_id);
            QL_MQTT_LOG("username_str=%s", client_info.client_user);
            QL_MQTT_LOG("password_str=%s", client_info.client_pass);
        }
        else 
        {
            client_info.keep_alive = 60;
    		client_info.pkt_timeout = 5;
    		client_info.retry_times = 3;
            client_info.clean_session = 1;
            client_info.will_qos = 0;
            client_info.will_retain = 0;
            client_info.will_topic = NULL;
            client_info.will_msg = NULL;
            client_info.client_id = MQTT_CLIENT_IDENTITY;
            client_info.client_user = MQTT_CLIENT_USER;
            client_info.client_pass = MQTT_CLIENT_PASS;
        }
        QL_MQTT_LOG("connect ssl %d onenet mode %d",case_id,is_user_onenet);
		if(case_id == 0){
			client_info.ssl_cfg = NULL;
		    if(is_user_onenet == 1)
		    {
                ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_ONENET_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
		    }
            else 
            {
                ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_QUECTEL_URL , mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
            }
			
		}else{
			struct mqtt_ssl_config_t quectel_ssl_cfg = {
				.ssl_ctx_id = 1,
				.verify_level = MQTT_SSL_VERIFY_NONE,
				.cacert_path = NULL,
				.client_cert_path = NULL,
				.client_key_path = NULL,
				.client_key_pwd = NULL,
				.ssl_version = QL_SSL_VERSION_ALL,
				.sni_enable = 0,
				.ssl_negotiate_timeout = QL_SSL_NEGOTIATE_TIME_DEF,
				.ignore_invalid_certsign = 0,
				.ignore_multi_certchain_verify = 0,
				.ignore_certitem = MBEDTLS_X509_BADCERT_EXPIRED,
			};
            struct mqtt_ssl_config_t ontnet_ssl_cfg = {
                .ssl_ctx_id = 1,
                .verify_level = MQTT_SSL_VERIFY_NONE,
                .cacert_path = "UFS:MQTTS-certificate.pem",
                .client_cert_path = NULL,
                .client_key_path = NULL,
                .client_key_pwd = NULL,
				.ssl_version = QL_SSL_VERSION_ALL,
				.sni_enable = 0,
				.ssl_negotiate_timeout = QL_SSL_NEGOTIATE_TIME_DEF,
				.ignore_invalid_certsign = 0,
				.ignore_multi_certchain_verify = 0,
				.ignore_certitem = MBEDTLS_X509_BADCERT_EXPIRED,
            };
		    if(is_user_onenet == 1)
		    {
                client_info.ssl_cfg = &ontnet_ssl_cfg;
                ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_ONENET_SSL_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
		    }
            else 
            {
                client_info.ssl_cfg = &quectel_ssl_cfg;
                ret = ql_mqtt_connect(&mqtt_cli, MQTT_CLIENT_QUECTEL_SSL_URL, mqtt_connect_result_cb, NULL, (const struct mqtt_connect_client_info_t *)&client_info, mqtt_state_exception_cb);
            }
            
		}
		if(ret  == MQTTCLIENT_WOUNDBLOCK){
			QL_MQTT_LOG("====wait connect result");
			ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
			if(mqtt_connected == 0){
				ql_mqtt_client_deinit(&mqtt_cli);
				break;
			}
		}else{
			QL_MQTT_LOG("===mqtt connect failed ,ret = %d",ret);
			break;
		}

		ql_mqtt_set_inpub_callback(&mqtt_cli, mqtt_inpub_data_cb, NULL);

        if(is_user_onenet == 1)
        {
            if(is_user_onenet == 1)
            {
                if(ql_mqtt_sub_unsub(&mqtt_cli, "$sys/417661/test_led/dp/post/json/+", 1, mqtt_requst_result_cb,NULL, 1) == MQTTCLIENT_WOUNDBLOCK){
                	QL_MQTT_LOG("======wait subscrible result");
                	ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
                }
                if(ql_mqtt_publish(&mqtt_cli, "$sys/417661/test_led/dp/post/json",MQTT_PUB_MSG0, strlen(MQTT_PUB_MSG0), 0, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
                	QL_MQTT_LOG("======wait publish result");
                	ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
                }
                if(ql_mqtt_publish(&mqtt_cli, "$sys/417661/test_led/dp/post/json", MQTT_PUB_MSG1, strlen(MQTT_PUB_MSG1), 1, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
                	QL_MQTT_LOG("======wait publish result");
                	ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
                }
                //  onenet 平台不支持qos2
                if(ql_mqtt_sub_unsub(&mqtt_cli,"$sys/417661/test_led/dp/post/json/+", 1, mqtt_requst_result_cb,NULL, 0) == MQTTCLIENT_WOUNDBLOCK){
                	QL_MQTT_LOG("=====wait unsubscrible result");
                	ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
                }
            }
        }
        else{
    		while(test_num < 10 && mqtt_connected == 1){
    			if(ql_mqtt_sub_unsub(&mqtt_cli, "test", 1, mqtt_requst_result_cb,NULL, 1) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("======wait subscrible result");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    			}

    			if(ql_mqtt_publish(&mqtt_cli, "test", "hi, mqtt qos 0", strlen("hi, mqtt qos 0"), 0, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("======wait publish result");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    			}

    			if(ql_mqtt_publish(&mqtt_cli, "test", "hi, mqtt qos 1", strlen("hi, mqtt qos 1"), 1, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("======wait publish result");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    			}

    			if(ql_mqtt_publish(&mqtt_cli, "test", "hi, mqtt qos 2", strlen("hi, mqtt qos 2"), 2, 0, mqtt_requst_result_cb,NULL) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("======wait publish result");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    			}
    			
    			if(ql_mqtt_sub_unsub(&mqtt_cli, "test", 1, mqtt_requst_result_cb,NULL, 0) == MQTTCLIENT_WOUNDBLOCK){
    				QL_MQTT_LOG("=====wait unsubscrible result");
    				ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
    			}
    			test_num++;
    			ql_rtos_task_sleep_ms(500);
    		}
        }
        if(mqtt_connected == 1 && ql_mqtt_disconnect(&mqtt_cli, mqtt_disconnect_result_cb, NULL) == MQTTCLIENT_WOUNDBLOCK){
            QL_MQTT_LOG("=====wait disconnect result");
            ql_rtos_semaphore_wait(mqtt_semp, QL_WAIT_FOREVER);
        }
		QL_MQTT_LOG("==============mqtt_client_test[%d] end=======%x=========\n",run_num,&mqtt_cli);
		ql_mqtt_client_deinit(&mqtt_cli);
		mqtt_connected = 0;
		run_num++;
		ql_rtos_task_sleep_s(1);
        if(is_user_onenet == 1)
        {
            break;
        }
	}
	
exit:
    if(is_user_onenet == 1)
    {
        free((void*)client_id);
        free((void*)client_user);
        free((void*)client_pass);
        client_id = NULL;
        client_user = NULL;
        client_pass = NULL;
    }

    ql_rtos_semaphore_delete(mqtt_semp);
    ql_rtos_task_delete(mqtt_task);	

   return;	
}


int ql_mqtt_app_init(void)
{
	QlOSStatus err = QL_OSI_SUCCESS;
	
    err = ql_rtos_task_create(&mqtt_task, 16*1024, APP_PRIORITY_ABOVE_NORMAL, "QmqttApp", mqtt_app_thread, NULL, 5);
	if(err != QL_OSI_SUCCESS)
    {
		QL_MQTT_LOG("mqtt_app init failed");
	}

	return err;
}

