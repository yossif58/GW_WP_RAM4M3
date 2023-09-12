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

#include <stdint.h>
#include <string.h>
#include <machine/endian.h>
#include "hal_config.h"
#include "boot_config.h"
#include "boot_platform.h"
#include "boot_fdl.h"
#include <sys/reent.h>
#include "hal_chip.h"
#include "boot_debuguart.h"
#include "boot_mem.h"
#include "boot_entry.h"
#include "cmsis_core.h"
#include "boot_secure.h"
#include "boot_spi_flash.h"
#include "boot_vfs.h"
#include "boot_mmu.h"
#include "boot_timer.h"
#include "boot_irq.h"
#include "boot_bsl_cmd.h"
#include "boot_pdl.h"
#include "hal_adi_bus.h"
#include "hal_spi_flash.h"
#include "flash_block_device.h"
#include "fupdate.h"
#include "fs_mount.h"
#include "osi_log.h"
#include "osi_api.h"
#include "drv_names.h"
#include "image_header.h"
#include "quec_proj_config.h"
#include "quec_boot_log.h"
#include "quec_boot_pff.h"


#ifdef CONFIG_QUEC_PROJECT_FEATURE_FOTA
#include "quec_boot_fota.h"
#include "quec_boot_pin_cfg.h"
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_NAND_FLASH
#include "quec_boot_nand.h"
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_URC
#include "quec_boot_uart.h"
#endif

#if (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_SDMMC) || (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_EMMC)
#include "quec_boot_sdmmc.h"
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
#include "quec_spi6_ext_norflash_cfg.h"
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_SPI4_EXTNSFFS
#include "fs_mount_spiflash.h"
#include "ql_boot_spi4_exnsffs_cfg.h"
#endif
#define PMU_BOOT_MODE_REG (hwp_rda2720mGlobal->por_rst_monitor)

static struct _reent gZeroedReent;
struct _reent *_impure_ptr = &gZeroedReent;

#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
//是否在boot中跳过初始化Spi6ExtNor，为了兼容不使用Spi6ExtNor，用作其他功能的用户
extern bool quec_skip_spi6_ext_nor_flash_init; 
#endif

#if (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_SDMMC) || (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_EMMC)
extern ql_boot_sdmmc_cfg_t ql_boot_sdmmc_cfg;			//sdmmc配置
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE
extern const image_header_t *quec_header;
extern uint32_t quec_boot_start_param;
extern void quec_boot_jump_kernel(uint32_t param,const image_header_t *header);
#endif

static void prvFirmwareUpdateProgress(int block_count, int block)
{
    OSI_LOGI(0, "FUPDATE block: %d/%d", block, block_count);

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_URC
    quec_boot_fota_urc_output(block_count, block);
#endif
}

#ifdef CONFIG_QUEC_PROJECT_FEATURE_FOTA
extern quec_boot_fs_type_e quec_boot_fs_type;
extern ql_boot_fat_mount_pt_e quec_partition_num;

