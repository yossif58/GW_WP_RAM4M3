/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include "osi_log.h"
#include "osi_sysnv.h"
#include "osi_trace.h"
#include "app_config.h"
#include "at_engine.h"
#include "fs_config.h"
#include "fs_mount.h"
#include "fs_mount_sdcard.h"
#include "ml.h"
#include "hal_shmem_region.h"
#include "hal_cp_ctrl.h"
#include "hal_iomux.h"
#include "net_config.h"
#include "hal_config.h"
#include "hal_chip.h"
#include "hal_blue_screen.h"
#include "hwregs.h"
#include "netmain.h"
#include "drv_ps_path.h"
#include "drv_pmic_intr.h"
#include "drv_charger.h"
#include "drv_rtc.h"
#include "drv_adc.h"
#include "drv_gpio.h"
#include "drv_usb.h"
#include "drv_axidma.h"
#include "drv_md_nvm.h"
#include "drv_md_ipc.h"
#include "drv_debug_port.h"
#include "drv_host_cmd.h"
#include "drv_spi_flash.h"
#include "drv_config.h"
#include "nvm.h"
#include "vfs.h"
#include "diag.h"
#include "diag_runmode.h"
#include "diag_auto_test.h"
#include "srv_trace.h"
#include "srv_rf_param.h"
#include "fupdate.h"
#include "srv_wdt.h"
#include "app_loader.h"
#include "mal_api.h"
#include "audio_device.h"
#include <stdlib.h>
#include "connectivity_config.h"
#ifdef CONFIG_TTS_SUPPORT
#include "tts_player.h"
#endif
#include "srv_simlock.h"
#include "srv_power_manager.h"
#include "srv_keypad.h"
#include "srv_usb.h"

#ifdef CONFIG_HEADSET_DETECT_SUPPORT
#include "srv_headset.h"
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
#include "ql_api_spi6_ext_nor_flash.h"
#endif

#include "ql_log.h"
#include "quec_init.h"
#include "quec_cust_patch.h"
#include "quec_common.h"
#include "quec_led_task.h"
#include "quec_internal_cfg.h"
#ifdef CONFIG_QUEC_PROJECT_FEATURE_URC
#include "quec_urc.h"
#endif
#include "quec_usbnet.h"
#include "quec_init.h"
#ifdef CONFIG_QUEC_PROJECT_FEATURE_GNSS    
#include "quec_gnss_handle.h"
#endif
#include "ql_api_dev.h"
#include "quec_atcmd_general.h"

#ifdef CONFIG_QUEC_PROJECT_FEATURE_FS_NAND_FLASH
#include "ql_nand_block_device.h"
#endif
#include "ql_power.h"

#ifdef CONFIG_QUEC_PROJECT_FEATURE_SDMMC 
#include "ql_sdmmc.h"
#endif

#ifdef __QUEC_OEM_VER_QDMHYT__
#include "quec_pwm.h"
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_NW
#include "ql_api_nw.h"
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_USB
#include "ql_usb.h"
#endif

#define QUEC_STARTUP_LOG(msg, ...)  custom_log("QSTART", msg, ##__VA_ARGS__)

#ifdef CONFIG_AT_BT_APP_SUPPORT
extern void bt_ap_init(void);
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_HEART_BEAT
#ifdef CONFIG_QUEC_PROJECT_FEATURE_HEART_BEAT_AT
extern int quec_heartbeat_param_init(void);
#endif
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_GPIO
extern void quec_pin_cfg_init(void);
extern void quec_gpio_interruput_init(void);
extern void quec_ext_flash_init(void);
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE   
extern bool quec_usb_setting_read();
extern void quec_at_config_init_from_nv(void);
#endif 

#ifdef CONFIG_CHARGER_POWERUP
unsigned char quec_charge_power_up=1;
#else 
unsigned char quec_charge_power_up=0;
#endif

// TODO:
extern const char gBuildRevision[];
extern void ipcInit(void);
extern int32_t ipc_at_init(void);
extern void CFW_RpcInit(void);
extern bool aworker_start();

static void _checkGpioCalibMode(void)
{
#ifdef CONFIG_BOARD_ENTER_CALIB_BY_GPIO
    drvGpioConfig_t cfg = {
        .mode = DRV_GPIO_INPUT,
        .intr_enabled = false,
        .intr_level = false,
        .rising = true,
        .falling = false,
        .debounce = false,
    };

    struct drvGpio *gpio = drvGpioOpen(CONFIG_BOARD_ENTER_CALIB_GPIO, &cfg, NULL, NULL);
    if (drvGpioRead(gpio))
        osiSetBootMode(OSI_BOOTMODE_CALIB);
#endif
}

static void prvSetFlashWriteProhibitByAddress(unsigned address, unsigned size)
{
    if (size == 0)
        return;

    drvSpiFlash_t *flash = drvSpiFlashOpen(HAL_FLASH_DEVICE_NAME(address));
    if (flash != NULL)
    {
        drvSpiFlashSetRangeWriteProhibit(flash, HAL_FLASH_OFFSET(address),
                                         HAL_FLASH_OFFSET(address) + size);
    }
}

