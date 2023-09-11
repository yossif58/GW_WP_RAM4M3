/***********************************************************************************************************************
 * File Name    : srec2Bin.c
 * Description  : Contains data structures and functions used for converting SREC file to bin flash to scratcpade location in flash
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
#include "flash_hp_ep.h"
#include "crc.h"
#include "utils.h"
#include "kk_srec.h"
//#include "PulserProtocol.h"
#define FLASH_BLOCK_SIZE              2048
#define START_SCRATCPAD_BLOCK         34     // 0x11000
#define START_IMAGE_1_BLOCK           3      // 0x1800
#define MAX_BLOCKS_IN_IMAGE           30
static uint8_t flash_blocks_num;
static void start_app(uint32_t pc, uint32_t sp) ;
static uint8_t block_data[FLASH_BLOCK_SIZE];
static uint16_t current_position_in_block = 0;
//extern PulserNewFirmwareStruct NewFirmware;
void ClearflashBlock_Data(void)
{
  memset(block_data,0,2048);
  current_position_in_block =0;
}
void PrepareFlashProgramming(void)
{
  ClearflashBlock_Data();
  flash_blocks_num = START_SCRATCPAD_BLOCK;
}


void testmessage_in(void);
uint8_t readbuff[250];
#if 0
void testmessage_in(void)
{
 //   fsp_err_t err = FSP_SUCCESS;    // error status
    uint32_t total_size = 0;
    PrepareFlashProgramming();
     //flash_lp_code_flash_operations(flash_blocks_num++,block_data,64);
    while (total_size < ( MAX_BLOCKS_IN_IMAGE)*FLASH_BLOCK_SIZE)
    { // simulates messages received from MDM.
      // read 128 bytes from imag_1 and wtites it to image_2
        memcpy(readbuff, (uint8_t*)FLASH_CF_BLOCK(START_IMAGE_1_BLOCK)+total_size , 128);
        total_size+=128;
        ReadBlockOtap(128,readbuff);
    }
}


fsp_err_t ReadBlockOtap(int argc, uint8_t *bufin)
{
    fsp_err_t err= FSP_SUCCESS;

    uint16_t tmp_count_in_message_input=0;
    while (tmp_count_in_message_input< argc)
    {
            block_data[current_position_in_block++]   = bufin[tmp_count_in_message_input++] ;
            if (current_position_in_block >=FLASH_BLOCK_SIZE)
            { // program block to scratcpad flash
                __disable_irq();
                flash_lp_code_flash_operations(flash_blocks_num++,block_data,FLASH_BLOCK_SIZE);
                __enable_irq();
                ClearflashBlock_Data();
            }
    }
    // check enf of file size of file is driven from first packets that hold the header in
    // In theory we wont get heae because the bin file's size is on bounderies of 2048
    if ((uint32_t )(flash_blocks_num*FLASH_BLOCK_SIZE+argc) >= NewFirmware.NewTotalBytesInPacket)
    { // handle last packet
        __disable_irq();
        flash_lp_code_flash_operations(flash_blocks_num,block_data,FLASH_BLOCK_SIZE);
        __enable_irq();
      //  ClearflashBlock_Data();
    }
    return err;
}
#endif

/*******************************************************************************************************************//**
 * Cleans up resources used by bootloader port and branches to application image.
 **********************************************************************************************************************/
void RM_MCUBOOT_PORT_BootApp (void)
{

    __disable_irq(); // y.f. 12/7/22 add
    /* The vector table is located after the header. Only running from internal flash is supported now. */
    //   uint32_t vector_table = rsp->br_image_off + rsp->br_hdr->ih_hdr_size;
    uint32_t vector_table = 0;  // bootloader address start
    uint32_t * app_image = (uint32_t *) vector_table;
    uint32_t   app_sp    = app_image[0];
    uint32_t   app_pc    = app_image[1];
    /* Set the applications vector table. */
    SCB->VTOR = (uint32_t) vector_table;
    /* Disable MSP monitoring. */
//    R_MPU_SPMON->SP[0].CTL = 0;
    /* Set SP and branch to PC. */
    start_app(app_pc, app_sp);
}



/* This function jumps to the application image. */
static void start_app (uint32_t pc __attribute__((unused)), uint32_t sp __attribute__((unused))) {
    /* This is a naked/stackless function. Do not pass arguments to the inline assembly when the GCC compiler is
     * used. */
    __asm volatile (

        /* Set stack pointer. */
        "MSR     MSP, R1                         \n"
        "DSB                                     \n"
        "ISB                                     \n"

        /* Branch to application. */
        "BX      R0                              \n"
        );
}





