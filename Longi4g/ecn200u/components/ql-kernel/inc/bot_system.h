/*
 *
 * Copyright (C) 2020-2021 Alibaba Group Holding Limited
*/
/**
 * @file bot_system.h
 *
 * @brief bot system header file.
 *
 * @details This header file defines the functional interface of os.
 *
 */

#ifndef __BOT_SYSTEM_H__
#define __BOT_SYSTEM_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define QUEC_BOT_PDP_SIMID 0
#define QUEC_BOT_CONTEXT_ID 1


#include "bot_system_utils.h"
#include "bot_platform.h"


#define PLATFORM_WAIT_INFINITE (~0)

/* MaaS heap info */
typedef struct _bot_mem_para {
    void        *start; /* start addr of bot heap for bot sdk */
    unsigned int len;   /* length of bot heap  for bot sdk    */
} bot_mem_para_t;

/* Task creation parameters */
typedef struct _antc_thread {
    unsigned int            priority;     /*initial thread priority */
    void                    *stack_addr;   /* thread stack address malloced by caller, use system stack by . */
    int                     stack_size;   /* stack size requirements in bytes; 0 is default stack size */
    char                    *name;         /* thread name. */
} bot_task_param_t;


/**
 * @brief Memory Alloc
 *
 * @param[in]  size   the length to alloc
 *
 * @return null on fail, otherwise will be success
 */
void* bot_os_alloc(unsigned int size);

/**
 * @brief Memory Free
 *
 * @param[in]  addr   the address of the memory to free
 *
 */
void bot_os_free(void* addr);

/**
 * @brief Memory Realloc
 *
 * @param[in]  addr   the address of the memory to free
 * @param[in]  size   the length to alloc
 * 
 * @return null on fail, otherwise will be success
 */
void* bot_os_realloc(void* addr, unsigned int newsize);

/**
 * Get current time in milliseconds.
 *
 * @return  type long long  elapsed time in milliseconds from system starting.
 */
unsigned long long bot_uptime(void);

/**
 * Get current time.
 * [out] tv    get the number of seconds and microseconds
 * [out] dummy reserved
 * @return  0: success, -1: failure.
 */
int bot_gettimeofday(struct timeval *tv, void* dummy);

/**
 * Task sleep
 *
 * @param[in]  ms  time for task to sleep in milliseconds.
 *
 * @return  none
 */
void bot_msleep(unsigned int ms);

/**
 * Create a semaphore.
 *
 * @return    pointer of semaphore object
 *            NULL means fail.
 */
void *bot_sem_create(void);

/**
 * Delete a semaphore.
 *
 * @param[in]  sem  semaphore object.
 *
 * @return  none.
 */
void bot_sem_delete(void *sem);

/**
 * Release a semaphore.
 *
 * @param[in]  sem  semaphore object.
 *             It will wakeup one task which is waiting for the sem according to task priority.
 *
 * @return  none.
 */
void bot_sem_post(void *sem);

/**
 * Acquire a semaphore.
 *
 * @param[in]  sem         semaphore object.
 * @param[in]  timeout_ms  waiting until timeout in milliseconds.
 *                         value:  PLATFORM_WAIT_INFINITE: wait mutex for ever.
 *                          0: return immediately if not get mutex.
 *
 * @return  0: success, otherwise: fail.
 */
int bot_sem_wait(void *sem, unsigned int timeout_ms);


/**
 * Create a queue.
 *
 * @param[in]  queue_length  the length of the buffer.
 * @param[in]  item_size     the max size for one msg.
 *
 * @return  pointer to the queue.
 *          NULL means fail
 */
void *bot_queue_create(int queue_length, int item_size);

/**
 * Delete a queue.
 *
 * @param[in]  queue  pointer to the queue.
 *
 * @return  0: success, otherwise: fail.
 */
int bot_queue_delete(void *queue);

/**
 * Receive msg from a queue.
 *
 * @param[in]   queue  pointer to the queue.
 * @param[out]  msg    buffer to store message.
 * @param[in]   size   size of the buffer to store message.
 * @param[in]   ms     ms to wait before receive.
 *
 * @return  0: success, otherwise: fail.
 */
int bot_queue_recv(void *mq, void *msg, unsigned int size, unsigned int ms);


/**
 * Send a msg to the front of a queue.
 *
 * @param[in]  queue  pointer to the queue.
 * @param[in]  msg    buffer of the message to send.
 * @param[in]  size   size of the message to send.
 * @param[in]  ms     time to wait before send in mi
 *
 * @return  0: success, otherwise: fail.
 */
int bot_queue_send(void *queue, void *msg, unsigned int size, unsigned int ms);

/**
 * Send a msg to the front of a queue in isr.
 *
 * @param[in]  queue  pointer to the queue.
 * @param[in]  msg    msg to send.
 * @param[in]  size   size of the msg.
 *
 * @return  0: success, otherwise: fail.
 */
int bot_queue_send_isr(void *queue, void *msg, unsigned int size);


/**
 * Create a task.
 *
 * @param[in]  task_handle          handle.
 * @param[in]  work_routine         task function.
 * @param[in]  arg                  argument of the function.
 * @param[in]  task_param           param of the task.
 * @param[in]  stack_used           stack-buf: if stack_buf==NULL, provided by kernel.
 *
 * @return  0: success, otherwise: fail.
 */
int bot_task_create(
            void **task_handle,
            void (*work_routine)(void *),
            void *arg,
            bot_task_param_t *task_param,
            int *stack_used);

/**
 * Delete a task by name.
 *
 * @param[in]  task_handle  task handle.
 *
 * @return  0: success, otherwise: fail.
 */
int bot_task_delete(void *task_handle);

/**
 * @brief   get task default priority
 *
 * @return  default priority
 */
int bot_task_default_priority_get();


/**
 * Alloc a mutex.
 *
 * @return    pointer of mutex object, mutex object must be alloced,
 *            hdl pointer will refer a kernel obj internally.
 */
void *bot_mutex_create(void);

/**
 * Free a mutex.
 *
 * @param[in]  mutex  mutex object, mem refered by hdl pointer in
 *
 * @return  none.
 */
void bot_mutex_delete(void *mutex);

/**
 * Lock a mutex.
 *
 * @param[in]  mutex    mutex object, it contains kernel obj pointer.
 *
 * @return  none.
 */
void bot_mutex_lock(void *mutex);

/**
 * Unlock a mutex.
 *
 * @param[in]  mutex  mutex object, it contains kernel obj pointer.
 *
 * @return  none.
 */
void bot_mutex_unlock(void *mutex);


/**
 * Get mem info for MaaS sdk.
 *
 * @param[out]  mem_info, describe the address and length for MaaS heap.
 *
 * @return  0: success, otherwise: fail.
 */
int bot_mem_info_get(bot_mem_para_t *mem_info);

/**
 * Platform initialize, which is called in bot SDK init function.
 *
 *
 * @return  0: success, otherwise: fail.
 */
int bot_platform_init(void);

#if defined(__cplusplus)
}
#endif


#endif /* __BOT_SYSTEM_H__ */


