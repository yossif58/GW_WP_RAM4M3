/***********************************************************************************************************************
 * File Name    : uart.c
 * Description  : Contains UART functions definition.
 **********************************************************************************************************************/
/***********************************************************************************************************************
 * DISCLAIMER
 * This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
 * other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
 * applicable laws, including copyright laws.
 * THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
 * THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
 * EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
 * SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
 * SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
 * this software. By using this software, you agree to the additional terms and conditions found by accessing the
 * following link:
 * http://www.renesas.com/disclaimer
 *
 * Copyright (C) 2020 Renesas Electronics Corporation. All rights reserved.
 ***********************************************************************************************************************/

#include "hal_data.h"
#include "stdbool.h"
#include "main.h"
#include "uart_LTE.h"
//#include "DLTProtocol.h"
#include "LongiWrapper.h"


//#define  DATA_LENGTH   1024
#define END_CHAR_TIMOUT 3
/*******************************************************************************************************************//**
 * @addtogroup r_sci_uart_ep
 * @{
 **********************************************************************************************************************/

/*
 * Private function declarations
 */

/*
 * Private global variables
 */

/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t g_LTE_temp_buffer[MAX_UART_SIZE] = {RESET_VALUE};

/* Counter to update g_temp_buffer index */
static volatile uint32_t g_LTE_counter_var = RESET_VALUE;

/* Flag to check whether data is received or not */
//static volatile uint8_t g_LTE_data_received_flag = false;

/* Temporary buffer to save data from receive buffer for further processing */
static uint8_t g_DEBUG_temp_buffer[MAX_UART_SIZE] = {RESET_VALUE};

/* Counter to update g_temp_buffer index */
static volatile uint8_t g_DEBUG_counter_var = RESET_VALUE;


/* Flag for user callback */
static volatile uint8_t g_DEBUG_uart_event = RESET_VALUE;
static volatile uint8_t g_DEBUG_timout_cntr;
static volatile uint8_t g_LTE_timout_cntr;
/* Flag for user callback */
static volatile uint8_t g_LTE_uart_event = RESET_VALUE;
static uint32_t yos_temp=0;

extern void ModemSequence(ModemModeSequence ModeSeq);
extern uint8_t modem_seq_state;
uint8_t bufintmp[MAX_UART_SIZE];
uint16_t bufintmpind;
uint32_t GsmTimer = 0;
uint32_t TimingDelay;
extern uint32_t Ltutmpindexin;
extern uint8_t Ltutemputartin[MAX_UART_SIZE];
extern uint32_t Ltutmpindexout;
extern uint8_t Ltutemputartout[MAX_UART_SIZE];

extern bool LtuGSM_Timeout;

extern void Gsm_Handler(uint8_t * pDdataIn, uint16_t Len , EventType Event);
extern void ModemInitSequence (ModemModeSequence  ModeSeq);
extern void ToggleLeds(void);
//uint32_t WaitAfterKC = 0;
bool ToggleLedsReq = false;
#if 0

 *  @brief       UART Example project to demonstrate the functionality
 *  @param[in]   None
 *  @retval      FSP_SUCCESS     Upon success
 *  @retval      Any Other Error code apart from FSP_SUCCESS
 ****************************************************************************************************************/
fsp_err_t uart_ep_demo(void)
{
    fsp_err_t err = FSP_SUCCESS;
    volatile bool b_valid_data = true;

    while (true)
    {
        if(g_LTE_data_received_flag)
        {
        	g_LTE_data_received_flag  = false;

            uint8_t input_length = RESET_VALUE;
            volatile uint32_t intensity = RESET_VALUE;

            /* Calculate g_temp_buffer length */
            input_length = ((uint8_t)(strlen((char *) &g_temp_buffer)));

            /* Check if input data length is in limit */
            if (MAX_UART_SIZE > (uint8_t)input_length)
            {
                /* This loop validates input data byte by byte to filter out decimals. (floating point input)
                 * Any such data will be considered as invalid. */
                for(int buf_index = RESET_VALUE; buf_index < input_length; buf_index++)
                {
                    if(ZERO_ASCII <= g_temp_buffer[buf_index] && NINE_ASCII >= g_temp_buffer[buf_index])
                    {
                        /* Set b_valid_data Flag as data is valid */
                        b_valid_data = true;
                    }
                    else
                    {
                        /* Clear data_valid flag as data is not valid, Clear the buffer and break the loop */
                        memset(g_temp_buffer, RESET_VALUE, MAX_UART_SIZE);
                        b_valid_data = false;
                        break;
                    }
                }

                /* All bytes in data are integers, convert input to integer value to set intensity. */
                intensity = ((uint32_t)(atoi((char *) &g_temp_buffer)));

                /* Validation input data is in 1 - 100 range. */
                if(MAX_INTENISTY < intensity || RESET_VALUE == intensity)
                {
                    /* Resetting the g_temp_buffer as data is out of limit */
                    memset(g_temp_buffer, RESET_VALUE, MAX_UART_SIZE);
                    b_valid_data = false;

                    /* Application is being run on Serial terminal hence transmitting error message to the same */
                    err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 100\r\n");
                    if (FSP_SUCCESS != err)
                    {
                        APP_ERR_PRINT ("\r\n **  UART WRITE FAILED  ** \r\n");
                        return err;
                    }
                }
            }
            else
            {
                /* Clear data_valid flag as data is not valid, Clear the g_temp_buffer */
                memset(g_temp_buffer, RESET_VALUE, MAX_UART_SIZE);
                b_valid_data = false;

                err = uart_print_user_msg((uint8_t *)"\r\nInvalid input. Input range is from 1 - 100\r\n");
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT ("\r\n **  UART WRITE FAILED  ** \r\n");
                    return err;
                }
            }

            /* Set intensity only for valid data */
            if(b_valid_data)
            {
                /* Change intensity of LED */
                err = set_intensity(intensity, TIMER_PIN);
                if (FSP_SUCCESS != err)
                {
                    APP_ERR_PRINT ("\r\n** GPT failed while changing intensity ** \r\n");
                    return err;
                }
                /* Resetting the temporary buffer */
                memset(g_temp_buffer, RESET_VALUE, MAX_UART_SIZE);
                b_valid_data = false;

                err = uart_print_user_msg((uint8_t *)"\r\nSet next value\r\n");
                if (FSP_SUCCESS != err)
                {
                    return err;
                }
            }
        } 
    }
}
#endif



