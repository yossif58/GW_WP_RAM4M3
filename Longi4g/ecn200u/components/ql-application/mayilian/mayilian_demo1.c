//socket
#include "bot_system.h"
#include "bot_network.h"
#include "bot_socket.h"
#include "lwip/netdb.h"

#define BOT_SERVER_DOMAIN "220.180.239.212"
#define BOT_SERVER_PORT 8586
#define TCP_CONNECT_TIMEOUT_S 10
#define TCP_CLOSE_LINGER_TIME_S 10
#define TCP_RECV_TIMEOUT_S 10
#define TCP_CLIENT_SEND_STR "1234567890abcdefghijklmnopqrseuvwxyz"
static char recv_buf[128]={0};

void bot_socket_test(void)
{
    int ret = 0;
    int             sock_fd     = -1;
    struct sockaddr_in	* ip4_svr_addr;
    struct addrinfo *pres = NULL;
	fd_set 			read_fds, write_fds;
	struct timeval	t;
    int             sock_error  = 0;

    bot_network_init();
    while(1)
    {
        if(bot_network_is_ready())
            break;
        bot_printf("bot_network_is_ready:\r\n");
        bot_msleep(1000);
    }
    bot_printf("bot socket test start:\r\n");

    
    ret = bot_socket_open(AF_INET, bot_net_tcp, 0);
	if(ret < 0)
	{
		bot_printf("*** bot socket create fail ***\r\n");
		goto exit;
	}
	bot_printf("*** bot_socket_open ok ***\r\n");
	sock_fd = ret;
	
	ret = getaddrinfo_with_pcid(BOT_SERVER_DOMAIN, NULL, NULL, &pres, (uint32)QUEC_BOT_CONTEXT_ID);
	if (ret < 0 || pres == NULL) 
	{
		bot_printf("bot DNS getaddrinfo failed! ret=%d; pres=%p!\n",ret,pres);
	}
    ip4_svr_addr = (struct sockaddr_in *)pres->ai_addr;
    ip4_svr_addr->sin_port = htons(BOT_SERVER_PORT);
    
    ret = bot_socket_connect(sock_fd, (struct sockaddr *)ip4_svr_addr, sizeof(struct sockaddr));
    bot_printf("bot connect ret: %d, errno: %u\r\n", ret, bot_socket_errno());

    t.tv_sec = TCP_CONNECT_TIMEOUT_S;
	t.tv_usec = 0;
	
    BOT_FD_ZERO(&read_fds);
	BOT_FD_ZERO(&write_fds);

	BOT_FD_SET(sock_fd, &read_fds);
	BOT_FD_SET(sock_fd, &write_fds);

	ret = bot_socket_select(sock_fd + 1, &read_fds, &write_fds, NULL, &t);
    bot_printf("bot select ret: %d\r\n", ret);
	if(ret <= 0)
	{
		bot_printf("*** bot select timeout or error ***\r\n");
		goto exit;
	}
    if(!BOT_FD_ISSET(sock_fd, &read_fds) && !BOT_FD_ISSET(sock_fd, &write_fds))
    {
        bot_printf("*** bot connect fail ***\r\n");
        goto exit;
    }
    else if(BOT_FD_ISSET(sock_fd, &read_fds) && BOT_FD_ISSET(sock_fd, &write_fds))
    {
        sock_error = bot_socket_errno();
        if(sock_error == 0)
        {
            bot_printf("bot connect success\r\n");
        }
        else
        {
            bot_printf("*** bot connect fail, sock_err = %d, errno = %u ***\r\n", sock_error, errno);
            goto exit;
        }
    }
    else if(!BOT_FD_ISSET(sock_fd, &read_fds) && BOT_FD_ISSET(sock_fd, &write_fds))
    {
        bot_printf("bot connect success\r\n");
    }
    else if(BOT_FD_ISSET(sock_fd, &read_fds) && !BOT_FD_ISSET(sock_fd, &write_fds))
    {
        bot_printf("*** bot connect fail ***\r\n");
        goto exit;
    }
    else
    {
        bot_printf("*** bot connect fail ***\r\n");
        goto exit;
    }
    ret = bot_socket_write(sock_fd, (const void*)TCP_CLIENT_SEND_STR, strlen(TCP_CLIENT_SEND_STR));
    if(ret < 0)
    {
        bot_printf("***bot  send fail ***\r\n");
        goto exit;
    }

_recv_:

    t.tv_sec = TCP_RECV_TIMEOUT_S;
    t.tv_usec = 0;

    FD_ZERO(&read_fds);
    FD_SET(sock_fd, &read_fds);

    ret = bot_socket_select(sock_fd + 1, &read_fds, NULL, NULL, &t);

    bot_printf("bot select ret: %d\r\n", ret);

    if(ret <= 0)
    {
        bot_printf("*** bot select timeout or error ***\r\n");
        goto exit;
    }

    if(FD_ISSET(sock_fd, &read_fds))
    {
        ret = bot_socket_read(sock_fd, recv_buf, sizeof(recv_buf));
        if(ret > 0)
        {
            bot_printf("bot recv data: [%d]%s\r\n", ret, recv_buf);
        }
        else if(ret == 0)
        {
            bot_printf("*** bot peer closed ***\r\n");
            goto exit;
        }
        else
        {
            if(!(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN))
            {
                bot_printf("***bot error occurs ***\r\n");
                goto exit;
            }
            else
            {
                bot_printf("bot wait for a while\r\n");
                bot_msleep(20);
                goto _recv_;
            }
        }

    }
	
exit:
    
    if(pres!=NULL)
    {
         freeaddrinfo(pres);
         pres = NULL;
    }

    if(sock_fd >= 0)
    {
        bot_socket_close(sock_fd);
    }

}

