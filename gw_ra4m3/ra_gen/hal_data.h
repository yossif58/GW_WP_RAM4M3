/* generated HAL header file - do not edit */
#ifndef HAL_DATA_H_
#define HAL_DATA_H_
#include <stdint.h>
#include "bsp_api.h"
#include "common_data.h"
#include "r_iwdt.h"
#include "r_wdt_api.h"
#include "r_lpm.h"
#include "r_lpm_api.h"
#include "r_agt.h"
#include "r_timer_api.h"
#include "r_lvd.h"
#include "r_lvd_api.h"
#include "r_sci_uart.h"
#include "r_uart_api.h"
#include "r_rtc.h"
#include "r_rtc_api.h"
#include "r_flash_hp.h"
#include "r_flash_api.h"
FSP_HEADER
/** WDT on IWDT Instance. */
extern const wdt_instance_t g_wdt0;

/** Access the IWDT instance using these structures when calling API functions directly (::p_api is not used). */
extern iwdt_instance_ctrl_t g_wdt0_ctrl;
extern const wdt_cfg_t g_wdt0_cfg;

#ifndef NULL
void NULL(wdt_callback_args_t *p_args);
#endif
/** lpm Instance */
extern const lpm_instance_t Meter_g_lpm0;

/** Access the LPM instance using these structures when calling API functions directly (::p_api is not used). */
extern lpm_instance_ctrl_t Meter_g_lpm0_ctrl;
extern const lpm_cfg_t Meter_g_lpm0_cfg;
/** AGT Timer Instance */
extern const timer_instance_t Pulser_Timer0_Generic;

/** Access the AGT instance using these structures when calling API functions directly (::p_api is not used). */
extern agt_instance_ctrl_t Pulser_Timer0_Generic_ctrl;
extern const timer_cfg_t Pulser_Timer0_Generic_cfg;

#ifndef Pulser_timer_generic_callback
void Pulser_timer_generic_callback(timer_callback_args_t *p_args);
#endif
/** LVD Instance */
extern const lvd_instance_t Battery_g_lvd;

/** Access the LVD instance using these structures when calling API functions directly (::p_api is not used). */
extern lvd_instance_ctrl_t Battery_g_lvd_ctrl;
extern const lvd_cfg_t Battery_g_lvd_cfg;

#ifndef Battery_low_callback
void Battery_low_callback(lvd_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t LTE_Uart;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_uart_instance_ctrl_t LTE_Uart_ctrl;
extern const uart_cfg_t LTE_Uart_cfg;
extern const sci_uart_extended_cfg_t LTE_Uart_cfg_extend;

#ifndef LTE_uart_callback
void LTE_uart_callback(uart_callback_args_t *p_args);
#endif
/** UART on SCI Instance. */
extern const uart_instance_t Debug_Uart1;

/** Access the UART instance using these structures when calling API functions directly (::p_api is not used). */
extern sci_uart_instance_ctrl_t Debug_Uart1_ctrl;
extern const uart_cfg_t Debug_Uart1_cfg;
extern const sci_uart_extended_cfg_t Debug_Uart1_cfg_extend;

#ifndef Debug_uart_callback
void Debug_uart_callback(uart_callback_args_t *p_args);
#endif
/* RTC Instance. */
extern const rtc_instance_t Yos1_RTC;

/** Access the RTC instance using these structures when calling API functions directly (::p_api is not used). */
extern rtc_instance_ctrl_t Yos1_RTC_ctrl;
extern const rtc_cfg_t Yos1_RTC_cfg;

#ifndef yos1_rtc_callback
void yos1_rtc_callback(rtc_callback_args_t *p_args);
#endif
/* Flash on Flash HP Instance */
extern const flash_instance_t g_flash;

/** Access the Flash HP instance using these structures when calling API functions directly (::p_api is not used). */
extern flash_hp_instance_ctrl_t g_flash_ctrl;
extern const flash_cfg_t g_flash_cfg;

#ifndef bgo_callback
void bgo_callback(flash_callback_args_t *p_args);
#endif
void hal_entry(void);
void g_hal_init(void);
FSP_FOOTER
#endif /* HAL_DATA_H_ */