static void prvSetFlashWriteProhibit(void)
{
    // ATTENTION: This will set write prohibit for bootloader and AP codes
    //
    // If there are requiement (though not reasonable) to change bootloader
    // or AP codes, the followings should be changed. And when there are
    // more regions are known never changed, more regions can be added.

    prvSetFlashWriteProhibitByAddress(CONFIG_BOOT_FLASH_ADDRESS, CONFIG_BOOT_FLASH_SIZE);
    prvSetFlashWriteProhibitByAddress(CONFIG_APP_FLASH_ADDRESS, CONFIG_APP_FLASH_SIZE);

    // Modem partition is designed to be read only also.
    prvSetFlashWriteProhibitByAddress(CONFIG_FS_MODEM_FLASH_ADDRESS, CONFIG_FS_MODEM_FLASH_SIZE);
}

#ifdef CONFIG_QUEC_PROJECT_FEATURE
unsigned quec_gSysnvTraceDevice = 0,quec_gSysnvFstraceMask = 0;;
drvHostCmdEngine_t *quec_dhost_device = NULL,*quec_diag_device = NULL,*quec_atuart_device = NULL;
bool fstrace_cplog = false, fstrace_moslog = false;
#endif

static void prvTraceInit(void)
{
    bool fstrace = false;
#ifdef CONFIG_FS_TRACE_ENABLE
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_gSysnvFstraceMask = gSysnvFstraceMask;
    gSysnvFstraceMask = 0;
#endif
    if (gSysnvFstraceMask & SRV_FSTRACE_OPT_AP_ENABLE)
        fstrace = true;
    
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_gSysnvTraceDevice = gSysnvTraceDevice;
    if(gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_SD || gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_NAND
#if QUEC_FSTRACE_SUPPORT_NOR_FLASH
    || gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_EXTNOR || gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_EXT
#endif
    )
    {
        gSysnvTraceDevice = (quec_gSysnvFstraceMask & QUEC_FS_TRACE_PORT_BEFORE)?OSI_TRACE_DEVICE_USBSERIAL:OSI_TRACE_DEVICE_DEBUGHOST;
        //gSysnvTraceDevice = OSI_TRACE_DEVICE_DEBUGHOST;    //输出log到SD/NAND/NOR/EFS之前,默认log从debug口输出
        //gSysnvTraceDevice = OSI_TRACE_DEVICE_USBSERIAL;    //输出log到SD/NAND/NOR/EFS之前,默认log从usb口输出
        
#ifndef CONFIG_QL_OPEN_EXPORT_PKG /* Standard */
    /*标准版本 sd初始化位置比较靠前并且在调用srvFstraceInit(gSysnvFstraceMask)之后代码还调用了drvDebugPortFindByName(diag_device),
      所以在quec_fs_trace_init()中要跳过quec_drvDebugUserialPortDestroy;否则会dump*/
        if (quec_gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_SD)
        {
            gSysnvTraceDevice = quec_gSysnvTraceDevice;
            fstrace = true;
        }
#endif /*CONFIG_QL_OPEN_EXPORT_PKG*/
    }


#endif /*CONFIG_QUEC_PROJECT_FEATURE*/    
#endif /*CONFIG_FS_TRACE_ENABLE*/

#ifdef CONFIG_KERNEL_HOST_TRACE
    drvDebugPortMode_t dhost_mode = {
        .protocol = DRV_DEBUG_PROTOCOL_HOST,
        .trace_enable = !fstrace && (gSysnvTraceDevice == OSI_TRACE_DEVICE_DEBUGHOST),
        .cmd_enable = true,
        .bs_enable = true,
    };
    drvDebugPort_t *dhost_port = drvDhostCreate(dhost_mode);
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_dhost_device = 
#endif
        drvHostCmdEngineCreate(dhost_port);

    drvDebugPortMode_t aplog_mode = {
        .protocol = DRV_DEBUG_PROTOCOL_HOST,
        .trace_enable = !fstrace && (gSysnvTraceDevice == OSI_TRACE_DEVICE_USBSERIAL),
        .cmd_enable = true,
        .bs_enable = true,
    };
    drvDebugPort_t *aplog_port = drvDebugUserialPortCreate(DRV_NAME_USRL_COM4, aplog_mode);
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_diag_device = 
#endif
        drvHostCmdEngineCreate(aplog_port);

    unsigned diag_device = diagDeviceName();
#ifdef CONFIG_DIAG_DEVICE_USRL_SUPPORT
    if (DRV_NAME_IS_USRL(diag_device))
    {
        drvDebugPortMode_t diag_mode = {
            .protocol = DRV_DEBUG_PROTOCOL_DIAG,
            .trace_enable = false,
            .cmd_enable = true,
            .bs_enable = true,
        };
        drvDebugUserialPortCreate(diag_device, diag_mode);
    }
#endif
    if (DRV_NAME_IS_UART(diag_device))
    {
        drvDebugPortMode_t diag_mode = {
            .protocol = DRV_DEBUG_PROTOCOL_DIAG,
            .trace_enable = false,
            .cmd_enable = true,
            .bs_enable = true,
        };
        drvDebugUartPortCreate(diag_device, diag_mode);
    }

#ifdef CONFIG_ATR_UART_BLUE_SCREEN_DEBUG
    drvDebugPortMode_t atuart_mode = {
        .protocol = DRV_DEBUG_PROTOCOL_HOST,
        .trace_enable = false,
        .cmd_enable = true,
        .bs_enable = true,
        .bs_only = true,
    };
    drvDebugPort_t *atuart_port = drvDebugUartPortCreate(CONFIG_ATR_DEFAULT_UART, atuart_mode);
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_atuart_device =
#endif
        drvHostCmdEngineCreate(atuart_port);
#endif
#endif

#ifdef CONFIG_KERNEL_DIAG_TRACE
    drvDebugPortMode_t dhost_mode = {
        .protocol = DRV_DEBUG_PROTOCOL_HOST,
        .trace_enable = false,
        .cmd_enable = true,
        .bs_enable = true,
    };
    drvDebugPort_t *dhost_port = drvDhostCreate(dhost_mode);
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_dhost_device = 
#endif    
    drvHostCmdEngineCreate(dhost_port);

    unsigned diag_device = diagDeviceName();
#ifdef CONFIG_DIAG_DEVICE_USRL_SUPPORT
    if (DRV_NAME_IS_USRL(diag_device))
    {
        drvDebugPortMode_t diag_mode = {
            .protocol = DRV_DEBUG_PROTOCOL_DIAG,
            .trace_enable = !fstrace,
            .cmd_enable = true,
            .bs_enable = true,
        };
        drvDebugUserialPortCreate(diag_device, diag_mode);
    }
#endif
    if (DRV_NAME_IS_UART(diag_device))
    {
        drvDebugPortMode_t diag_mode = {
            .protocol = DRV_DEBUG_PROTOCOL_DIAG,
            .trace_enable = !fstrace,
            .cmd_enable = true,
            .bs_enable = true,
        };
        drvDebugUartPortCreate(diag_device, diag_mode);
    }
#endif
}