fsp_err_t Uarts_Initialize(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_SCI_UART_Open(&LTE_Uart_ctrl , &LTE_Uart_cfg);
    err = R_SCI_UART_Open(&Debug_Uart1_ctrl, &Debug_Uart1_cfg);

    return err;
}
fsp_err_t Uarts_Close(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_SCI_UART_Close(&LTE_Uart_ctrl);
    err = R_SCI_UART_Close(&Debug_Uart1_ctrl);

    return err;
}
/* Callback function */
void Debug_uart_callback(uart_callback_args_t *p_args)
{
    /* TODO: add your own code here */
    /* Logged the event in global variable */
      g_DEBUG_uart_event = (uint8_t)p_args->event;

      /* Reset g_temp_buffer index if it exceeds than buffer size */
      if(MAX_UART_SIZE == g_DEBUG_counter_var)
      {
          g_DEBUG_counter_var = RESET_VALUE;
      }

      if(UART_EVENT_RX_CHAR == p_args->event)
      {
          g_DEBUG_temp_buffer[g_DEBUG_counter_var++] = (uint8_t ) p_args->data;
       //   g_LTE_counter_var = RESET_VALUE;
          g_DEBUG_timout_cntr=3;  //Start new timer for end of rx chars
      }

}


void LTE_uart_callback(uart_callback_args_t *p_args)
{
    /* Logged the event in global variable */
    g_LTE_uart_event = (uint8_t)p_args->event;

    /* Reset g_temp_buffer index if it exceeds than buffer size */
    if(MAX_UART_SIZE == g_LTE_counter_var)
    {
        g_LTE_counter_var = RESET_VALUE;
    }

    if(UART_EVENT_RX_CHAR == p_args->event)
    {
        g_LTE_temp_buffer[g_LTE_counter_var++] = (uint8_t ) p_args->data;
     //   g_LTE_counter_var = RESET_VALUE;
        g_LTE_timout_cntr=3; //Start new timer for end of rx chars
    }
}
/* Callback function */


void Pulser_timer_generic_callback(timer_callback_args_t *p_args)
{
    /* TODO: add your own code here */
    // check global uarts flags
	if (g_LTE_timout_cntr>RESET_VALUE)
	{
	    g_LTE_timout_cntr--;
	    if (g_LTE_timout_cntr == 0)
	    { // End of message detected
	   // 	g_LTE_timout_cntr = RESET_VALUE;
	 //   	g_LTE_data_received_flag=true; // indication that message has received
//#ifdef DEBUG_UART_MODE
	    	memcpy (Ltutemputartin,g_LTE_temp_buffer,g_LTE_counter_var);
	    	Ltutmpindexin= g_LTE_counter_var;
//#endif
//14/9/22 moved to background task "modemprocess"	    	Gsm_Handler( g_LTE_temp_buffer, g_LTE_counter_var , DATA_IN_EVENT);
	        g_LTE_counter_var=0;
	    }
	}
    if (g_DEBUG_timout_cntr>RESET_VALUE)
    {
        g_DEBUG_timout_cntr--;
        if (g_DEBUG_timout_cntr ==0)
        { // End of message detected
         //   g_DEBUG_timout_cntr = RESET_VALUE;
        //    g_DEBUG_data_received_flag=true; // indication that message has received
            memcpy (bufintmp,g_DEBUG_temp_buffer,g_DEBUG_counter_var);
  //          HandleConfigRxFromMETER(bufintmp,g_DEBUG_counter_var);
            HandleRXRFGwWrapperFromMeter(bufintmp,g_DEBUG_counter_var);
            g_DEBUG_counter_var=0;

        }
    }
    // modem seq process
    if (TimingDelay != 0)
    {
      if ((--TimingDelay) == 0)
      {
        if(modem_seq_state)
        {
            ModemSequence(MODEM_CONT_SEQUENCE);
        }
      }
    }
    else
         if (GsmTimer)
        {
            if ((--GsmTimer) ==0)
            {
 //           14/09/22 removed to background task "modemprocess"    Gsm_Handler( 0, 0 , TIME_OUT_EVENT);
                LtuGSM_Timeout = true;

            }
        }
#if 0
         else
           if (WaitAfterKC)
           {
               if (--WaitAfterKC ==0)
                   SetGsmCompleate();
           }

#endif

    if ((!ToggleLedsReq) && (++yos_temp%100 == 0))
          ToggleLeds(); // every 2 seconds)


}

void  SendData2Modem(uint8_t * DataIn, uint16_t Len)
{
    fsp_err_t status;
    status = R_SCI_UART_Write(&LTE_Uart_ctrl, DataIn, Len);
//#ifdef DEBUG_UART_MODE4
    Ltutmpindexout = Len;
    memcpy (Ltutemputartout,DataIn,Len);
//#endif
}

void SendData2Meter(uint8_t * DataIn, uint16_t Len)
{

    fsp_err_t status;
    status = R_SCI_UART_Write(&Debug_Uart1_ctrl, DataIn, Len);
}

