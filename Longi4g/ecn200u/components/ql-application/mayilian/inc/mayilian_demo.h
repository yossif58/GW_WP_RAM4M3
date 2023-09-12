/**  @file
  mayilian_demo.h

  @brief
  This file is used to define mayilian demo for different Quectel Project.

*/

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


#ifndef MAYILIAN_DEMO_H
#define MAYILIAN_DEMO_H


#ifdef __cplusplus
extern "C" {
#endif


/*========================================================================
 *  Variable Definition
 *========================================================================*/
#define MAYILIAN_DEMO_TASK_PRIO           12
#define MAYILIAN_DEMO_TASK_STACK_SIZE     8*1024
#define MAYILIAN_DEMO_TASK_EVENT_CNT      4

/*========================================================================
 *  function Definition
 *========================================================================*/
void ql_mayilian_demo_init(void);
void mayilin_demo1(void );



#ifdef __cplusplus
} /*"C" */
#endif

#endif /* MAYILIAN_DEMO_H */