static void prvPowerOn(void *arg)
{
    ipcInit();
    drvNvmIpcInit();

    svcRfParamInit();

    ipc_at_init();
    drvPsPathInit();
//add by sum.li 2021/03/10
#ifndef CONFIG_QUEC_PROJECT_FEATURE   
#ifdef CONFIG_FS_MOUNT_SDCARD
    bool mount_sd = fsMountSdcard();
    OSI_LOGI(0, "application mount sd card %d", mount_sd);
#endif
#else
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SDMMC 
    extern bool quec_sdmmc_fatfs_init(void);
    quec_sdmmc_fatfs_init();
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_FS_MOUNT_SDCARD
    if (1 == ql_model_diff_ctx.sdmmc_enabled)
    {
        //bool mount_sd = fsMountSdcard();
        //OSI_LOGI(0, "application mount sd card %d", mount_sd);
        ql_errcode_sdmmc_e mount_sd = ql_sdmmc_mount();
        OSI_LOGI(0, "application mount sd card %x", mount_sd);
    }
#endif
#endif
#endif
#endif

#ifdef CONFIG_FS_TRACE_ENABLE
#ifndef CONFIG_QUEC_PROJECT_FEATURE
    if (gSysnvFstraceMask != 0)
        srvFstraceInit(gSysnvFstraceMask);
#else
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
     if (quec_gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_SD)
     {
         ql_fs_trace_init();
     }
#endif
#endif
#endif

#ifdef CONFIG_SRV_SIMLOCK_ENABLE
    srvSimlockInit();
#endif

    _checkGpioCalibMode();

    unsigned diag_device = diagDeviceName();
    drvDebugPort_t *diag_port = drvDebugPortFindByName(diag_device);
    if (osiGetBootMode() == OSI_BOOTMODE_NORMAL)
    {
        diagRunMode_t run_mode = diagRunModeCheck(diag_port);
        if (run_mode == DIAG_RM_CALIB)
            osiSetBootMode(OSI_BOOTMODE_CALIB);
        else if (run_mode == DIAG_RM_CALIB_POST)
            osiSetBootMode(OSI_BOOTMODE_CALIB_POST);
        else if (run_mode == DIAG_RM_BBAT)
            osiSetBootMode(OSI_BOOTMODE_BBAT);
    }

    diagInit(diag_port);

    if (osiGetBootMode() == OSI_BOOTMODE_CALIB)
    {
        OSI_LOGI(0, "application enter calibration mode");
        drvChargeDisable();
        fsRemountFactory(0); // read-write
    }
    else if (osiGetBootMode() == OSI_BOOTMODE_CALIB_POST)
    {
        OSI_LOGI(0, "application enter calibration post mode");
        fsRemountFactory(0); // read-write
        diagCalibPostInit();
    }
    else if (osiGetBootMode() == OSI_BOOTMODE_BBAT)
    {
        OSI_LOGI(0, "application enter BBAT mode");
        fsRemountFactory(0); // read-write
#ifndef CONFIG_QUEC_PROJECT_FEATURE        
        diagAutoTestInit();
#endif
    }
    else
    {       
#ifdef CONFIG_USB_SUPPORT
        bool usbok = drvUsbSetWorkMode(gSysnvUsbWorkMode);
        OSI_LOGI(0, "application start usb mode %d/%d/%d", gSysnvUsbWorkMode, gSysnvUsbDetMode, usbok);
        if (usbok)
        {
            drvUsbEnable(CONFIG_USB_DETECT_DEBOUNCE_TIME);
        }
#endif
    }
#ifndef CONFIG_QUEC_PROJECT_FEATURE
    bool fstrace_cplog = false, fstrace_moslog = false;
#endif
#ifdef CONFIG_FS_TRACE_ENABLE
    if (gSysnvFstraceMask & (SRV_FSTRACE_OPT_CP_ENABLE | SRV_FSTRACE_OPT_ZSP_ENABLE))
        fstrace_cplog = true;
    if (gSysnvFstraceMask & SRV_FSTRACE_OPT_MOS_ENABLE)
        fstrace_moslog = true;
#endif

    if (!fstrace_moslog)
    {
        drvDebugPortMode_t moslog_mode = {
            .protocol = DRV_DEBUG_PROTOCOL_MOS_LOG,
            .bs_enable = true,
        };
        drvDebugUserialPortCreate(DRV_NAME_USRL_COM2, moslog_mode);
    }

    if (!fstrace_cplog)
    {
        drvDebugPortMode_t modemlog_mode = {
            .protocol = DRV_DEBUG_PROTOCOL_MODEM_LOG,
            .bs_enable = true,
        };
        drvDebugUserialPortCreate(DRV_NAME_USRL_COM3, modemlog_mode);
    }

#ifdef CONFIG_AT_BT_APP_SUPPORT
    if (!srvBtTraceInit())
        OSI_LOGE(0, "init bt log fail.");
#endif

    CFW_RpcInit();
    malInit();

#ifdef CONFIG_APP_START_ATR
    // asynchrous worker, start before at task
#ifndef CONFIG_QUEC_PROJECT_FEATURE    
    aworker_start();
#endif
    atEngineStart();
#endif

    // zsp_uart & uart_3 are both for cp
    halIomuxSetFunction(HAL_IOMUX_FUN_ZSP_UART_TXD);
    halIomuxSetFunction(HAL_IOMUX_FUN_UART_3_TXD);

#ifndef CONFIG_QL_OPEN_EXPORT_PKG /* Standard */
#ifndef CONFIG_QUEC_PROJECT_FEATURE_QDSIM 
    /* dual sim single standby readnv */
extern QuecOSStatus quec_dual_sim_single_standby_readnv(void);
    quec_dual_sim_single_standby_readnv();
#endif
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_GPIO
#ifdef __QUEC_OEM_VER_LGR__
    extern bool quec_gpio_init_cfg(void);
    quec_gpio_init_cfg();
#endif
    OSI_LOGI(0, "QUEC GPIO SET");
    quec_pin_cfg_init();
    
#ifdef QUEC_AT_SIM_APREADYCTRL
extern void quec_sim_apreadypin_read_flash(void);
    quec_sim_apreadypin_read_flash();
#endif

#ifndef CONFIG_QL_OPEN_EXPORT_PKG /* Standard */
    /* dual sim single standby setting */
extern void quec_dual_sim_single_standby_func_set(void);
    quec_dual_sim_single_standby_func_set();

extern void quec_sleep_readnv_set_sleepind(void);
    quec_sleep_readnv_set_sleepind();
    quec_gpio_interruput_init();//for std only, open version init it at app img
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
    extern bool ql_get_spi6_ext_nor_status(void);
    if(ql_get_spi6_ext_nor_status()) //if ext nor flash is exist
    {
        OSI_LOGI(0, "EXT FLASH is exist");
        ql_spi6_ext_nor_flash_init();//for std only, open version init it at app img
    }  
#endif
#endif
#endif

#ifdef CONFIG_FS_TRACE_ENABLE
#ifndef CONFIG_QL_OPEN_EXPORT_PKG /* Standard */
#if QUEC_FSTRACE_SUPPORT_NOR_FLASH
        if (quec_gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_EXT)
        {
            ql_fs_trace_init();
        }
#endif
#endif
#endif

    //cp log device pin init
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    extern void quec_cp_device_read();
    quec_cp_device_read();
#endif

    if (!halCpLoad())
        osiPanic();

#ifndef CONFIG_QUEC_PROJECT_FEATURE_AUDIO
    audevInit();
#else
	audevInit(OSI_PRIORITY_ABOVE_NORMAL);
#endif

#ifdef CONFIG_TTS_SUPPORT
    ttsPlayerInit();
#endif

#ifdef CONFIG_NET_TCPIP_SUPPORT
    net_init();
#ifdef CONFIG_NET_NAT_SUPPORT
    OSI_LOGE(0, "init nat to %d", gSysnvNATCfg);
    extern void set_nat_enable(uint32_t natCfg);
    set_nat_enable(gSysnvNATCfg);
    extern void netif_setup_lwip_lanOnly();
    netif_setup_lwip_lanOnly();
#endif
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE

#if defined(CONFIG_APPIMG_LOAD_FLASH) && defined(CONFIG_APPIMG_FLASH_ADDRESS)
    const void *flash_img_address = (const void *)CONFIG_APPIMG_FLASH_ADDRESS;
    if (appImageFromMem(flash_img_address, &gAppImgFlash))
    {
        OSI_LOGI(0, "Find app image at 0x%x", flash_img_address);
        gAppImgFlash.enter(NULL);
    }
#endif

#if defined(CONFIG_APPIMG_LOAD_FILE) && defined(CONFIG_APPIMG_LOAD_FILE_NAME)
    if (appImageFromFile(CONFIG_APPIMG_LOAD_FILE_NAME, &gAppImgFile))
    {
        OSI_LOGI(0, "Find file image at " CONFIG_APPIMG_LOAD_FILE_NAME);
        gAppImgFile.enter(NULL);
    }
#endif

#endif

#ifdef CONFIG_AT_BT_APP_SUPPORT
    bt_ap_init();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE
	quec_net_app_startup();
#endif

    // HACK: Now CP will change hwp_debugUart->irq_mask. After CP is changed,
    // the followings should be removed.
    osiThreadSleep(1000);
    drvDhostRestoreConfig();
#ifdef CONFIG_HEADSET_DETECT_SUPPORT
    srvHeadsetInit();
#endif
}

