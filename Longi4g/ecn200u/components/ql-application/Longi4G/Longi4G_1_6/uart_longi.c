
/*================================================================
  Copyright (c) 2021, Quectel Wireless Solutions Co., Ltd. All rights reserved.
  Quectel Wireless Solutions Proprietary and Confidential.
=================================================================*/
    
/*=================================================================

						EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

WHEN			  WHO		  WHAT, WHERE, WHY
------------	 -------	 -------------------------------------------------------------------------------

=================================================================*/


/*===========================================================================
 * include files
 ===========================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ql_api_osi.h"
#include "ql_log.h"
#include "ql_uart.h"
#include "ql_gpio.h"
#include "ql_pin_cfg.h"
#include "ql_usb.h"
#include "longi4g.h"

/*===========================================================================
 *Definition
 ===========================================================================*/
#define QL_UART_DEMO_LOG_LEVEL			QL_LOG_LEVEL_INFO
#define QL_UART_DEMO_LOG(msg, ...)		QL_LOG(QL_UART_DEMO_LOG_LEVEL, "ql_uart_demo", msg, ##__VA_ARGS__)


#define QL_UART_TASK_STACK_SIZE     		4096
#define QL_UART_TASK_PRIO          	 	    APP_PRIORITY_NORMAL
#define QL_UART_TASK_EVENT_CNT      		5


#define QL_UART_RX_BUFF_SIZE                2048
#define QL_UART_TX_BUFF_SIZE                2048
#define QL_DEBUG_UART2_TX_PIN				30
#define QL_DEBUG_UART2_RX_PIN				31

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define QL_USB_PRINTER_ENABLE	0

/*===========================================================================
 * Variate
 ===========================================================================*/

/*===========================================================================
 * Functions
 ===========================================================================*/
