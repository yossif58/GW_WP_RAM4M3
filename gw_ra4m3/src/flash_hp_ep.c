/***********************************************************************************************************************
 * File Name    : flash_hp_ep.c
 * Description  : Contains macros, data structures and functions used in flash_hp_ep.c
 ***********************************************************************************************************************/
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

#include "common_utils.h"
#include "main.h"
#include "flash_hp_ep.h"
/*******************************************************************************************************************//**
 * @addtogroup FLASH_HP_EP
 * @{
 **********************************************************************************************************************/


/* Flags, set from Callback function */
static volatile _Bool g_b_flash_event_not_blank = false;
static volatile _Bool g_b_flash_event_blank = false;
static volatile _Bool g_b_flash_event_erase_complete = false;
static volatile _Bool g_b_flash_event_write_complete = false;
/* Static Function */
static fsp_err_t blankcheck_event_flag(void);
bool FLashCheck(void);

//fsp_err_t WriteInternalFlashConfig(bool reset_req);

/*
 * private function declarations
 */
//extern long long freeztotalconsumption;
extern uint32_t upgradestate;
extern RepeaterConfigStruct RepeaterConfig;
uint8_t read_buff[FLASH_HP_CF_BLOCK_SIZE_8KB] =
{ RESET_VALUE };  // read data buffer
uint8_t write_buff[FLASH_HP_CF_BLOCK_SIZE_8KB] =
{ RESET_VALUE }; // write data buffer
bool FLashCheck(void)
{
//    static int8_t block_num = 1;
    uint16_t crc;
    uint16_t block_num;
    uint8_t *pBuf=  (uint8_t *)&RepeaterConfig;
    //   for (block_num = 0 ; block_num <sizeof( RepeaterConfigStruct)/FLASH_HP_DF_BLOCK_SIZE ;block_num++)
    //        memcpy ((unsigned char*) &pBuf[block_num*FLASH_HP_DF_BLOCK_SIZE],(uint8_t*)(FLASH_HP_DF_BLOCK_1+block_num*FLASH_HP_DF_BLOCK_SIZE), FLASH_HP_DF_BLOCK_SIZE);
           memcpy ((unsigned char*) pBuf,(uint8_t*) FLASH_HP_DF_BLOCK_1, sizeof( RepeaterConfigStruct));

    crc = CrcBlockCalc ((unsigned char*) &RepeaterConfig, sizeof(RepeaterConfigStruct) - 2);
    if (RepeaterConfig.Crc != crc)
        return false;
    return true;
}


fsp_err_t WriteInternalFlashConfig(void)
{
    uint32_t ConfigSize;
    uint8_t block_num=1;// RepeaterConfig starts from block num 1
    fsp_err_t err;
    uint8_t *pBuf;
    uint8_t bind;
    uint32_t write_buf[16];
    RepeaterConfig.Crc = CrcBlockCalc ((unsigned char*) &RepeaterConfig, sizeof(RepeaterConfigStruct) - 2);
    ConfigSize = sizeof(RepeaterConfigStruct);
    pBuf = (uint8_t*) &RepeaterConfig;
//    while (ConfigSize % 4)
//          ConfigSize++; // be sure that we have modulo 4 size
    // write FLASH_HP_DF_BLOCK_SIZE bytes each time
    for (bind = 0 ; bind < ConfigSize/FLASH_HP_DF_BLOCK_SIZE +1 ;bind++)
    {
        memcpy((uint8_t*) write_buf ,&pBuf[bind*FLASH_HP_DF_BLOCK_SIZE], FLASH_HP_DF_BLOCK_SIZE);
        err= flash_hp_data_flash_operations(block_num+bind,write_buf,FLASH_HP_DF_BLOCK_SIZE);
        if (err != FSP_SUCCESS)        return err;
    }
    return FSP_SUCCESS;
}