static void quec_boot_ext_flash_init()
{
    switch(quec_boot_fs_type)
    {
        case QUEC_BOOT_SFFS_EXT:
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
        quec_boot_spi6_ext_norflash_init();
#endif
        break;
        
        case QUEC_BOOT_FAT_SDMMC:
#if (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_SDMMC)
	ql_boot_sdmmc_cfg.power_mv = POWER_LEVEL_3200MV;	//default sdcard voltage is 3.2 V
	ql_boot_sdmmc_cfg.sdmmc_clk = QL_BOOT_SDMMC_CLK_FREQ_SDHC; 	//400000 <= freq <= 50000000,
														//default sdcard clk is 50MHz(SDHC) or 25MHz(NOT SDHC).
#if (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_FAT_RDONLY)
		quec_boot_sdmmc_init(quec_partition_num);
#elif (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_FAT_RW)
		quec_boot_sdmmc_init_ex(quec_partition_num);
#endif /* CONFIG_QUEC_PROJECT_FEATURE_BOOT_FAT */

#elif (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_EMMC)
	ql_boot_sdmmc_cfg.power_mv = POWER_LEVEL_1800MV;	//default emmc voltage is 1.8 V
	ql_boot_sdmmc_cfg.sdmmc_clk = QL_BOOT_SDMMC_CLK_FREQ_SD;	//default emmc clk is 25MHz
#if (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_FAT_RDONLY)
		quec_boot_emmc_init(quec_partition_num);
#elif (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_FAT_RW)
		quec_boot_emmc_init_ex(quec_partition_num);
#endif /* CONFIG_QUEC_PROJECT_FEATURE_BOOT_FAT */

#endif /* CONFIG_QUEC_PROJECT_FEATURE_BOOT_SDMMC */
        break;
        
        case QUEC_BOOT_FAT_EXNAND_FLASH:
#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_NAND_FLASH
        quec_boot_nand_init(QL_BOOT_SPI_PORT_1);
#endif
        break;

        case QUEC_BOOT_SFFS_SPI4EXTNOR:
#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_SPI4_EXTNSFFS
        ql_boot_spi4_nor_flash_init();
        fsMountGeneralSpiFlash(false);
#endif
        break;

        default:
        break;
    }
}
#endif

static void prvFirmwareUpdate(void)
{
#ifdef CONFIG_QUEC_PROJECT_FEATURE_SPI6_EXT_NOR
    quec_skip_spi6_ext_nor_flash_init = true;//跳过Spi6ExtNor初始化和挂载
#endif

    fsMountSetScenario(FS_SCENRARIO_BOOTLOADER);
    if (!fsMountAll())
        return;

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_SPI4_NORFLASH
		void quec_boot_spi4_nor_flash_example();
		quec_boot_spi4_nor_flash_example();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_FOTA
    quec_boot_fota_init();
    quec_boot_ext_flash_init();
#endif

#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_URC    
    quec_boot_urc_init();
#endif 
    //urc fota start

    //Check whether the FOTA package is correct in boot.This code cannot be masked.
    quec_boot_fota_pack_verify();

    fupdateResult_t result = fupdateRun(prvFirmwareUpdateProgress);

    OSI_LOGI(0, "FUPDATE: %d", result);
    //urc fota end err_code  
    if (result == FUPDATE_RESULT_FAILED)
        bootReset(BOOT_RESET_NORMAL);
}

#ifdef CONFIG_BOOT_TIMER_IRQ_ENABLE
static void prvTimerCallback(void)
{
    // usually, just feed external watchdog here
}
#endif

static const image_header_t *prvAppUimageHeader(void)
{
    return (const image_header_t *)CONFIG_APP_FLASH_ADDRESS;
}

static void prvSetFlashWriteProhibit(void)
{
    // ATTENTION: This will set write prohibit for bootloader
    //
    // If there are requiement (though not reasonable) to change bootloader
    // the followings should be changed. And when there are more regions are
    // known never changed, more regions can be added.

    bootSpiFlash_t *flash = bootSpiFlashOpen(HAL_FLASH_DEVICE_NAME(CONFIG_BOOT_FLASH_ADDRESS));
    bootSpiFlashSetRangeWriteProhibit(flash, HAL_FLASH_OFFSET(CONFIG_BOOT_FLASH_ADDRESS),
                                      HAL_FLASH_OFFSET(CONFIG_BOOT_FLASH_ADDRESS) + CONFIG_BOOT_FLASH_SIZE);
}

static bool prvCheckNormalPowerUp()
{
#ifndef CONFIG_CHARGER_POWERUP
    return (bootPowerOnCause() != OSI_BOOTCAUSE_CHARGE);
#endif
    return true;
}

