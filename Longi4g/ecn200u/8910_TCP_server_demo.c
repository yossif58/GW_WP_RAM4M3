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

作者：李燕-kelly
说明：8910 平台支持模块作为TCP 服务器，由于SDK 内没有相关demo, 所以自己写了个模块作为TCP 服务器的简单例程
TCP server 建立流程：socket () - > bind () - > listen () - > accept () - > read ()/write () - > close ()
注意：
1.  TCP server 用ipv6，ipv4返回的是网关给的内网地址，外网访问不了
2.  具体能否被访问，还和sim 卡运营商有关。
    需要使用联通卡，因为联通卡的IP 地址是固定的； 合肥这边联通卡V6地址结尾是::1，这种地址是可以稳定访问的，具体原因不清楚，估计和运营商有关
3.  客户端连接也需要是IPV6,  或兼容IPV4/IPV6

Auther:李燕-kelly
declare: 8910 platform support module as a TCP server, because there is no relevant demo in the SDK, so I wrote a module as a simple routine of the TCP server
TCP server setup process: socket () - > bind () - > listen () - > accept () - > read ()/write () - > close ()

Attention:
1. The TCP server uses ipv6 and ipv4 to return the Intranet address given by the gateway, and the external network cannot access it
2. Whether it can be accessed depends on the sim card operator.
Need to use Unicom card, because Unicom card IP address is fixed; Hefei Unicom card V6 address end is ::1, this address is stable access, the specific reasons are not clear, estimated and operators related



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

#define QL_TCP_SERVER_LOG_LEVEL QL_LOG_LEVEL_INFO
#define QL_TCP_SERVER_LOG(msg, ...) QL_LOG(QL_TCP_SERVER_LOG_LEVEL, "ql_tcp_Server", msg, ##__VA_ARGS__)
#define QL_TCP_SERVER_LOG_PUSH(msg, ...) QL_LOG_PUSH("ql_SSL", msg, ##__VA_ARGS__)

#define QL_TCP_SERVER_DEMO_CHANGE_DEMO 0

#if QL_TCP_SERVER_DEMO_CHANGE_DEMO
#define QL_TCP_SERVER_DEMO_URL "www.kelly.com"
#else

#endif

static ql_task_t tcp_server_demo_task = NULL;