fsp_err_t ResetUpgradeState(void)
{

    memcpy (&upgradestate, ( unsigned char*) (__IO uint32_t*) FLASH_HP_DF_BLOCK_0 ,4);
    if (upgradestate == UPGRADE_REQ_IS_WAITING)
    {
        upgradestate =0;
        WriteBootStateInFlash(&upgradestate);
        R_SCI_UART_Write (&Debug_Uart1_ctrl, (uint8_t*)"debugGW card was upgraded\r\n",27); // y.f. 10/1/22 Debug via Silicon
    }
    return FSP_SUCCESS;
}

fsp_err_t WriteBootStateInFlash(uint32_t * UpgradeSate)
{
   return  flash_hp_data_flash_operations(0,UpgradeSate,4);
}

/*******************************************************************************************************************//**
 * @brief This function is called from the RTT input to do the code flash operations.
 * @param[IN]   None
 * @retval      FSP_SUCCESS             Upon successful Flash_HP code flash operations.
 * @retval      Any Other Error code    Upon unsuccessful code flash operations
 **********************************************************************************************************************/
fsp_err_t flash_hp_code_flash_operations(uint8_t block_num,uint32_t* pDatain,uint16_t size)
{
    fsp_err_t err = FSP_SUCCESS;
    flash_result_t blank_check_result = FLASH_RESULT_BLANK;
    uint8_t read_buffer[BLOCK_SIZE] = {RESET_VALUE};
    /* Erase Block */
    err = R_FLASH_HP_Erase(&g_flash_ctrl, FLASH_HP_CF_BLCOK_0+block_num*FLASH_HP_CF_BLOCK_SIZE_8KB, 1);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nErase API failed, Restart the Application");
        return err;
    }
    APP_PRINT("\r\nErase successful");

    /* Blank Check */
    err = R_FLASH_HP_BlankCheck(&g_flash_ctrl, FLASH_HP_CF_BLCOK_0+block_num*FLASH_HP_CF_BLOCK_SIZE_8KB, size, &blank_check_result);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nBlankCheck API failed, Restart the Application");
        return err;
    }
    APP_PRINT("\r\nBlankCheck API Successful");

    /* Validate the blank check result */
    if (FLASH_RESULT_BLANK == blank_check_result)
    {
        APP_PRINT("\r\n FLASH is blank ");
    }
    else if (FLASH_RESULT_NOT_BLANK == blank_check_result)
    {
        APP_ERR_PRINT("\r\n Flash is not Blank, not to write the data. Restart the application");
        return (fsp_err_t)FLASH_RESULT_NOT_BLANK;
    }

    /* Write code flash data*/
    err = R_FLASH_HP_Write(&g_flash_ctrl,  pDatain,FLASH_HP_CF_BLCOK_0+block_num*FLASH_HP_CF_BLOCK_SIZE_8KB, size);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nWrite API failed, Restart the Application");
        return err;
    }
    APP_PRINT("\r\nWriting flash data is successful\r\n");

    /*Read code flash data */
    memcpy(read_buffer, (uint8_t *) FLASH_HP_CF_BLCOK_0+block_num*FLASH_HP_CF_BLOCK_SIZE_8KB, size);

    /* comparing the write_buffer and read_buffer */
    if (RESET_VALUE == memcmp(read_buffer, pDatain, size))
    {
        APP_PRINT("\r\nRead and Write buffer is verified and successful");
        /* Print the read data on the RTT terminal */
        APP_PRINT("\r\nRead Data : \r\n");
        READ_DATA_PRINT(read_buffer);
        return FSP_SUCCESS;
    }
    else
    {
        APP_PRINT("Read and Write buffer is verified and not successful");
        return FSP_ERR_WRITE_FAILED;
    }
#if 0
    /* Erase block again */
    APP_PRINT("\r\nErase block again");
    err = R_FLASH_HP_Erase(&g_flash_ctrl, FLASH_HP_CF_BLCOK_0+block_num*FLASH_HP_CF_BLOCK_SIZE_8KB, BLOCK_NUM);
    if(FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nErase API failed, Restart the Application");
        return err;
    }
    APP_PRINT("\r\nErase successful");