#ifdef CONFIG_BOOT_SMPL_ENABLE
static bool prvSmplEnable(void)
{
    uint32_t smpl_mode;

#define TIMER_THRESHOLD_MASK 0x7
#define SMPL_ENBLAE 0x1935

    smpl_mode = ((CONFIG_BOOT_SMPL_THRESHOLD & TIMER_THRESHOLD_MASK) << 13) | SMPL_ENBLAE;

    REG_RDA2720M_GLOBAL_SMPL_CTRL0_T smpl_ctrl0 = {};
    smpl_ctrl0.b.smpl_mode = smpl_mode;
    halAdiBusWrite(&hwp_rda2720mGlobal->smpl_ctrl0, smpl_ctrl0.v);

    REG_RDA2720M_GLOBAL_SMPL_CTRL1_T smpl_ctrl1 = {halAdiBusRead(&hwp_rda2720mGlobal->smpl_ctrl1)};

    REG_ADI_WAIT_FIELD_NEZ(smpl_ctrl1, hwp_rda2720mGlobal->smpl_ctrl1, smpl_mode_wr_ack_flag);

    return smpl_ctrl1.b.smpl_en == 1 ? true : false;
}
#endif

#ifdef CONFIG_BOOT_UART1_PDL_ENABLE
static void prvUart1Download(void)
{
    bool download = false;
    uint32_t boot_mode_val = halAdiBusRead(&PMU_BOOT_MODE_REG);
    if (boot_mode_val == OSI_SHUTDOWN_BL_DOWNLOAD)
    {
        download = true;
        halAdiBusWrite(&PMU_BOOT_MODE_REG, 0);
    }

    uint32_t rom_flags = hwp_idle->idle_res5;
    if (rom_flags == 0xdeadcafe)
        download = true;

    if (!download)
        return;

    OSI_LOGI(0, "UART1 download, magic/0x%04x/0x%08x", boot_mode_val, rom_flags);
    fdlChannel_t *ch = fdlOpenUart(DRV_NAME_UART1, 921600, true);
    if (ch == NULL)
    {
        OSI_LOGE(0, "Failed to open %4c", DRV_NAME_UART1);
        osiPanic();
    }

    if (!pdlDnldUimage(ch, BOOT_DOWNLOAD_UART1, CONFIG_FDL1_IMAGE_START, CONFIG_FDL1_IMAGE_SIZE, 10000))
    {
        OSI_LOGE(0, "%4c PDL download failed", DRV_NAME_UART1);
        osiPanic();
    }

    OSI_LOGI(0, "%4c PDL download timeout", DRV_NAME_UART1);
}
#endif


/*********************************************************************************************************************************
* example: how to set pin function as gpio and how to set or get gpio level (take GPIO17 as an example)
* Attention:
    1. the register name can refer to quec_pin_cfg_ECXXXU.c;
    2. gpio corresponding function can refer to quec_pin_cfg_ECXXXU.c;
    3. some pins need to open the voltage domain before use , V_PAD_1V8 voltage domain is on by default and does not need to be set
***********************************************************************************************************************************/
void quec_boot_gpio_example()
{
    //set gpio17 as gpio function
    hwp_iomux->pad_i2c_m1_sda_cfg_reg = 4;

    //Set gpio17 direction to output
    hwp_gpio1->gpio_oen_set_out = (1 << 17);

    //set gpio17 output high level
    hwp_gpio1->gpio_set_reg = (1 << 17);

    //set gpio17 output low level
    hwp_gpio1->gpio_clr_reg = (1 << 17);

    //get gpio level status, gpio_lvl=1 indicates high level, gpio_lvl=0 indicates low level
    uint32_t reg_value = hwp_gpio1->gpio_set_reg & (1 << 17);
    int gpio_lvl = (reg_value >> 17) & 0x1;
    OSI_LOGI(0, "gpio_lvl=%d", gpio_lvl);
}

/*********************************************************************************************************************************
* example: how to use spi4 nor flash in boot
* Attention:
    1. the SPI port currently used in boot,it can be configured in quec_boot_pin_cfg.h;
    
***********************************************************************************************************************************/
#ifdef CONFIG_QUEC_PROJECT_FEATURE_BOOT_SPI4_NORFLASH
#include "quec_boot_spi.h"
#include "ql_boot_spi4_nor_flash.h"