#ifdef CONFIG_QUEC_PROJECT_FEATURE_QDBGCFG_AT
#ifdef CONFIG_SYS_WDT_ENABLE
extern bool wdt_log_print;
static void sys_wdt_feed_dog(void)
{	
	if(wdt_log_print){
		OSI_LOGI(0, "sys feed wdt");
	}
	halSysWdtStart();
}
#endif
#endif

void osiAppStart(void)
{
#ifdef CONFIG_FS_TRACE_ENABLE
    srv_fsTraceTimeStamp();
#endif
    OSI_LOGXI(OSI_LOGPAR_S, 0, "application start (%s)", gBuildRevision);

#ifdef CONFIG_SYS_WDT_ENABLE
    // Feed sys_wdt in low priority work queue, it is always enabled.
    halSysWdtStart();
#ifndef CONFIG_QUEC_PROJECT_FEATURE_QDBGCFG_AT
    osiWork_t *sys_wdt_work = osiWorkCreate((osiCallback_t)halSysWdtStart, NULL, NULL);
    osiTimer_t *sys_wdt_timer = osiTimerCreateWork(sys_wdt_work, osiSysWorkQueueLowPriority());
    osiTimerStartPeriodicRelaxed(sys_wdt_timer, CONFIG_SYS_WDT_FEED_INTERVAL, OSI_DELAY_MAX);
#else
	extern osiTimer_t *ql_wdt_timer;
	osiWork_t *sys_wdt_work = osiWorkCreate((osiCallback_t)sys_wdt_feed_dog, NULL, NULL);
	ql_wdt_timer = osiTimerCreateWork(sys_wdt_work, osiSysWorkQueueLowPriority());
	osiTimerStartPeriodicRelaxed(ql_wdt_timer, CONFIG_SYS_WDT_FEED_INTERVAL, OSI_DELAY_MAX);
#endif
#endif

#ifdef CONFIG_WDT_ENABLE
    srvWdtInit(CONFIG_APP_WDT_MAX_INTERVAL, CONFIG_APP_WDT_FEED_INTERVAL);
#ifndef CONFIG_QUEC_PROJECT_FEATURE
	srvWdtStart();
#endif
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE
	extern int quec_api_task_init();
	quec_api_task_init();
#if (defined CONFIG_QUEC_PROJECT_FEATURE_LCD) || (defined CONFIG_QUEC_PROJECT_FEATURE_MIPI_LCD)
    extern int quec_lcd_mutex_create();
    quec_lcd_mutex_create();
#endif
#endif

    osiInvokeGlobalCtors();
    prvSetFlashWriteProhibit();

    mlInit();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
extern bool quec_skip_spi6_ext_nor_flash_init;
#ifdef CONFIG_QL_OPEN_EXPORT_PKG
    quec_skip_spi6_ext_nor_flash_init = true; //open本版 内核跳过挂载spi6_ext_nor_flash
#else
    quec_skip_spi6_ext_nor_flash_init = false;
#endif
#endif
    if (!fsMountAll())
        osiPanic();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
#ifdef CONFIG_QL_OPEN_EXPORT_PKG
        quec_skip_spi6_ext_nor_flash_init = false;
#endif
#endif

    osiBuffer_t bscore_buf = halGetBscoreBuf();
    if (bscore_buf.size > 0)
    {
        OSI_LOGI(0, "restart from blue screen");

        // When needed, the buffer can be copied to another place, and
        // send to server after network is ready.
    }
    halClearBscoreBuf(); // clear it no matter it is valid or not

    // besure ap nvm directory exist
    vfs_mkdir(CONFIG_FS_AP_NVM_DIR, 0);
#ifdef CONFIG_QUEC_PROJECT_FEATURE
	quec_fs_dir_check();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_AUDIO
	extern bool ql_aud_nvm_init(void);
	ql_aud_nvm_init();	
#endif
    osiSysnvLoad();
    drvAxidmaInit();
    prvTraceInit();
    nvmInit();
    nvmMigration();

#ifdef CONFIG_QUEC_PROJECT_FEATURE
	bool wdtrst = quec_wdt_cfg_read();
	if(ql_dev_cfg_wdt(wdtrst) != QL_DEV_SUCCESS)
	{
		OSI_LOGI(0, "cfg wdt failed");
	}
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE
    fupdateStatus_t fup_status = fupdateGetStatus();
    if (fup_status == FUPDATE_STATUS_FINISHED)
    {
        char *old_version = NULL;
        char *new_version = NULL;
        if (fupdateGetVersion(&old_version, &new_version))
        {
            OSI_LOGXI(OSI_LOGPAR_SS, 0, "FUPDATE: %s -> %s",
                      old_version, new_version);
            free(old_version);
            free(new_version);
        }
        // fupdateInvalidate(true);
    }
#endif
    if (!halShareMemRegionLoad())
        osiPanic();

    osiPsmRestore();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_PSM
    extern void quec_psm_data_init(void);
    quec_psm_data_init();
#endif

#ifndef CONFIG_QUEC_PROJECT_FEATURE_GPIO
    drvGpioInit();
#else
    // if set 1 in one bit, corresponding GPIO will be set low_level
    // MSB is GPIO31 <-- LSB is GPIO0
    // don't select pinmux_8910_v2.json's [init_fun] corresponding pin
    quec_drvGpioInit(0xffffffff);
#endif
    drvPmicIntrInit();
    drvRtcInit();
    drvAdcInit();
    drvChargerInit();
    srvKpdInit();

#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_usb_setting_read();
    quec_at_config_init_from_nv();
#endif

    if (!srvPmInit(prvPowerOn, NULL))
        osiPanic();

    srvUsbInit();
    srvPmRun();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_PWK
    extern void quec_power_key_status_refresh();
    quec_power_key_status_refresh();
#endif


    // wait a while for PM source created
    osiThreadSleep(10);
    osiPmStart();
	
#ifdef CONFIG_QUEC_PROJECT_FEATURE
	quec_startup(4096);
#endif
}