#if !(defined (BOARD_RA6M5_EK) || defined (BOARD_RA6M4_EK) || defined (BOARD_RA4M3_EK)||defined(BOARD_RA4M2_EK)) //Not supported for this MCU
    /* Set Access window.
     * CAUTION: Highly recommended not to use this function if not aware of consequences OR
     * use it with the accessWindowClear API at the end of application.
     * This API locks the Code flash and the lock retains even after power cycle.
     * Which means, even after power cycle, user will not be able to program the code to code flash if the
     * access window is wrongly set.
     *
     * WORKAROUND: If uses uses accessWindowSet and locks the window. Flash can be unlocked by running
     * different application performing just "open" call and "accessWindowClear()" running from the RAM.
     */
    err = R_FLASH_HP_AccessWindowSet(&g_flash_ctrl, FLASH_HP_CF_BLOCK_3, FLASH_HP_CF_BLOCK_7);
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nAccessWindowSet API failed, Restart the Application");
        return err;
    }
    APP_PRINT("\r\nAccessWindowSet successful");

    /* Write code flash data*/
    err = R_FLASH_HP_Write(&g_flash_ctrl, (uint32_t) write_buffer,FLASH_HP_CF_BLOCK_5, BLOCK_SIZE);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nWrite API failed, Restart the Application");
        return err;
    }

    APP_PRINT("\r\nWriting flash data is successful");

    /* Clear Flash Access Window */
    err = R_FLASH_HP_AccessWindowClear(&g_flash_ctrl);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nAccessWindoeClear API failed, Restart the Application");
        return err;
    }
    APP_PRINT("\r\nAccess Window cleared ");
#endif
#endif

    return err;
}

/*******************************************************************************************************************//**
 * @brief This function is called from the RTT input to do the data flash operations.
 * @param[IN]   None
 * @retval      FSP_SUCCESS             Upon successful FLash_HP data flash operations.
 * @retval      Any Other Error code    Upon unsuccessful Flash_HP data flash operations.
 **********************************************************************************************************************/
fsp_err_t flash_hp_data_flash_operations(uint8_t block_num,uint32_t* pDatain,uint16_t size)
{
    fsp_err_t err = FSP_SUCCESS;
    flash_result_t blank_check_result = FLASH_RESULT_BLANK;
  //  uint8_t write_buffer[BLOCK_SIZE] = {RESET_VALUE};
    uint8_t read_buffer[BLOCK_SIZE]= {RESET_VALUE};
    /* Erase Block */
    err = R_FLASH_HP_Erase(&g_flash_ctrl, FLASH_HP_DF_BLOCK_0+block_num*FLASH_HP_DF_BLOCK_SIZE, 1);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nErase API failed, Restart the Application");
        return err;
    }

    /* Wait for the erase complete event flag, if BGO is SET  */
    if (true == g_flash_cfg.data_flash_bgo)
    {
        APP_PRINT("\r\n BGO has enabled");
        while (!g_b_flash_event_erase_complete);
        g_b_flash_event_erase_complete = false;
    }
    APP_PRINT("\r\nErase successful");

    /* Data flash blank check */
    err = R_FLASH_HP_BlankCheck(&g_flash_ctrl,  FLASH_HP_DF_BLOCK_0+block_num*FLASH_HP_DF_BLOCK_SIZE,FLASH_HP_DF_BLOCK_SIZE, &blank_check_result);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nBlankCheck API failed, Restart the Application");
        return err;
    }

    /* Validate the blank check result */
    if (FLASH_RESULT_BLANK == blank_check_result)
    {
        APP_PRINT("\r\n BlankCheck is successful");
    }
    else if (FLASH_RESULT_NOT_BLANK == blank_check_result)
    {
        APP_ERR_PRINT("\r\n BlankCheck is not blank,not to write the data. Restart the application");
        return (fsp_err_t)FLASH_RESULT_NOT_BLANK;
    }
    else if (FLASH_RESULT_BGO_ACTIVE == blank_check_result)
    {
        /* BlankCheck will update in Callback */
        /* Event flag will be updated in the blank check function when BGO is enabled */
        err = blankcheck_event_flag();
        if(FSP_SUCCESS != err)
        {
            return err;
        }
    }
    else
    {
        /* No Operation */
    }

    /* Write code flash data*/
    err = R_FLASH_HP_Write(&g_flash_ctrl,  pDatain, FLASH_HP_DF_BLOCK_0+block_num*FLASH_HP_DF_BLOCK_SIZE, size);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nWrite API failed, Restart the Application");
        return err;
    }
    /* Wait for the write complete event flag, if BGO is SET  */
    if (true == g_flash_cfg.data_flash_bgo)
    {
        while (!g_b_flash_event_write_complete);
        g_b_flash_event_write_complete = false;
    }

    APP_PRINT("\r\nWriting flash data is successful\r\n");

    /*Read code flash data */
    memcpy(read_buffer, (uint8_t *) FLASH_HP_DF_BLOCK_0+block_num*FLASH_HP_DF_BLOCK_SIZE, size);

    /* comparing the write_buffer and read_buffer */
    if (RESET_VALUE == memcmp(read_buffer, pDatain, FLASH_HP_DF_BLOCK_SIZE))
    {
        APP_PRINT("\r\nRead and Write buffer is verified and successful");
        /* Print the read data on the RTT terminal */
        APP_PRINT("\r\nRead Data : \r\n");
        READ_DATA_PRINT(read_buffer);
    }
    else
    {
        APP_PRINT("Read and Write buffer is verified and not successful");
        return FSP_ERR_WRITE_FAILED;
    }

    return err;
}