//Note:the SPI port currently used in boot,it can be configured in quec_boot_pin_cfg.h
extern uint8_t ql_boot_cur_spi4_port;
void quec_boot_spi4_nor_flash_example()
{
	//Initialize pin to SPI function
	quec_boot_spi_pin_init(ql_boot_cur_spi4_port);

	//Configure nor Flash initialization parameters
	ql_boot_spi_nor_config_s flash_config = {0};
	flash_config.port = ql_boot_cur_spi4_port;
	flash_config.spiclk = QL_BOOT_SPI_CLK_25MHZ;
	flash_config.input_sel = QL_BOOT_SPI_DI_1;
	flash_config.transmode = QL_BOOT_SPI_DIRECT_POLLING;//BOOT中不支持DMA模式
	flash_config.cs = QL_BOOT_SPI_CS0;
	ql_boot_spi_nor_init_ext(flash_config);

	OSI_LOGI(0, "ql_boot_spi4_nor_flash_init");
	
    unsigned int addr = 0x000008;
    unsigned int len = 0;
    unsigned char *data = NULL;
    int i = 0;

	if(ql_boot_spi_nor_erase_sector(ql_boot_cur_spi4_port, addr) != QL_BOOT_SPI_SUCCESS)
	{
		return;
	}
	
	len = 1024;
	data = bootExtRamCalloc(1,len+1);
	if (data == NULL)
	{
		return;
	}
	
	for (i = 0; i < len; i++)
	{
		data[i] = 'b';
	}
	QUEC_BOOT_LOG("write data:%s", data);
	if(ql_boot_spi_nor_write(ql_boot_cur_spi4_port, data, addr, len) == QL_BOOT_SPI_SUCCESS)
	{
		memset(data,0x00,len+1);
		ql_boot_spi_nor_read(ql_boot_cur_spi4_port, data, addr, len);
		QUEC_BOOT_LOG("read data:%s", data);
	}
	bootFree(data);
}
#endif

