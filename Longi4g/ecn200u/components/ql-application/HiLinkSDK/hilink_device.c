/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2021. All rights reserved.
 * Description: HiLink产品适配实现源文件
 * Notes: 该文件中的接口需要对外提供给第三方厂商使用，为了前向兼容，部分老接口暂不按最新编码规范整改.
 */
#include "hilink_device.h"
#include <stdlib.h>
#include "hilink.h"
//#include "hilink_log.h"
#include "hilink_bt_api.h"
//#include "ql_api_dev.h"
#include "ql_log.h"
//#include "ql_fs.h"

#define HILINK_DEV_LOG_LEVEL                QL_LOG_LEVEL_INFO
#define HILINK_DEV_LOG(msg, ...)			QL_LOG(HILINK_DEV_LOG_LEVEL, "hilink_dev", msg, ##__VA_ARGS__)

/* 设备类型定义 */
typedef struct {
    const char *sn;     /* 设备唯一标识，比如sn号，长度范围（0,40] */
    const char *prodId; /* 设备HiLink认证号，长度范围（0,5] */
    const char *model;  /* 设备型号，长度范围（0,32] */
    const char *dev_t;  /* 设备类型，长度范围（0,4] */
    const char *manu;   /* 设备制造商，长度范围（0,4] */
    const char *mac;    /* 设备MAC地址，固定32字节 */
    const char *btMac;    /* 蓝牙MAC地址，固定32字节 */
    const char *hiv;    /* 设备Hilink协议版本，长度范围（0,32] */
    const char *fwv;    /* 设备固件版本，长度范围[0,64] */
    const char *hwv;    /* 设备硬件版本，长度范围[0,64] */
    const char *swv;    /* 设备软件版本，长度范围[0,64] */
    int prot_t;         /* 设备协议类型，取值范围[1,3] */
} dev_info_t;

/* 设备类型英文名和厂商英文名长度之和不能超过17字节 */
typedef struct {
    const char *devTypeName; /* 设备类型英文名称 */
    const char *manuName;    /* 厂商英文名称 */
} DevNameEn;

/* 服务类型定义 */
typedef struct {
    const char *st;     /* 服务类型，长度范围（0,32] */
    const char *svc_id; /* 服务ID，长度范围（0,64] */
} svc_info_t;

/* 设备信息定义 */
dev_info_t dev_info = {
    "Device SN",
    PRODUCT_ID,
    DEVICE_MODEL,
    DEVICE_TYPE,
    MANUAFACTURER,
    "Device Mac",
    "Device BtMac",
    "1.0.0",
    "1.0.0",
    "1.0.0",
    "1.0.0",
    PROTOCOL_TYPE
};

/* 设备名称定义, 请确保设备类型英文名和厂商英文名长度之和不超过17字节 */
DevNameEn g_devNameEn = {
    DEVICE_TYPE_NAME,
    MANUAFACTURER_NAME
};

/* 服务信息定义 */
int gSvcNum = 1;
svc_info_t gSvcInfo[] = {
    { "binarySwitch", "switch" }
};

/* AC参数 */
unsigned char A_C[48] = {
    0x49, 0x3F, 0x45, 0x4A, 0x3A, 0x72, 0x38, 0x7B, 0x36, 0x32, 0x50, 0x3C, 0x49, 0x39, 0x62, 0x38,
    0x72, 0xCB, 0x6D, 0xC5, 0xAE, 0xE5, 0x4A, 0x82, 0xD3, 0xE5, 0x6D, 0xF5, 0x36, 0x82, 0x62, 0xEB,
    0x89, 0x30, 0x6C, 0x88, 0x32, 0x56, 0x23, 0xFD, 0xB8, 0x67, 0x90, 0xA7, 0x7B, 0x61, 0x1E, 0xAE
};

/* BI参数 */
char *bi_rsacipher = "";

static HILINK_BT_DevInfo g_btDevInfo;

/* 获取加密 AC 参数  */
unsigned char *hilink_get_auto_ac(void)
{
    return A_C;
}

