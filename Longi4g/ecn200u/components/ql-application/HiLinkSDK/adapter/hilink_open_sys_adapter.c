/*
* Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
* Description: OS基础能力抽象层接口函数(需设备厂商实现)
*/
#include "hilink_open_sys_adapter.h"
#include "hilink_sec_random.h"
//#include "hilink_log.h" //Quectel larson.li 提供的SDK中未包含此头文件

/*
 * IoT OS 创建线程接口
 * 返回值: 返回0 成功, 其他 失败
 */
int HILINK_CreateTask(const void *handle, TaskInitParam *initParam)
{
    return 0;
}

/*
 * IoT OS 删除线程接口
 * 输入参数handle 删除线程句柄
 * 返回0 成功, 其他 失败
 */
int HILINK_DeleteTask(const void *handle)
{
    return 0;
}

/*
 * IoT OS 创建锁
 * 输入参数 HiLinkMutex *mutex  返回创建的锁
 * 返回0 成功, 其他 失败
 */
int HILINK_MutexCreate(HiLinkMutex *mutex)
{
    return 0;
}

/*
 * IoT OS 锁定
 * 输入参数HiLinkMutex *mutex 锁定的资源
 * 返回0 成功, 其他 失败
 */
int HILINK_MutexLock(HiLinkMutex *mutex)
{
    return 0;
}

/*
 * IoT OS 解锁接口函数
 * 输入参数HiLinkMutex *mutex
 * 返回0 成功, 其他 失败
 */
int HILINK_MutexUnlock(HiLinkMutex *mutex)
{
    return 0;
}

/*
 * IoT OS 删除锁
 * 输入参数HiLinkMutex *mexut  删除锁
 * 返回0 成功, 其他 失败
 */
int HILINK_MutexDestroy(HiLinkMutex *mutex)
{
    return 0;
}

/*
 * 创建信号量
 * handle是指向信号量的对象，count指定信号量值的大小
 * 返回0表示成功, 其他值表示失败
 */
int HILINK_SemaphoreCreate(HiLinkSemaphore *handle, int count)
{
    return 0;
}

/*
 * 给信号量的值减1
 * ms指定在信号量为0无法减1的情况下阻塞的时间限制
 * 返回0表示成功, 其他值表示失败
 */
int HILINK_SemaphoreTimedWait(HiLinkSemaphore handle, int ms)
{
    return 0;
}

/*
 * 给信号量的值加1
 * 返回0表示成功, 其他值表示失败
 */
int HILINK_SemaphorePost(HiLinkSemaphore handle)
{
    return 0;
}

/* 销毁信号量 */
void HILINK_SemaphoreDestroy(HiLinkSemaphore handle)
{
    return;
}

/*
 * IoT OS 获取当前时间
 * 输出参数表示当前设备启动后运行时间 单位毫秒
 * 返回0 成功，其他失败
 */
int HILINK_GetCurrTime(unsigned long *ms)
{
    return 0;
}

/*
 * 获取当前运行线程的线程号
 * 注意:freeRTOS和mico2.0返回的是当前线程的Handle
 */
unsigned long HILINK_GetCurrentTaskId(void)
{
    return 0;
}
#if 0
/* 厂商返回满足要求的硬件随机源，如果不满足，则根据安全组提供的方法进行优化 */
int ThirdRandomEntropy(unsigned int *entropyValue)
{
    *entropyValue = 88888; /* 88888是示例的硬件随机源值，厂家根据实际情况替换 */
    return 0;
}
#endif
/*
 * 获取随机数，厂家实现该接口
 * 如果模组有硬件熵源，需要调用HILINK_RegisterRandomEntropy注册硬件随机源，然后调用HILINK_SecRandom返回安全随机数
 * 如果模组没有硬件熵源，则厂家自行生成随机数，同时需要向华为报备
 */
int HILINK_Rand(void)
{
    /* 参考实现 */
    //HILINK_RegisterRandomEntropy(ThirdRandomEntropy);
    unsigned int securityRandom = 0;
	/*
    if (HILINK_SecRandom(&securityRandom) != 0) {
        //hilink_error("get security random number error!\n\r"); //Quectel larson.li SDK中未定义hilink_error
    }*/
    return (int)securityRandom;
}