//fs
#include "bot_fs.h"
#define BOT_TEST_DISK    "UFS:"
#define BOT_TEST_DIR	 "bot"
#define BOT_FILE_NAME	 "test_file.txt"

#define BOT_DIR_PATH     ""BOT_TEST_DISK"/"BOT_TEST_DIR""
#define BOT_FILE_PATH	 ""BOT_DIR_PATH"/"BOT_FILE_NAME""
#define BOT_TEST_STR     "0123456789ABCDEFG"

void bot_fs_test(void)
{
    int  ret = 0;
    bool  is_vaild = TRUE;
    bot_file_t  fp = 0;
    char buffer[100] = {0};
    
    ret = bot_exitdir(BOT_DIR_PATH);
    bot_printf("bot_exitdir ret:%d",ret);
    if(ret == 0)
    {
        ret = ql_rmdir(BOT_DIR_PATH);
        bot_printf("bot_unlink ret:%d ",ret);
    }
    ret = bot_fs_init();
    bot_printf("bot_fs_init ret:%d",ret);
    
    ret = bot_exitdir(BOT_DIR_PATH);
    bot_printf("bot_exitdir ret:%d",ret);

    ret = bot_exitdir("UFS:zfw");
    bot_printf("bot_exitdir ret:%d",ret);    
    if(ret == 0)
    {
        ret = ql_rmdir("UFS:zfw");
        bot_printf("bot_unlink ret:%d ",ret);
    }
    ret = bot_mkdir("UFS:zfw");
    bot_printf("bot_mkdir ret:%d",ret);
    
    fp = bot_fopen(BOT_FILE_PATH, "wb+");
    bot_printf("bot_fopen ret:%d",ret);
    
    ret = bot_fwrite(BOT_TEST_STR, strlen(BOT_TEST_STR) + 1, 1, fp);   //strlen not include '\0'
    bot_printf("bot_fwrite ret:%d",ret);
    
    ret = bot_fseek(fp, 0, SEEK_SET);
    bot_printf("bot_fseek ret:%d",ret);
    
    memset(buffer, 0, 100);
    ret = bot_fread(buffer, 100, 1, fp);
    bot_printf("bot_fread ret:%d buffer:%s",ret,buffer);

    ret = bot_fflush(fp);
    bot_printf("bot_fflush ret:%d ",ret);

    is_vaild = bot_file_is_valid(fp);
    bot_printf("bot_file_is_valid is_vaild:%d ",is_vaild);

    ret = bot_fclose(fp);
    bot_printf("bot_close ret:%d ",ret);

    is_vaild = bot_file_is_valid(fp);
    bot_printf("bot_file_is_valid is_vaild:%d ",is_vaild);

    ret = bot_unlink(BOT_FILE_PATH);
    bot_printf("bot_unlink ret:%d ",ret);
    bot_fs_print_errno();


}