/* 获取加密 BI 参数 */
char *hilink_get_auto_bi_rsa_cipher(void)
{
    return bi_rsacipher;
}

/*
 * 修改服务当前字段值
 * svcId为服务的ID，payload为接收到需要修改的Json格式的字段与其值，len为payload的长度
 * 返回0表示服务状态值修改成功，不需要底层设备主动上报，由Hilink Device SDK上报；
 * 返回-101表示获得报文不符合要求；
 * 返回-111表示服务状态值正在修改中，修改成功后底层设备必须主动上报；
 */
int hilink_put_char_state(const char *svcId, const char *payload, unsigned int len)
{
    return 0;
}

/*
 * 获取服务字段值
 * svcId表示服务ID。厂商实现该函数时，需要对svcId进行判断；
 * in表示接收到的Json格式的字段与其值；
 * inLen表示接收到的in的长度；
 * out表示保存服务字段值内容的指针,内存由厂商开辟，使用完成后，由Hilink Device SDK释放；
 * outLen表示读取到的payload的长度；
 * 返回0表示服务状态字段值获取成功，返回非0表示获取服务状态字段值不成功。
 */
int hilink_get_char_state(const char *svcId, const char *in, unsigned int inLen, char **out, unsigned int *outLen)
{
    return 0;
}

/*
 * 获取设备sn号
 * 参数len表示sn的最大长度,39字节
 * 参数sn表示设备sn
 * sn指向的字符串长度为0时将使用设备mac地址作为sn
 */
void HilinkGetDeviceSn(unsigned int len, char *sn)
{
    /* 在此处添加实现代码, 将sn赋值给*sn回传 */
    return;
}

/*
 * 获取设备相关版本号
 * 返回0表示版本号获取成功，返回其他表示版本号获取失败
 * 注意，此接口为HiLink内部调用函数
 */
int getDeviceVersion(char **firmwareVer, char **softwareVer, char **hardwareVer)
{
    *firmwareVer = FIRMWARE_VER;
    *softwareVer = SOFTWARE_VER;
    *hardwareVer = HARDWARE_VER;
    return 0;
}

/*
 * 获取SoftAp配网PIN码
 * 返回值为8位数字PIN码, 返回-1表示使用HiLink SDK的默认PIN码
 * 该接口需设备开发者实现
 */
int HiLinkGetPinCode(void)
{
    /* 测试时，这个数字可以随便改，只要是8位数字即可 */
    return -1;
}

/*
 * 查询当前设备敏感性标识
 * 返回0为非敏感设备，返回1为敏感设备
 */
int HILINK_IsSensitiveDevice(void)
{
    return 0;
}

/*
 * 通知设备的状态
 * status表示设备当前的状态
 * 注意，此函数由设备厂商根据产品业务选择性实现
 */
void hilink_notify_devstatus(int status)
{
    switch (status) {
        case HILINK_M2M_CLOUD_OFFLINE:
            /* 设备与云端连接断开，请在此处添加实现 */
            break;
        case HILINK_M2M_CLOUD_ONLINE:
            /* 设备连接云端成功，请在此处添加实现 */
            break;
        case HILINK_M2M_LONG_OFFLINE:
            /* 设备与云端连接长时间断开，请在此处添加实现 */
            break;
        case HILINK_M2M_LONG_OFFLINE_REBOOT:
            /* 设备与云端连接长时间断开后进行重启，请在此处添加实现 */
            break;
        case HILINK_UNINITIALIZED:
            /* HiLink线程未启动，请在此处添加实现 */
            break;
        case HILINK_LINK_UNDER_AUTO_CONFIG:
            /* 设备处于配网模式，请在此处添加实现 */
            break;
        case HILINK_LINK_CONFIG_TIMEOUT:
            /* 设备处于10分钟超时状态，请在此处添加实现 */
            break;
        case HILINK_LINK_CONNECTTING_WIFI:
            /* 设备正在连接路由器，请在此处添加实现 */
            break;
        case HILINK_LINK_CONNECTED_WIFI:
            /* 设备已经连上路由器，请在此处添加实现 */
            break;
        case HILINK_M2M_CONNECTTING_CLOUD:
            /* 设备正在连接云端，请在此处添加实现 */
            break;
        case HILINK_M2M_CLOUD_DISCONNECT:
            /* 设备与路由器的连接断开，请在此处添加实现 */
            break;
        case HILINK_DEVICE_REGISTERED:
            /* 设备被注册，请在此处添加实现 */
            break;
        case HILINK_DEVICE_UNREGISTER:
            /* 设备被解绑，请在此处添加实现 */
            break;
        case HILINK_REVOKE_FLAG_SET:
            /* 设备被复位标记置位，请在此处添加实现 */
            break;
        case HILINK_NEGO_REG_INFO_FAIL:
            /* 设备协商配网信息失败 */
            break;
        default:
            break;
    }

    return;
}