#ifdef CONFIG_QUEC_PROJECT_FEATURE

#ifdef CONFIG_QUEC_PROJECT_FEATURE_SOCKET
extern void ql_socket_engine_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TCPIP
extern void ql_tcpip_app_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_PING
extern void ql_ping_app_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_NTP
extern void ql_ntp_service_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_LBS
extern void ql_lbs_service_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_FIREWALL
extern void quec_firewall_handler_main(void);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_MMS
extern void ql_mms_service_start();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_MQTT
extern void ql_mqtt_client_start();
#endif

#if defined(__QUEC_OEM_VER_CUC__) && defined(__QUEC_OEM_VER_OPC__)
extern void quec_cert_info_init(void);
#endif

#ifdef __QUEC_OEM_VER_OPC__
#ifdef CONFIG_QUEC_PROJECT_FEATURE_CTSREG
extern void ql_ctsreg_service_start();
#endif
#endif

#ifdef __QUEC_OEM_VER_CUC__
#ifdef CONFIG_QUEC_PROJECT_FEATURE_UNIOT_IDENTITY
extern void quec_uniot_identity_server_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_UNIOT_AT
extern void quec_unify_server_start();
#endif
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_HTTP
extern void ql_http_service_start();
#endif
#if defined(CONFIG_QUEC_PROJECT_FEATURE_SSL) && defined(CONFIG_QUEC_PROJECT_FEATURE_TCPIP)
extern void quec_ssl_ctx_int();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_FOTA
extern void quec_get_fota_result();
#endif/*CONFIG_QUEC_PROJECT_FEATURE_FOTA*/

