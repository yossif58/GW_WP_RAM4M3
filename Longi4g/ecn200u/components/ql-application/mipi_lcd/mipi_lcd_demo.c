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
#include <string.h>
#include <stdlib.h>

#include "ql_api_osi.h"
#include "ql_log.h"

#include "mipi_lcd_demo.h"
#include "a_rabbit.h"
#include "rabbit_mouse.h"


/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define QL_MIPILCDDEMO_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_MIPILCDDEMO_LOG(msg, ...)         QL_LOG(QL_MIPILCDDEMO_LOG_LEVEL, "ql_MIPILCDDEMO", msg, ##__VA_ARGS__)
#define QL_MIPILCDDEMO_LOG_PUSH(msg, ...)    QL_LOG_PUSH("ql_MIPILCDDEMO", msg, ##__VA_ARGS__)

/*===========================================================================
 * Variate
 ===========================================================================*/
// cmd send example
// example is send sw reset cmd
#if 0
static const char test_cmd[] = {0x01};
static const ql_mipi_cmd_s cmd_test[] = {
    {QL_MIPI_LCD_CMD_SWRITE,0,sizeof(test_cmd),test_cmd},
};
#endif
#define LCD_RED        0xf800
#define LCD_GREEN      0x07e0
#define LCD_BLUE       0x001f
    
#define LCD_WHITE      0xffff
#define LCD_BLACK      0x0000
    
#define LCD_YELLOW     0xffe0
#define LCD_PURPLE     0xf81f

#define QL_IMGW_LCDC   480
#define QL_IMGH_LCDC   854

uint16_t Test_image1[30*50] = {};
uint16_t Test_image2[50*100] = {};
uint16_t Test_image3[100*200] = {};

/*===========================================================================
 * Functions
 ===========================================================================*/
void image_test_set(void)
{
    uint16_t count = 0;
    uint16_t image1_len = sizeof(Test_image1)/sizeof(Test_image1[0]);
    uint16_t image2_len = sizeof(Test_image2)/sizeof(Test_image2[0]);
    uint16_t image3_len = sizeof(Test_image3)/sizeof(Test_image3[0]);

    for( count = 0; count < image1_len; count++ )
    {
        Test_image1[count] = LCD_YELLOW;
    }
    for( count = 0; count < image2_len; count++ )
    {
        Test_image2[count] = LCD_PURPLE;
    }
    for( count = 0; count < image3_len; count++ )
    {
        Test_image3[count] = LCD_RED;
    }
}


void ql_mipi_lcd_write_area(void *buffer, uint16_t start_x, uint16_t start_y, uint16_t end_x, uint16_t end_y)
{	
    if( buffer == NULL )
    {
        return;
    }

    if( (start_x >= QL_IMGW_LCDC)
     || (end_x >= QL_IMGW_LCDC)
     || (start_y >= QL_IMGH_LCDC)
     || (end_y >= QL_IMGH_LCDC) )
    {
        return;
    }

    uint16_t *area_data = (uint16_t *)buffer;
    uint16_t height, width;

    for( height = start_y; height < (end_y + 1); height++ )
    {
        for( width = start_x; width < (end_x + 1); width++ )
        {
            lcd_temp_buffer[height * QL_IMGW_LCDC + width] = *area_data;
            area_data++;
        }
    }
	
	ql_mipi_lcd_write_screen(lcd_temp_buffer);
}

/*********************************************************************************
* Attention:
*    1. To run mipi demo, you need to modify the app partition and app ram size. 
*       It is recommended to modify the app partition to 0x220000 and app ram to 0x200000
*    2. LCD backlight needs to be controlled by the customer
*    3. lcd_temp_buffer can be replaced by the customer's own malloc memory, customer 
*       can adjust the partition according to the actual use
*    4. Currently only support 480*854 mipi lcd (st7701s)
**********************************************************************************/
static void ql_mipi_lcd_demo_thread(void *param)
{
    QL_MIPILCDDEMO_LOG("mipi_lcd demo thread enter, param 0x%x", param);

    //ql_event_t event;
    ql_mipi_lcd_info_t lcd_info = {480, 854};

    ql_mipi_lcd_init(&lcd_info);
	image_test_set();

    while(1)
    {
        //ql_event_wait(&event, 1);

		ql_mipi_lcd_write_screen(rabbit_mouse);
        ql_rtos_task_sleep_ms(3000);

        ql_mipi_lcd_write_screen(lcd_temp_buffer);
        ql_rtos_task_sleep_ms(3000);
	
		ql_mipi_lcd_write_area(rabbit_mouse, 0, 0, 480-1, 400-1);
		ql_rtos_task_sleep_ms(3000);

        ql_mipi_lcd_write_area(Test_image1, 70, 100, 100-1, 150-1);
        ql_rtos_task_sleep_ms(3000);

        ql_mipi_lcd_write_area(Test_image2, 120, 150, 170-1, 250-1);
        ql_rtos_task_sleep_ms(3000);

		ql_mipi_lcd_write_area(Test_image3, 200, 200, 300-1, 400-1);
        ql_rtos_task_sleep_ms(3000);

        //ql_mipi_lcd_write_cmd((void *)cmd_test);    // mipi lcd sw reset
    }

    ql_rtos_task_delete(NULL);
}

void ql_mipi_lcd_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;
    ql_task_t mipilcd_task = NULL;

    err = ql_rtos_task_create(&mipilcd_task, 5*1024, APP_PRIORITY_NORMAL, "ql_mipilcddemo", ql_mipi_lcd_demo_thread, NULL, 1);
    if( err != QL_OSI_SUCCESS )
    {
        QL_MIPILCDDEMO_LOG("mipi_lcd demo task created failed");
    }
}