extern ql_queue_t DebugUartqueue;
extern ql_queue_t MeterUartqueue;
MeterQueueStruct InfoMeter;
void ql_debug_uart_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size)
{
    unsigned char *recv_buff = calloc(1, QL_UART_RX_BUFF_SIZE+1);
    unsigned int real_size = 0;
    int read_len = 0;
    uint8_t bufin[1024];
	uint16_t buflen;   
    QL_UART_DEMO_LOG("UART port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
    switch(ind_type)
    {
        case QUEC_UART_RX_OVERFLOW_IND:  //rx buffer overflow
        case QUEC_UART_RX_RECV_DATA_IND:
        {
			buflen =0;
            while(size > 0)
            {
                memset(recv_buff, 0, QL_UART_RX_BUFF_SIZE+1);
                real_size= MIN(size, QL_UART_RX_BUFF_SIZE);
                
                read_len = ql_uart_read(port, recv_buff, real_size);
				buflen+=read_len;
				memcpy((uint8_t*)&bufin[buflen],recv_buff,buflen);
                QL_UART_DEMO_LOG("read_len=%d, recv_data=%s", read_len, recv_buff);
                if((read_len > 0) && (size >= read_len))
                {
                    size -= read_len;
                }
                else
                {
					break;
                }
            }
			ql_rtos_queue_release(DebugUartqueue, buflen, (uint8 *)bufin, QL_NO_WAIT); //lbk
	
            break;
        }
        case QUEC_UART_TX_FIFO_COMPLETE_IND: 
        {
            QL_UART_DEMO_LOG("tx fifo complete");
            break;
        }
    }
    free(recv_buff);
    recv_buff = NULL;
//	free (bufin);
}


void ql_uart_meter_notify_cb(unsigned int ind_type, ql_uart_port_number_e port, unsigned int size)
{
    unsigned char *recv_buff = calloc(1, QL_UART_RX_BUFF_SIZE+1);
    unsigned int real_size = 0;
    int read_len = 0;
    
    QL_UART_DEMO_LOG("UART port %d receive ind type:0x%x, receive data size:%d", port, ind_type, size);
    switch(ind_type)
    {
        case QUEC_UART_RX_OVERFLOW_IND:  //rx buffer overflow
        case QUEC_UART_RX_RECV_DATA_IND:
        {
            while(size > 0)
            {
                memset(recv_buff, 0, QL_UART_RX_BUFF_SIZE+1);
                real_size= MIN(size, QL_UART_RX_BUFF_SIZE);
                
                read_len = ql_uart_read(port, recv_buff, real_size);
                QL_UART_DEMO_LOG("read_len=%d, recv_data=%s", read_len, recv_buff);
                if((read_len > 0) && (size >= read_len))
                {
                    size -= read_len;
                }
                else
                {
					InfoMeter.len=size;
					InfoMeter.rcvbuf = recv_buff;
					ql_rtos_queue_release(MeterUartqueue, size,(uint8_t*)&InfoMeter, QL_NO_WAIT);    // y.f. send to MeterUartIn handler    		
                    break;
                }
            }
            break;
        }
        case QUEC_UART_TX_FIFO_COMPLETE_IND: 
        {
            QL_UART_DEMO_LOG("tx fifo complete");
            break;
        }
    }
    free(recv_buff);
    recv_buff = NULL;
}

static void ql_debug_uart_thread(void *param)
{
    uint8_t recvMsg[1028];
    int ret = 0;
	QlOSStatus err = 0;
    ql_uart_config_s uart_cfg = {0};
    int write_len = 0;
    ql_uart_tx_status_e tx_status;

    /***********************************************************
	Note start:
        1.If the BAUD rate is QL UART BAUD_AUTO,a string of 'at'should be sent at least once to identify the baud rate.
        2.Once the baud rate is identified, it cannot be changed unless restarted.
    ************************************************************/
    uart_cfg.baudrate = QL_UART_BAUD_115200;
    uart_cfg.flow_ctrl = QL_FC_NONE;
    uart_cfg.data_bit = QL_UART_DATABIT_8;
    uart_cfg.stop_bit = QL_UART_STOP_1;
    uart_cfg.parity_bit = QL_UART_PARITY_NONE;

    ret = ql_uart_set_dcbconfig(QL_UART_PORT_2, &uart_cfg);
    QL_UART_DEMO_LOG("ret: 0x%x", ret);
	if(QL_UART_SUCCESS != ret)
	{
		goto exit;
	}
	
	/***********************************************************
	Note start:
		1. If QL_UART_PORT_1 is selected for use, there is no need to set TX and RX pin and function
		2. According to the QuecOpen GPIO table, user should select the correct PIN to set function
		3. CTS and RTS pins (UART2 and UART3) also need to be initialized if hardware flow control function is required
	************************************************************/
	ret = ql_pin_set_func(QL_DEBUG_UART2_TX_PIN, 5); // alternate function 5
	if(QL_GPIO_SUCCESS != ret)
	{
		goto exit;
	}
	ret = ql_pin_set_func(QL_DEBUG_UART2_RX_PIN, 5); // alternate function 5
	if(QL_GPIO_SUCCESS != ret)
	{
		goto exit;
	}
	/*Note end*/
	
    ret = ql_uart_open(QL_UART_PORT_2);
    QL_UART_DEMO_LOG("ret: 0x%x", ret);
	

	if(QL_UART_SUCCESS == ret)
	{
	    ret = ql_uart_register_cb(QL_UART_PORT_2, ql_debug_uart_notify_cb);
	    QL_UART_DEMO_LOG("ret: 0x%x", ret);


	    memset(&uart_cfg, 0, sizeof(ql_uart_config_s));
	    ret = ql_uart_get_dcbconfig(QL_UART_PORT_2, &uart_cfg);
	    QL_UART_DEMO_LOG("ret: 0x%x, baudrate=%d, flow_ctrl=%d, data_bit=%d, stop_bit=%d, parity_bit=%d", 
	                        ret, uart_cfg.baudrate, uart_cfg.flow_ctrl, uart_cfg.data_bit, uart_cfg.stop_bit, uart_cfg.parity_bit);
	    
	    while(1)
	    {
	    	ql_rtos_queue_wait (DebugUartqueue , recvMsg, 6, QL_WAIT_FOREVER);
			
			//unsigned char data[] = "Powercom modem for Longi meter DEBUG CHANNEL\r\n";
		//		write_len = ql_uart_write(QL_UART_PORT_2, recvMsg, strlen((char *)recvMsg));
				write_len = ql_uart_write(QL_UART_PORT_2, recvMsg, 20);  // temp 20
	       // write_len = ql_uart_write(QL_UART_PORT_2, data, strlen((char *)data));
	        QL_UART_DEMO_LOG("write_len:%d", write_len);
            ql_uart_get_tx_fifo_status(QL_UART_PORT_2, &tx_status);
            QL_UART_DEMO_LOG("tx_status:%d", tx_status);
	    //    ql_rtos_task_sleep_ms(5000);
	    }
	}

exit:
    err = ql_rtos_task_delete(NULL);
	if(err != QL_OSI_SUCCESS)
	{
		QL_UART_DEMO_LOG("task deleted failed");
	}
}


static void ql_meter_uart_thread(void *param)
{
    int ret = 0;
	QlOSStatus err = 0;
    ql_uart_config_s uart_cfg = {0};
    int write_len = 0;
    ql_uart_tx_status_e tx_status;
   

    /***********************************************************
	Note start:
        1.If the BAUD rate is QL UART BAUD_AUTO,a string of 'at'should be sent at least once to identify the baud rate.
        2.Once the baud rate is identified, it cannot be changed unless restarted.
    ************************************************************/
    uart_cfg.baudrate = QL_UART_BAUD_9600;
    uart_cfg.flow_ctrl = QL_FC_NONE;
    uart_cfg.data_bit = QL_UART_DATABIT_8;
    uart_cfg.stop_bit = QL_UART_STOP_1;
    uart_cfg.parity_bit = QL_UART_PARITY_NONE;

    ret = ql_uart_set_dcbconfig(QL_UART_PORT_1, &uart_cfg);
    QL_UART_DEMO_LOG("ret: 0x%x", ret);
	if(QL_UART_SUCCESS != ret)
	{
		goto exit;
	}
	
	/***********************************************************
	Note start:
		1. If QL_UART_PORT_1 is selected for use, there is no need to set TX and RX pin and function
		2. According to the QuecOpen GPIO table, user should select the correct PIN to set function
		3. CTS and RTS pins (UART2 and UART3) also need to be initialized if hardware flow control function is required
	************************************************************/

	
    ret = ql_uart_open(QL_UART_PORT_1);
    QL_UART_DEMO_LOG("ret: 0x%x", ret);
	

	if(QL_UART_SUCCESS == ret)
	{
	    ret = ql_uart_register_cb(QL_UART_PORT_1, ql_uart_meter_notify_cb); 
		                                         
	    QL_UART_DEMO_LOG("ret: 0x%x", ret);

	    ret = ql_uart_register_cb(QL_UART_PORT_2, ql_debug_uart_notify_cb); 
		                                         
	    QL_UART_DEMO_LOG("ret: 0x%x", ret);

	    memset(&uart_cfg, 0, sizeof(ql_uart_config_s));
	    ret = ql_uart_get_dcbconfig(QL_UART_PORT_1, &uart_cfg);
	    QL_UART_DEMO_LOG("ret: 0x%x, baudrate=%d, flow_ctrl=%d, data_bit=%d, stop_bit=%d, parity_bit=%d", 
	                        ret, uart_cfg.baudrate, uart_cfg.flow_ctrl, uart_cfg.data_bit, uart_cfg.stop_bit, uart_cfg.parity_bit);
	    
	    while(1)
	    {
			unsigned char data[] = "hello uart demo for powercom\r\n";
	        write_len = ql_uart_write(QL_UART_PORT_1, data, strlen((char *)data));
	        QL_UART_DEMO_LOG("write_len:%d", write_len);
            ql_uart_get_tx_fifo_status(QL_UART_PORT_1, &tx_status);
            QL_UART_DEMO_LOG("tx_status:%d", tx_status);
	        ql_rtos_task_sleep_ms(4000);
	    }
	}

exit:
    err = ql_rtos_task_delete(NULL);
	if(err != QL_OSI_SUCCESS)
	{
		QL_UART_DEMO_LOG("task deleted failed");
	}
}
 
 


void ql_longi_uart_app_init(void)
{

	QlOSStatus err = 0;
	ql_task_t uart_task = NULL;


	err = ql_rtos_task_create(&uart_task, QL_UART_TASK_STACK_SIZE, QL_UART_TASK_PRIO, "QUARTDEBUG", ql_debug_uart_thread, NULL, QL_UART_TASK_EVENT_CNT);
	if (err != QL_OSI_SUCCESS)
	{
		QL_UART_DEMO_LOG("demo task created failed");
        return;
	}

	err = ql_rtos_task_create(&uart_task, QL_UART_TASK_STACK_SIZE, QL_UART_TASK_PRIO, "QUARTMETER", ql_meter_uart_thread, NULL, QL_UART_TASK_EVENT_CNT);
	if (err != QL_OSI_SUCCESS)
	{
		QL_UART_DEMO_LOG("demo task created failed");
        return;
	}
	
}


