
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
#include "ql_pin_cfg.h"
#include "longi4g.h"

#include "gpio_int_demo.h"

/*===========================================================================
 * Macro Definition
 ===========================================================================*/
#define QL_GPIOINTDEMO_LOG_LEVEL             QL_LOG_LEVEL_INFO
#define QL_GPIOINTDEMO_LOG(msg, ...)         QL_LOG(QL_GPIOINTDEMO_LOG_LEVEL, "ql_GPIOINTDEMO", msg, ##__VA_ARGS__)
#define QL_GPIOINTDEMO_LOG_PUSH(msg, ...)    QL_LOG_PUSH("ql_GPIOINTDEMO", msg, ##__VA_ARGS__)
extern uint32_t sendlastgasp(void );
extern uint32_t SendHeartBit2Server(void);
extern uint16_t blinkFlag;
#define DYING_GASP_IND  55
bool last_gasp = false;
/*===========================================================================
 * Variate
 ===========================================================================*/


/*===========================================================================
 * Functions
 
 
 ===========================================================================*/

static ql_task_t gpioint_task 	= NULL;

static void _gpioint_callback01(void *ctx)
{
    ql_LvlMode  gpio_lvl;
 //   ql_event_t ql_event;
    ql_gpio_get_level(GPIO_0, &gpio_lvl);  // y.f. see if we can remove this 
    if(gpio_lvl == LVL_LOW)
	{ 	
		last_gasp = true;
#if 0		
		stop_interrupt= true;
		ql_event.param1 = 0;
		ql_event.id = DYING_GASP_IND;
		ql_rtos_event_send(gpioint_task, &ql_event); 
		blinkFlag =DATA_IN_METER;
#endif
		
	}
	else 
	{
	//	ql_event.param1 = 1;

	}	
  
}



static void ql_gpioint_demo_thread(void *param)
{
  //  ql_event_t event;
    ql_gpio_set_direction(GPIO_0, GPIO_INPUT);
    ql_gpio_set_pull(GPIO_0, PULL_NONE);
    ql_pin_set_func(QUEC_PIN_DNAME_GPIO_0, 0);      // set GPIO0
    ql_int_register(GPIO_0, EDGE_TRIGGER, DEBOUNCE_EN, EDGE_FALLING, PULL_NONE, _gpioint_callback01, NULL);
    ql_int_enable(GPIO_0);
    ql_rtos_task_sleep_s(2); 

    while(1)
    {
		ql_event_t ql_event = {0};
		if(ql_event_try_wait(&ql_event) != 0)
		{
			continue;
		}
		
		if( ql_event.id == DYING_GASP_IND )
		{
			ql_int_disable(GPIO_0);
			ql_int_unregister(GPIO_0);
			sendlastgasp();
		
		
		}
    }

    ql_rtos_task_delete(NULL);
}

void ql_gpioint_app_init(void)
{
    QlOSStatus err = QL_OSI_SUCCESS;
 //   ql_task_t gpioint_task = NULL;

    err = ql_rtos_task_create(&gpioint_task, 1024, APP_PRIORITY_NORMAL, "ql_gpiointdemo", ql_gpioint_demo_thread, NULL, 1);
    if( err != QL_OSI_SUCCESS )
    {
        QL_GPIOINTDEMO_LOG("gpio int demo task created failed");
    }
}