/*******************************************************************************************************************//**
 * @brief       This functions de-initializes Flash_HP module.
 **********************************************************************************************************************/
void flash_hp_deinit(void)
{
    fsp_err_t err = FSP_SUCCESS;
    err = R_FLASH_HP_Close(&g_flash_ctrl);
    /* Error Handle */
    if (FSP_SUCCESS != err)
    {
        APP_ERR_PRINT("\r\nClose API failed in BlankCheck API");
    }
}

/*******************************************************************************************************************//**
 * @brief Callback function for FLASH HP HAL
 **********************************************************************************************************************/
void bgo_callback(flash_callback_args_t *p_args)
{
    if (FLASH_EVENT_NOT_BLANK == p_args->event)
    {
        g_b_flash_event_not_blank = true;
    }
    else if (FLASH_EVENT_BLANK == p_args->event)
    {
        g_b_flash_event_blank = true;
    }
    else if (FLASH_EVENT_ERASE_COMPLETE == p_args->event)
    {
        g_b_flash_event_erase_complete = true;
    }
    else if (FLASH_EVENT_WRITE_COMPLETE == p_args->event)
    {
        g_b_flash_event_write_complete = true;
    }
    else
    {
        /*No operation */
    }

}

/*******************************************************************************************************************//**
 * @brief This function is called to set the blank check event flags
 * @param[IN]   None
 * @retval      FSP_SUCCESS             Upon successful Flash_HP is blank
 * @retval      Any Other Error code    Upon unsuccessful Flash_HP is not blank
 **********************************************************************************************************************/
static fsp_err_t blankcheck_event_flag(void)
{
    fsp_err_t err = FSP_SUCCESS;
    /* Wait for callback function to set flag */
    while (!(g_b_flash_event_not_blank || g_b_flash_event_blank));

    if (g_b_flash_event_not_blank)
    {
        APP_ERR_PRINT("\n\rFlash is not blank, not to write the data. Restart the application\n");
        /* Reset Flag */
        g_b_flash_event_not_blank = false;
        return (fsp_err_t)FLASH_EVENT_NOT_BLANK;
    }
    else
    {
        APP_PRINT("\r\nFlash is blank\n");
        /* Reset Flag */
        g_b_flash_event_blank = false;
    }
    return err;
}

/*******************************************************************************************************************//**
 * @} (end defgroup FLASH_HP_EP)
 **********************************************************************************************************************/