extern void quec_data_engine_start();
extern void quec_at_config_action(void);

extern void quec_msg_process_init(void);

extern QuecOSStatus quec_sleep_init(void);

#ifdef CONFIG_QUEC_PROJECT_FEATURE_NW
extern void ql_nw_init();
#endif
extern void ql_datacall_init(void);

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BT
extern void ql_bt_init();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_BT_A2DP_AVRCP
extern void ql_bt_audio_init();
#endif
#endif

#ifdef CONFIG_QL_OPEN_EXPORT_PKG
extern void quec_app_start(void);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_WIFISCAN
extern void ql_wifiscan_task_init(void);
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_FILE
extern int ql_open_file_init(void);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_UART
extern void ql_uart_init(void);
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_AUDIO
extern void ql_audio_init(void);
#endif

#ifdef CONFIG_QUEC_PROJECT_NOTIFY_EVENT
extern QuecOSStatus quec_event_notify_init(void);
#endif

#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TTS_AT
extern void quec_tts_task_init(void);
#endif
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_TTS
	extern int ql_init_tts_license(void);
    extern bool quec_tts_resource_merge();
#endif

#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SDDET
extern void quec_sd_det_init(void);
#endif
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_AUDIO_AT
extern int  quec_atcmd_audio_task_init(void);
#endif
#if defined(CONFIG_QUEC_PROJECT_FEATURE_QTHSDK_AT) && defined(__QUEC_OEM_VER_QTH__)
extern void Ql_iotInit(void);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SMTP_AT
extern void at_smtp_client_init(void);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_ALIIOTSMARTCARD
extern void ql_aliot_smartcard_app_start(void);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TP
extern void quec_tp_init();
#endif
#if defined(CONFIG_QUEC_PROJECT_FEATURE_ALIPAY_IOT_SDK_AT) && defined(__QUEC_OEM_VER_ALIPAY__)
extern void quec_alipay_int(void);
#endif
int quec_init_done = 0;