static void tcp_server_app_thread(void *arg)
{
    int ret = 0;
    int i = 0;
    int profile_idx = 1;
    ql_data_call_info_s info;
    char ip4_addr_str[50] = {0};

    int socket_fd = -1;
    int sock_connect;

     struct sockaddr_in local4;
    struct sockaddr_in connect_addr;
    struct addrinfo *pres = NULL;
    struct addrinfo *temp = NULL;
    uint8_t nSim = 0;
    // ql_nw_data_count_info_s data_count_info = {0};
#if QL_TCP_SERVER_DEMO_CHANGE_DEMO
    ql_datacall_dns_info_s dns_pri = {0};
    ql_datacall_dns_info_s dns_sec = {0};
#endif

    ql_rtos_task_sleep_s(10);
    QL_TCP_SERVER_LOG("========== tcp_server demo start ==========");
    QL_TCP_SERVER_LOG("tcp_server wait for network register done");

    while ((ret = ql_network_register_wait(nSim, 120)) != 0 && i < 10)
    {
        i++;
        ql_rtos_task_sleep_s(1);
    }

    QL_TCP_SERVER_LOG("====tcp_server network registered!!!!====");
    if (ret == 0)
    {
        i = 0;
        QL_TCP_SERVER_LOG("====tcp_server network registered!!!!====");
    }
    else
    {
        QL_TCP_SERVER_LOG("====network register failure!!!!!====");
        goto exit;
    }

    ql_set_data_call_asyn_mode(nSim, profile_idx, 0);

    QL_TCP_SERVER_LOG("===tcp_server start data call====");
    ret = ql_start_data_call(nSim, profile_idx, QL_PDP_TYPE_IPV6, "UNINET", NULL, NULL, 0);
    QL_TCP_SERVER_LOG("===tcp_server data call result:%d", ret);
    if (ret != 0)
    {
        QL_TCP_SERVER_LOG("====tcp_server data call failure!!!!=====");
    }
    memset(&info, 0x00, sizeof(ql_data_call_info_s));

    ret = ql_get_data_call_info(nSim, profile_idx, &info);
    if (ret != 0)
    {
        QL_TCP_SERVER_LOG("ql_get_data_call_info ret: %d", ret);
        ql_stop_data_call(nSim, profile_idx);
        goto exit;
    }
    QL_TCP_SERVER_LOG("info->profile_idx: %d", info.profile_idx);
    QL_TCP_SERVER_LOG("info->ip_version: %d", info.ip_version);

    QL_TCP_SERVER_LOG("info->v6.state: %d", info.v6.state);
    inet_ntop(AF_INET6, &info.v6.addr.ip, ip4_addr_str, sizeof(ip4_addr_str));
    QL_TCP_SERVER_LOG("info.v6.addr.ip: %s\r\n", ip4_addr_str);

    inet_ntop(AF_INET6, &info.v6.addr.pri_dns, ip4_addr_str, sizeof(ip4_addr_str));
    QL_TCP_SERVER_LOG("info.v6.addr.pri_dns: %s\r\n", ip4_addr_str);

    inet_ntop(AF_INET6, &info.v6.addr.sec_dns, ip4_addr_str, sizeof(ip4_addr_str));
    QL_TCP_SERVER_LOG("info.v6.addr.sec_dns: %s\r\n", ip4_addr_str);

#if 1
    //绑定本地网口
    memset(&local4, 0x00, sizeof(struct sockaddr_in));
    local4.sin_family = AF_INET6;
    local4.sin_port = htons(2020);
    local4.sin_addr.s_addr = &info.v6.addr.ip;
  

#endif
loop:
    QL_TCP_SERVER_LOG("tcp_server loop!\n"); 
 

         QL_TCP_SERVER_LOG("tcp_server for!\n");
     
            QL_TCP_SERVER_LOG("tcp_server temp->ai_family == AF_INET6\n");
          
            socket_fd = socket(AF_INET6, SOCK_STREAM, 0);
            if (socket_fd < 0)
            {
                QL_TCP_SERVER_LOG("tcp_server socket error\n");
               // continue;
            }
            QL_TCP_SERVER_LOG("tcp_server socket\n");
            ret = bind(socket_fd, (struct sockaddr *)&local4, sizeof(struct sockaddr));
            if (ret < 0)
            {
                 QL_TCP_SERVER_LOG("tcp_server bind error\n");
                close(socket_fd);
                socket_fd = -1;
                //continue;
            }
            listen(socket_fd, 5);
            QL_TCP_SERVER_LOG("tcp_server listen\n");
            while (1)
            {
                QL_TCP_SERVER_LOG("tcp_server while 1\n");
                struct sockaddr_in clnaddr = {0};
                int clnsize = sizeof(clnaddr);
                
                 char buf[128] = {0};//存放客户端发送的数据
                 int  buf_len = 0;//客户端具体发过来的消息的长度

             

                sock_connect = accept(socket_fd, (struct sockaddr *)&connect_addr, (socklen_t *)&clnsize);
             
                QL_TCP_SERVER_LOG("tcp_server accept sock_connect=%d\n",sock_connect);
                if (sock_connect == -1)
                {
                    QL_TCP_SERVER_LOG("no socket,waitting others socket disconnect.\n");
                    continue;
                }

                 //读取客户端发送的数据
                 buf_len = read(sock_connect,buf,sizeof(buf)-1);

                 QL_TCP_SERVER_LOG("recive: buf_len is %d, buf is \n %s\n",buf_len,buf);


              
                buf_len = write(sock_connect,buf,buf_len);

                QL_TCP_SERVER_LOG("write finished, send len is %d\n",buf_len);

              //  close(sock_connect);//关闭客户端的socket
                


            
            }
  
    if (pres != NULL)
    {
        freeaddrinfo(pres);
        pres = NULL;
    }
    goto loop;
exit:
    ql_rtos_task_delete(tcp_server_demo_task);
    close(sock_connect);//关闭客户端的socket
    return;
}

int ql_tcp_server_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;

    err = ql_rtos_task_create(&tcp_server_demo_task, 16 * 1024, APP_PRIORITY_ABOVE_NORMAL, "QtcpServerApp", tcp_server_app_thread, NULL, 5);
    if (err != QL_OSI_SUCCESS)
    {
        QL_TCP_SERVER_LOG("tcp_server_app init failed");
    }

    return err;
}