/*
 * 实现模组重启前的设备操作
 * flag为0表示HiLink SDK 线程看门狗触发模组重启; 为1表示APP删除设备触发模组重启
 * 返回0表示处理成功, 系统可以重启，使用硬重启; 返回1表示处理成功, 系统可以重启，使用软重启;
 * 返回负值表示处理失败, 系统不能重启
 * 注意，此函数由设备厂商实现；若APP删除设备触发模组重启时，设备操作完务必返回0，否则会导致删除设备异常
 */
int hilink_process_before_restart(int flag)
{
    /* HiLink SDK线程看门狗超时触发模组重启 */
    if (flag == HILINK_REBOOT_WATCHDOG) {
        /* 实现模组重启前的操作(如:保存系统状态等) */
        return -1;
    }

    /* APP删除设备触发模组重启 */
    if (flag == HILINK_REBOOT_DEVDELETE) {
        /* 实现模组重启前的操作(如:保存系统状态等) */
        return 0;
    }

    return -1;
}

/*
 * 获取设备故障码，并通知APP
 * status表示是否发送故障，0表示不发送，1表示发送；code表示故障码
 * 返回0表示成功，返回非0失败
 */
int get_faultDetection_state(int *status, int *code)
{
    /* 由设备厂商实现，将服务faultDetection属性当前值赋予出参 */
    return 0;
}

/*
 * 获取设备的子型号，长度固定两个字节
 * subProdId为保存子型号的缓冲区，len为缓冲区的长度
 * 如果产品定义有子型号，则填入两字节子型号，并以'\0'结束, 返回0
 * 没有定义子型号，则返回-1
 * 该接口需设备开发者实现
 */
int HILINK_GetSubProdId(char *subProdId, int len)
{
    return -1;
}


int HILINK_BT_GetDevSurfacePower(char *power)
{
    if (power == NULL) {
        return -1;
    }
    *power = 0xF8;
    return 0;
}

HILINK_BT_DevInfo *HILINK_BT_GetDevInfo(void)
{
    g_btDevInfo.manuName = (char *)g_devNameEn.manuName;
    g_btDevInfo.devName = (char *)g_devNameEn.devTypeName;
    g_btDevInfo.productId = (char *)dev_info.prodId;
    g_btDevInfo.mac = (char *)dev_info.mac;
    return &g_btDevInfo;
}

/* 
 * 写文件
 * key:文件名
 * value:需要写入的数据
 * len:需要写入的数据长度
 * sync:是否需要同步(默认为 1)
 * 返回值:0 成功 -1 失败
 */
int duet_flash_kv_set(const char *key, const void *value, int len, int sync)
{
	return 0;
}

/*读取文件
 *key:文件名
 *buffer:读取数据的缓存区
 *buffer_len:需要读取的数据长度
 *返回值:0 成功 -1 失败
 */
int duet_flash_kv_get(const char *key, void *buffer, int *buffer_len)
{
	return 0;
}

/*删除文件
 *key:文件名
 *返回值:0 成功 -1 失败
 */
int duet_flash_kv_del(const char *key)
{
    return 0;
}

int GetBtNetworkState(void)
{	
	return 0;	
}

int HILINK_ql_printf(const char *format, ...)
{
	return 0;		
}

int HILINK_BT_GetNetworkState()
{
	return 0;		
}