#ifdef CONFIG_QL_OPEN_EXPORT_PKG
void quec_app_start(void)
{
#if defined(CONFIG_APPIMG_LOAD_FLASH) && defined(CONFIG_APPIMG_FLASH_ADDRESS)
    const void *flash_img_address = (const void *)CONFIG_APPIMG_FLASH_ADDRESS;
    if (appImageFromMem(flash_img_address, &gAppImgFlash))
    {
        OSI_LOGI(0, "Find app image at 0x%x", flash_img_address);
        gAppImgFlash.enter(NULL);
    }
#endif

#if defined(CONFIG_APPIMG_LOAD_FILE) && defined(CONFIG_APPIMG_LOAD_FILE_NAME)
    if (appImageFromFile(CONFIG_APPIMG_LOAD_FILE_NAME, &gAppImgFile))
    {
        OSI_LOGI(0, "Find file image at " CONFIG_APPIMG_LOAD_FILE_NAME);
        gAppImgFile.enter(NULL);
    }
#endif
}
#endif

void quec_net_app_init(void)
{
	QUEC_STARTUP_LOG("### quec net app init... ###");
#ifdef CONFIG_QUEC_PROJECT_FEATURE_LEDCFG
	quec_ledcfg_init();
#endif
    quec_at_config_action();
	quec_sleep_init();
    quec_init_set_status_bit(QUEC_INIT_SLEEP);

#ifdef CONFIG_QUEC_PROJECT_FEATURE_NW
	ql_nw_init();
#endif
	ql_datacall_init();

	quec_msg_process_init();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_USBNET
	quec_netdev_init();
#endif
	quec_data_engine_start();
	ql_socket_engine_start();
#if defined(CONFIG_QUEC_PROJECT_FEATURE_SSL) && defined(CONFIG_QUEC_PROJECT_FEATURE_TCPIP)
	quec_ssl_ctx_int();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TCPIP
	ql_tcpip_app_start();	
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_PING
	ql_ping_app_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_NTP
	ql_ntp_service_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_MQTT
	ql_mqtt_client_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_HTTP
	ql_http_service_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_MMS
	 ql_mms_service_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_LBS
	ql_lbs_service_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_FILE
	ql_open_file_init();
#endif
#ifdef CONFIG_QUEC_PROJECT_NOTIFY_EVENT
	quec_event_notify_init();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_FILE_AT
    extern QuecOSStatus quec_file_task_init(void);
    quec_file_task_init();
#endif
#ifdef CONFIG_QUEC_PROJECT_ATCMD_APP_SUPPORT
	{
		extern void quec_at_app_init(void);
		quec_at_app_init();
	}
#endif
#if defined(CONFIG_QUEC_PROJECT_FEATURE_QTHSDK_AT) && defined(__QUEC_OEM_VER_QTH__)
	Ql_iotInit();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_FIREWALL
	quec_firewall_handler_main();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SMTP_AT
	at_smtp_client_init();
#endif

#if defined(CONFIG_QUEC_PROJECT_FEATURE_ALIIOTSMARTCARD) && defined(__QUEC_OEM_VER_ALIPAY__)
	ql_aliot_smartcard_app_start();
#endif

	quec_init_set_status_bit(QUEC_INIT_PROTOCOL_STACK);

#ifdef __QUEC_OEM_VER_CUC__
#ifdef CONFIG_QUEC_PROJECT_FEATURE_UNIOT_IDENTITY
	quec_uniot_identity_server_start();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_UNIOT_AT
	quec_unify_server_start();
#endif
#endif

#if defined(__QUEC_OEM_VER_CUC__) && defined(__QUEC_OEM_VER_OPC__)
	quec_cert_info_init();
#endif

#ifdef __QUEC_OEM_VER_OPC__
#ifdef CONFIG_QUEC_PROJECT_FEATURE_CTSREG
	ql_ctsreg_service_start();
#endif
#endif

#if defined(CONFIG_QUEC_PROJECT_FEATURE_ALIPAY_IOT_SDK_AT) && defined(__QUEC_OEM_VER_ALIPAY__)
	quec_alipay_int();
#endif

#if defined(CONFIG_QUEC_PROJECT_FEATURE_MAYILIAN_AT) && defined(__QUEC_OEM_VER_ALILINK__)
    extern int bot_maas_init(void);
    bot_maas_init();
#endif

	QUEC_STARTUP_LOG("### quec net app init done... ###");
}