//network
void bot_network_test()
{
    int is_ready = 0;
    bot_network_init();
    while(1)
    {
        is_ready = bot_network_is_ready();
        if(is_ready)
            break;
        bot_printf("bot_msleep");
        bot_msleep(500);
    }
    signed short rssi;
    unsigned short mcc, mnc;
    unsigned int cid, lac;
    unsigned char rat;
    bot_printf("bot_network_test enter");
    bot_network_rssi_get(&rssi);
    bot_printf("[zfw]bot rssi:%d",rssi);
    bot_network_lbs_get( &mcc, &mnc, &cid, &lac);
    bot_printf("[zfw]bot mcc:%d mnc:%d cid:%d lac:%d",mcc, mnc, cid, lac);
    bot_network_rat_get( &rat);
    bot_printf("[zfw]bot rat:%d",rat);
}

//hal_iot
void bot_hal_iot_test(void)
{
    int ret = 0;
    char ccid_str[128] = {0};
    char imsi_str[128] = {0};
    uint8_t random_str[12] = {0};

     unsigned long long time_ms = bot_uptime();
     bot_printf("bot time_ms:%lld",time_ms);
     
     struct timeval tv ={0};
     bot_gettimeofday(&tv,NULL);
     bot_printf("bot_gettimeofday tv.tv_sec:%lld tv.tv_usec:%d",tv.tv_sec, tv.tv_usec);


    ret = bot_network_ccid_get(ccid_str);
    bot_printf("bot_network_ccid_get ret =%d  ccid_str:%s",ret, ccid_str);
    
    ret = bot_network_imei_get(imsi_str);
    bot_printf("bot_network_imei_get ret =%d imsi_str:%s",ret, imsi_str);
    
    bot_printf("bot_software_version_get:%s",bot_software_version_get());
    bot_printf("bot_hardware_version_get:%s",bot_hardware_version_get());
    
    for(int i =0; i< 10;i++)
    {
        ret = bot_random_generate(random_str, 11);
        bot_printf("bot random:%s",random_str);
    }

}

#define SOCKET_TEST 0x01
#define FS_TEST 0x02
#define NETWORK_TEST 0x04
#define HAL_IOT_TEST 0x08

int mayi_test(int testCaseId)
{
    if (testCaseId & SOCKET_TEST)
    {
         bot_socket_test();
         bot_msleep(2000);
    }
    if (testCaseId & FS_TEST)
    {
         bot_fs_test();
         bot_msleep(2000);
    }
    if (testCaseId & NETWORK_TEST)
    {
         bot_network_test();
         bot_msleep(2000);
    }
    if (testCaseId & HAL_IOT_TEST)
    {
         bot_hal_iot_test();
         bot_msleep(2000);
    }
    
return 0;
}

extern void bot_app_entry(void);

void mayilin_demo1(void )
{

   bot_app_entry();
   #if 0
    bot_network_init();

    int ret;
    int fd = 0;
    
    while(!bot_network_is_ready()) {
        bot_msleep(1000);
    }
    mayi_test(HAL_IOT_TEST);
    ret = bot_socket_app_connect(&fd,"iot-auth.cn-shanghai.aliyuncs.com","443",bot_net_tcp);
    bot_printf(" bot2 ret=%d",ret);
    bot_printf(" bot3 fd=%d",fd);
    #endif
}