void bootStart(uint32_t param)
{
    OSI_CLEAR_SECTION(bss);

    bootSpiFlashOpen(DRV_NAME_SPI_FLASH); // ensure accessible
    halClockInit();
    halRamInit();

    extern unsigned __mmu_ttbl1_start[];
    extern unsigned __mmu_ttbl2_start[];
    bootMmuEnable((uint32_t *)__mmu_ttbl1_start, (uint32_t *)__mmu_ttbl2_start);

    __FPU_Enable();
    _REENT_INIT_PTR_ZEROED(_impure_ptr);

    halAdiBusInit();

#ifdef CONFIG_BOOT_SMPL_ENABLE
    if (!prvSmplEnable())
        OSI_LOGI(0, "SMPL enable fail");
#endif

    bootResetPinEnable();

    const image_header_t *header = prvAppUimageHeader();
#ifdef CONFIG_QUEC_PROJECT_FEATURE
    quec_header = header;
    quec_boot_start_param = param;
#endif
    if (bootIsFromPsmSleep())
    {
        bootJumpFunc_t entry = (bootJumpFunc_t)__ntohl(header->ih_ep);
        osiDCacheCleanInvalidateAll();
        bootJumpImageEntry(param, entry);
    }

    if (!prvCheckNormalPowerUp())
    {
        osiDelayUS(1000 * 10);
        bootPowerOff();
    }

    bool trace_enable = false;
#ifdef CONFIG_BOOT_LOG_ENABLED
    trace_enable = true;
#endif
    bootTraceInit(trace_enable);

    extern unsigned __sram_heap_start[];
    extern unsigned __sram_heap_end[];
    unsigned sram_heap_size = OSI_PTR_DIFF(__sram_heap_end, __sram_heap_start);
    bootHeapInit((uint32_t *)__sram_heap_start, sram_heap_size,
                 (uint32_t *)CONFIG_RAM_PHY_ADDRESS, CONFIG_RAM_SIZE);
    bootHeapDefaultExtRam();

#ifdef CONFIG_BOOT_TIMER_IRQ_ENABLE
    bootEnableInterrupt();
    bootEnableTimer(prvTimerCallback, CONFIG_BOOT_TIMER_PERIOD);
#endif

#ifdef CONFIG_BOOT_UART1_PDL_ENABLE
    prvUart1Download();
#endif

    //halPmuExtFlashPowerOn();//移到boot fota中
    prvSetFlashWriteProhibit();
    prvFirmwareUpdate();

/* get pwrkey status in boot */
#if 0
    REG_RDA2720M_GLOBAL_MODULE_EN0_T module_en0;
    REG_RDA2720M_GLOBAL_RTC_CLK_EN0_T rtc_clk_en0;

    halAdiBusBatchChange(
        // enable EIC
        &hwp_rda2720mGlobal->module_en0,
        REG_FIELD_MASKVAL1(module_en0, eic_en, 1),
        // enable EIC RTC clock
        &hwp_rda2720mGlobal->rtc_clk_en0,
        REG_FIELD_MASKVAL1(rtc_clk_en0, rtc_eic_en, 1),
        HAL_ADI_BUS_CHANGE_END);

    uint32_t mask = 0;
    mask = halAdiBusRead(&hwp_rda2720mEic->eicdmsk);
    mask = mask | (1<<1);
    halAdiBusWrite(&hwp_rda2720mEic->eicdmsk, mask);

    // if pwrkey status get error when reset, increase delay time
    osiDelayUS(5000);
    uint32_t data = halAdiBusRead(&hwp_rda2720mEic->eicdata);
    OSI_LOGI(0, "bootStart data = 0x%x", data); // bit1: 0 is press  1 is release
#endif

#ifdef CONFIG_BOOT_UART1_PDL_ENABLE
#if !((defined CONFIG_QL_OPEN_EXPORT_PKG) && (defined CONFIG_QUEC_PROJECT_FEATURE_BOOT_FAT_RW))
	//这块代码是用于通过uart下载非bootloader代码
    uint32_t sync_cnt = 0;
    uint32_t timeout = 50; // ms
    fdlChannel_t *ch = fdlOpenUart(CONFIG_FDL_DEFAULT_UART, CONFIG_FDL_UART_BAUD, true);
    if (ch == NULL)
    {
        OSI_LOGE(0, "BOOT fail, can't open uart fdl channel");
        osiPanic();
    }

    fdlEngine_t *fdl = fdlEngineCreate(ch, CONFIG_FDL_PACKET_MAX_LEN);
    if (fdl == NULL)
    {
        OSI_LOGE(0, "BOOT fail, can not create fdl engine");
        osiPanic();
    }
    osiElapsedTimer_t elapsed;
    osiElapsedTimerStart(&elapsed);
    for (;;)
    {
        uint8_t sync;
        if (fdlEngineReadRaw(fdl, &sync, 1))
        {
            if (sync == HDLC_FLAG)
            {
                sync_cnt++;
                timeout = 150; // ms
            }
            OSI_LOGI(0, "BOOT recv 0x%x , recv 0x7e cnt %d", sync, sync_cnt);
        }
        if (sync_cnt >= 3)
        {
            fdlEngineSendVersion(fdl);

            OSI_LOGI(0, "ENTER DOWNLOAD MODE ");
            osiDelayUS(50 * 1000);
            bootReset(BOOT_RESET_FORCE_DOWNLOAD);
            //or download fdl1 and run fdl1 TODO
            break;
        }

        if (osiElapsedTime(&elapsed) > timeout)
            break;
    } 
#endif
#endif
#ifndef CONFIG_QUEC_PROJECT_FEATURE
    if (__ntohl(header->ih_magic) == IH_MAGIC && bootSecureUimageSigCheck((void *)header))
    {
#ifdef CONFIG_BOOT_TIMER_IRQ_ENABLE
        bootDisableTimer();
        bootDisableInterrupt();
#endif
        bootJumpFunc_t entry = (bootJumpFunc_t)__ntohl(header->ih_ep);
        osiDCacheCleanInvalidateAll();
        bootJumpImageEntry(param, entry);
    }

    osiDebugEvent(0xdead0000);
#else
    quec_boot_jump_kernel(param,header);
#endif
    OSI_DEAD_LOOP;
}