void quec_init_thread(void *arg)
{  

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BT
    ql_bt_init();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_BT_A2DP_AVRCP
    ql_bt_audio_init();
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TP
    quec_tp_init();
#endif

	quec_init_set_status_bit(QUEC_INIT_BT);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_WIFISCAN
    ql_wifiscan_task_init();
#endif
	
#ifdef CONFIG_QUEC_PROJECT_FEATURE_UART
    ql_uart_init();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_AUDIO
	ql_audio_init();
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TTS
	ql_init_tts_license();
#endif
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TTS_AT
	quec_tts_task_init();
#endif
#ifdef CONFIG_QUEC_PROJECT_ATCMD_APP_SUPPORT
#ifdef CONFIG_QUEC_PROJECT_FEATURE_AUDIO_AT
	quec_atcmd_audio_task_init();
#endif
#endif
#endif
	quec_init_set_status_bit(QUEC_INIT_AUDIO);
#endif

#ifndef CONFIG_QL_OPEN_EXPORT_PKG
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SDDET
    quec_sd_det_init();
#endif
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_FS_NAND_FLASH
extern void quec_nand_flash_fatfs_init(void);
    quec_nand_flash_fatfs_init();
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
    bool mount_nand_flash = ql_fs_mount_nand_flash();
    OSI_LOGI(0, "application mount nand flash %d", mount_nand_flash);    
#endif
#endif

#ifdef CONFIG_FS_TRACE_ENABLE
#ifndef CONFIG_QL_OPEN_EXPORT_PKG
    if (quec_gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_NAND
#if QUEC_FSTRACE_SUPPORT_NOR_FLASH
        || quec_gSysnvTraceDevice == OSI_TRACE_DEVICE_FILESYS_EXTNOR
#endif
    )
    {
        ql_fs_trace_init();
    }
#endif

#endif


#ifdef CONFIG_QUEC_PROJECT_FEATURE_URC
	quec_urc_rdy("RDY");
#endif
    quec_init_done = QUEC_INIT_DONE;
	
#ifdef CONFIG_QUEC_PROJECT_FEATURE_GNSS
	gpsautocheck();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_HEART_BEAT
#ifdef CONFIG_QUEC_PROJECT_FEATURE_HEART_BEAT_AT
	quec_heartbeat_param_init();
#endif
#endif

#ifndef CONFIG_QL_OPEN_EXPORT_PKG //same as __QUEC_VER_EXPORT_OPEN_CPU__
#ifdef CONFIG_QUEC_PROJECT_FEATURE_FOTA
    quec_get_fota_result();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_AUDIO
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TTS   
    quec_tts_resource_merge();
#endif
#endif

#ifdef __QUEC_OEM_VER_QDMHYT__
    //richard.liang:for HYT
    quec_pwm_virt_open();
#endif
#else

#ifdef CONFIG_QUEC_PROJECT_FEATURE_AUDIO
#ifdef CONFIG_QUEC_PROJECT_FEATURE_TTS   
    quec_tts_resource_merge();
#endif
#endif
    //jensen.fang: for open project, add your initialization here
    quec_app_start();
#endif

    uint8_t pwrup_reason = QL_PWRUP_UNKNOWN;
    ql_get_powerup_reason(&pwrup_reason);

	QUEC_STARTUP_LOG("### quec init done... pwrup=%d ###", pwrup_reason);
#ifdef CONFIG_QUEC_PROJECT_FEATURE_NW
    extern ql_nw_errcode_e ql_nw_get_cfg(void);
    ql_nw_get_cfg();
#endif
    ql_rtos_task_sleep_ms(10);
	osiThreadExit();
}


void quec_extend_at_port_init()
{
#ifdef CONFIG_QUEC_PROJECT_FEATURE_USB_MODEM_AT
    _atDeviceUserialInit_Ext(DRV_NAME_USRL_COM5);
#endif
#ifdef CONFIG_QUEC_PROJECT_FEATURE_USB_NMEA_AT
	QL_USB_ENMU_MODE_E mode = ql_usb_get_enum_mode();
	if(mode == QL_USB_ENUM_USBNET_COM)
	{
		_atDeviceUserialInit_Ext(DRV_NAME_USRL_COM6);
	} 
#endif
    //_atDeviceUserialInit_Ext(DRV_NAME_USRL_COM7);
}

#endif

