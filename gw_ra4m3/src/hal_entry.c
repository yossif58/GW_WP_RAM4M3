/***********************************************************************************************************************
 * File Name    : hal_entry.c
 * Description  : Contains data structures and functions used in hal_entry.c.
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

#include "common_utils.h"
#include "main.h"
#include "flash_hp_ep.h"
FSP_CPP_HEADER
void R_BSP_WarmStart(bsp_warm_start_event_t event);
FSP_CPP_FOOTER
RepeaterConfigStruct RepeaterConfig;
rtc_time_t LastPulseTime;
bool IncPulseFlag = false;
bool toggle = false;
void ToggleLeds(void);
void SetLed(ioport_port_pin_t pin_num, bool STATE);
void SendData2Modem(uint8_t *DataIn, uint16_t Len);
void HourProcess(int hour,int year,int month,int day);
void modemProcess(void);
void RTC_Alarm_Set(void);
void ModemSequence(ModemModeSequence ModeSeq);
void EnterDefaultConfig(void);
void EnterDefaultTime(void);
void StartWMP_lpm(void);
uint32_t Start_ADC_VBAT(void);
uint32_t Vbat_val;
extern void Gsm_Handler(uint8_t * pDdataIn, uint32_t Len , EventType Event);
extern fsp_err_t Uarts_Initialize(void);
extern fsp_err_t Uarts_Close(void);
extern int8_t tmpbufout[];  // to be removed
extern fsp_err_t flash_hp_init(void);
extern uint8_t GsmCompleateStatus(void);
extern void ClrGsmCompleate(void);
extern void Gsm_Init(void);
extern bool FLashCheck(void);
extern uint32_t TimingDelay;
uint8_t Event_status[];
uint8_t modem_seq_state = 0;
uint32_t conttemp = 0;
bool ready_to_read = false;
//bool ready_to_send_daily=true;
//bool ready_to_update_daily=true;
const bsp_delay_units_t bsp_delay_units = BSP_DELAY_UNITS_MILLISECONDS;
const uint32_t delay = 5; // 10 sec y.f.
rtc_time_t CurrentTime;
bool RTC_Event = false;
bool write_internal_config_flag = false;
bool write_internal_logger_flag = false;
bool powerup_start = true;
#ifdef DEBUG_UART_MODE3
    char  str1[20];
#endif
void ClrLeds(void);

/* Temporary buffer to save data from receive buffer for further processing */
bool LtuGSM_Timeout = false;
uint32_t Ltutmpindexin;
uint8_t Ltutemputartin[MAX_UART_SIZE];
uint32_t Ltutmpindexout;
uint8_t Ltutemputartout[MAX_UART_SIZE];
#define LPM_CLOCK_LOCO                       2 // The local 32k
#define LPM_CLOCK_SUBCLOCK                   4 // The subclock oscillator.
#define LPM_CLOCK_HOCO                       0 // The high speed on chip oscillator.
uint32_t upgradestate;
long long  const DCU_id   =        210600402902;              // y.f. 25/12/22 add meter id in constant flash code
uint32_t firmware_date          BSP_PLACE_IN_SECTION(".wmp_manufacture_date*") = 0x010801;                // y.f. 27/12/22 add frimware manufacture date
uint32_t CurrentFirmwareVersion BSP_PLACE_IN_SECTION(".wmp_firmware_version*") = 102;

void R_BSP_WarmStart(bsp_warm_start_event_t event);
void readfromRTT(void);

/*******************************************************************************************************************//**
 * The RA Configuration tool generates main() and uses it to generate threads if an RTOS is used.  This function is
 * called by main() when no RTOS is used.
 **********************************************************************************************************************/
void hal_entry(void)
{
    fsp_err_t err = FSP_SUCCESS;
    fsp_pack_version_t version;
    uint32_t read_data = RESET_VALUE;
    lvd_status_t p_lvd_status;
       /* Setup MCU port settings after C runtime environment and system clocks are setup*/
    R_BSP_WarmStart(BSP_WARM_START_POST_C);
    R_BSP_PinAccessEnable();

    R_BSP_PinWrite ((bsp_io_port_pin_t) RADIO_RESET, BSP_IO_LEVEL_HIGH); // 12/02/23 enable EFR chip
    R_BSP_PinAccessDisable();
    R_LVD_Open (&Battery_g_lvd_ctrl, &Battery_g_lvd_cfg);
    R_AGT_Open (&Pulser_Timer0_Generic_ctrl, &Pulser_Timer0_Generic_cfg);
    R_AGT_Enable (&Pulser_Timer0_Generic_ctrl);
    Uarts_Initialize (); // Init both uarts DEBUG & LTE
     R_FLASH_HP_Open(&g_flash_ctrl, &g_flash_cfg);
    __enable_irq();
    ResetUpgradeState();  // y.f 12/11/22
    __enable_irq();
    char strwmp[100];
 //    while (1)
    {
        R_BSP_SoftwareDelay (3000, bsp_delay_units); // delay 1 sec
         R_SCI_UART_Write (&Debug_Uart1_ctrl, strwmp, 60);

    }
    if (FLashCheck () == false)
    { //first time or flash corrupted
        EnterDefaultConfig ();
    }
#ifdef PROGRAME_BY_E2STUDIO
    RepeaterConfig.ExtendedAddress = DCU_id;

#else
    RepeaterConfig.ExtendedAddress = pDCU_id;
#endif
    sprintf (strwmp, "debugGW Debug Port version %d.%02d pOWERCOM GW %d\r\n", CurrentFirmwareVersion/100,CurrentFirmwareVersion%100,RepeaterConfig.ExtendedAddress);

    R_RTC_Open (&Yos1_RTC_ctrl, &Yos1_RTC_cfg);
    EnterDefaultTime ();
    R_RTC_CalendarTimeGet (&Yos1_RTC_ctrl, &CurrentTime);

  R_BSP_SoftwareDelay (2000, bsp_delay_units); // delay 1 sec
  RTC_Alarm_Set();
 // status = R_CGC_Open(&g_cgc0_ctrl, &g_cgc0_cfg);
  powerup_start = true;
  __enable_irq();
  R_AGT_Start (&Pulser_Timer0_Generic_ctrl);

  while (1)
  {
//           R_AGT_Start (&Pulser_Timer0_Generic_ctrl);
           R_RTC_CalendarTimeGet (&Yos1_RTC_ctrl, &CurrentTime);
           R_LVD_StatusGet (&Battery_g_lvd_ctrl, &p_lvd_status);
           if (p_lvd_status.current_state == 0)
           {
#ifdef DEBUG_UART_MODE
                R_SCI_UART_Write (&Debug_Uart1_ctrl, "debugGW Battery Low Detect\r\n", 28);
                R_BSP_SoftwareDelay (100, bsp_delay_units); // delay 1 sec temp to be removed
#endif
                Event_status[0] |= LOW_BATTERY_DETECT;
           }
           if ((powerup_start) || (  modem_seq_state == 0))
           {
               powerup_start = false;
               ModemSequence (MODEM_START_SEQUENCE);
   //             modemProcess(); // start connection to server when magnet detect
           }
        else if (GsmCompleateStatus()==0)
            {
               ClrGsmCompleate();
               ModemSequence (MODEM_STOP_SEQUENCE);
            }
           else
           {
               if (Ltutmpindexin)
                {
                    Gsm_Handler( Ltutemputartin, Ltutmpindexin , DATA_IN_EVENT);
        #ifdef DEBUG_UART_MODE3
        //            R_BSP_SoftwareDelay (1000, bsp_delay_units); // delay 1 sec                               {
         //           R_SCI_UART_Write (&Debug_Uart1_ctrl, Ltutemputartin, Ltutmpindexin);
        //            R_BSP_SoftwareDelay (1000, bsp_delay_units); // delay 1 sec                               {

        #endif
                    R_AGT_Disable(&Pulser_Timer0_Generic_ctrl);
                    Ltutmpindexin=0;
                    R_AGT_Enable(&Pulser_Timer0_Generic_ctrl);
                }
                else
                if (LtuGSM_Timeout)
                {
                    LtuGSM_Timeout = false;
                    Gsm_Handler( 0, 0 , TIME_OUT_EVENT);
                }
#ifdef DEBUG_UART_MODE3
                if (Ltutmpindexout)
                {
         //           R_SCI_UART_Write (&Debug_Uart1_ctrl, Ltutemputartout, Ltutmpindexout);
          //          R_BSP_SoftwareDelay (500, bsp_delay_units); // delay 1 sec                               {
                    R_AGT_Disable(&Pulser_Timer0_Generic_ctrl);
                    Ltutmpindexout=0;
                    R_AGT_Enable(&Pulser_Timer0_Generic_ctrl);

                }
#endif
                if (RTC_Event) // 8.8.22 y.f.
                 {
                    RTC_Event = false;
                    HourProcess (CurrentTime.tm_hour,CurrentTime.tm_year,CurrentTime.tm_mon,CurrentTime.tm_mday);
                }
                if  (write_internal_config_flag == true) // 14/6/22 temp use flag instead of direct write internal
                {
                    write_internal_config_flag = false;
                    WriteInternalFlashConfig(); // store the configuration
                }
           }
 // 23.1.23 temp removed            StartWMP_lpm();
  }
}
#if 0
void modemProcess(void)
{
    ModemSequence (MODEM_START_SEQUENCE);
    while (GsmCompleateStatus ())
    {
        if (Ltutmpindexin)
        {
            Gsm_Handler( Ltutemputartin, Ltutmpindexin , DATA_IN_EVENT);
#ifdef DEBUG_UART_MODE3
//            R_BSP_SoftwareDelay (1000, bsp_delay_units); // delay 1 sec                               {
 //           R_SCI_UART_Write (&Debug_Uart1_ctrl, Ltutemputartin, Ltutmpindexin);
//            R_BSP_SoftwareDelay (1000, bsp_delay_units); // delay 1 sec                               {

#endif
            R_AGT_Disable(&Pulser_Timer0_Generic_ctrl);
            Ltutmpindexin=0;
            R_AGT_Enable(&Pulser_Timer0_Generic_ctrl);
        }
        else
            if (LtuGSM_Timeout)
            {
                LtuGSM_Timeout = false;
                Gsm_Handler( 0, 0 , TIME_OUT_EVENT);
            }
#ifdef DEBUG_UART_MODE3
        if (Ltutmpindexout)
        {
 //           R_SCI_UART_Write (&Debug_Uart1_ctrl, Ltutemputartout, Ltutmpindexout);
  //          R_BSP_SoftwareDelay (500, bsp_delay_units); // delay 1 sec                               {
            R_AGT_Disable(&Pulser_Timer0_Generic_ctrl);
            Ltutmpindexout=0;
            R_AGT_Enable(&Pulser_Timer0_Generic_ctrl);

        }
#endif


    }
    ModemSequence (MODEM_STOP_SEQUENCE);

}
#endif
void ModemSequence(ModemModeSequence ModeSeq)
{
    R_BSP_PinAccessEnable ();
    /* Write to this pins */
    R_BSP_PinWrite ((bsp_io_port_pin_t) DTR, BSP_IO_LEVEL_HIGH);
    R_BSP_PinWrite ((bsp_io_port_pin_t) LTE_VCC_ENABLE, BSP_IO_LEVEL_HIGH);
    R_BSP_PinWrite ((bsp_io_port_pin_t) WRITE_DISABLE_LTE, BSP_IO_LEVEL_LOW);
    R_BSP_PinWrite ((bsp_io_port_pin_t) WAKE_UP_LTE, BSP_IO_LEVEL_HIGH);

    if (ModeSeq == MODEM_START_SEQUENCE)
    {
        ClrGsmCompleate ();
        R_BSP_PinAccessEnable ();
        /* Write to this pin */
#ifdef EC200N
        // Y.F. 15/12/22
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_HIGH);
              R_BSP_PinAccessDisable ();
              R_BSP_SoftwareDelay (400, bsp_delay_units); // delay 1 sec
              TimingDelay = 10;


#else
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
              R_BSP_PinAccessDisable ();
              TimingDelay = 50;

#endif

//        R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);

        R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_RESET_KEY_IN, BSP_IO_LEVEL_HIGH);

#if 0
      HAL_GPIO_WritePin(MODEM_POWER_KEY_PORT, MODEM_POWER_KEY_PIN, GPIO_PIN_RESET );
      HAL_GPIO_WritePin(MODEM_RESET_PORT, MODEM_RESET_PIN, GPIO_PIN_SET );
#endif

        modem_seq_state = 1;
    }

    else

    if (ModeSeq == MODEM_STOP_SEQUENCE)
    {

        R_BSP_PinAccessEnable ();
        /* Write to this pin */
        R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_HIGH);
        R_BSP_PinAccessDisable ();

#if 0
      HAL_GPIO_WritePin(MODEM_POWER_KEY_PORT, MODEM_POWER_KEY_PIN, GPIO_PIN_SET );
#endif

        // HAL_GPIO_WritePin(MODEM_RESET_PORT, MODEM_RESET_PIN, GPIO_PIN_SET );
        TimingDelay = 65;
        modem_seq_state = 5;

    }
    else
        switch (modem_seq_state)
        {
            case 0:
            {

                R_BSP_PinAccessEnable ();
                /* Write to this pin */
#ifdef EC200N
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
#else
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
#endif

 //               R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);

                R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_RESET_KEY_IN, BSP_IO_LEVEL_HIGH);
                R_BSP_PinAccessDisable ();

#if 0
          HAL_GPIO_WritePin(MODEM_POWER_KEY_PORT, MODEM_POWER_KEY_PIN, GPIO_PIN_RESET );
          HAL_GPIO_WritePin(MODEM_RESET_PORT, MODEM_RESET_PIN, GPIO_PIN_SET );
#endif

                TimingDelay = 50;
                modem_seq_state++;
                break;

            }
            case 1:
            {
#if 0
        HAL_GPIO_WritePin(MODEM_POWER_KEY_PORT, MODEM_POWER_KEY_PIN, GPIO_PIN_SET );
#endif

                R_BSP_PinAccessEnable ();
                /* Write to this pin */
                R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_HIGH);
                R_BSP_PinAccessDisable ();
                TimingDelay = 3;
                modem_seq_state++;
                break;

            }
            case 2:
            {
#if 0
        HAL_GPIO_WritePin(MODEM_RESET_PORT, MODEM_RESET_PIN, GPIO_PIN_RESET );
#endif
                R_BSP_PinAccessEnable ();
                /* Write to this pin */
                R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_RESET_KEY_IN, BSP_IO_LEVEL_LOW);
                R_BSP_PinAccessDisable ();

                TimingDelay = 47;
                modem_seq_state++;
                break;

            }
            case 3:
            {

                R_BSP_PinAccessEnable ();
                /* Write to this pin */
#ifdef EC200N
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
#else
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
#endif

  //              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
                R_BSP_PinAccessDisable ();

#if 0
      HAL_GPIO_WritePin(MODEM_POWER_KEY_PORT, MODEM_POWER_KEY_PIN, GPIO_PIN_RESET );
#endif

                modem_seq_state++;
                Gsm_Init ();
                //  TimingDelay = 100;

                break;

            }
            case 4:
            {
                // modem wake up do nothing wait for stop modem or start modem
            }
            case 5:
            {
                R_BSP_PinAccessEnable ();
                /* Write to this pin */
                R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_HIGH);
                R_BSP_PinAccessDisable ();

#if 0
       HAL_GPIO_WritePin(MODEM_POWER_KEY_PORT, MODEM_POWER_KEY_PIN, GPIO_PIN_SET );
#endif

                TimingDelay = 65;
                modem_seq_state++;
                break;

            }
            case 6:
            {
#if 0
       HAL_GPIO_WritePin(MODEM_POWER_KEY_PORT, MODEM_POWER_KEY_PIN, GPIO_PIN_RESET );
#endif
                R_BSP_PinAccessEnable ();
                /* Write to this pin */
#ifdef EC200N
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
#else
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
#endif

  //              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
                R_BSP_PinWrite ((bsp_io_port_pin_t) LTE_VCC_ENABLE, BSP_IO_LEVEL_LOW); // Power off ltu
                R_BSP_PinWrite ((bsp_io_port_pin_t) WAKE_UP_LTE, BSP_IO_LEVEL_LOW);
                R_BSP_PinAccessDisable ();

                modem_seq_state = 0;
                break;
            }
        }
}

void RTC_Alarm_Set(void)
{
    fsp_err_t status;
    rtc_alarm_time_t hour_alarm;
    hour_alarm.mday_match = false;
    hour_alarm.mon_match = false;
    hour_alarm.year_match = false;
    hour_alarm.mday_match = false;
    hour_alarm.hour_match = false;
    hour_alarm.sec_match = true; // once a hour 8/8/22
    hour_alarm.min_match = true;
    hour_alarm.time.tm_min = 1;
    hour_alarm.time.tm_sec = 10;
      // y.f. every hour set alarm to wake up processsor
    status = R_RTC_CalendarAlarmSet (&Yos1_RTC_ctrl, &hour_alarm);

}

void EnterDefaultTime(void)
{
    fsp_err_t status;

    // check if RTC holds valid time
                     // y.f. 22/8/2022 no need to set default time RTC holds valid time

            static rtc_time_t g_set_time =
            { .tm_hour = 23, .tm_isdst = 0, .tm_mday = 1, .tm_min = 10, .tm_mon = 3, .tm_sec = 12, .tm_wday = 1, .tm_yday = 4,
            .tm_year = 22 };
            // y.f. every hour set alarm to wake up processor
            R_RTC_CalendarTimeSet (&Yos1_RTC_ctrl, &g_set_time);

}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open(&g_ioport_ctrl, &g_bsp_pin_cfg);
    }
}

/* Callback function */
void yos1_rtc_callback(rtc_callback_args_t *p_args)
{
    /* TODO: add your own code here */

    if (p_args->event == RTC_EVENT_ALARM_IRQ)
    {

 //       min_counter++;
        RTC_Event = true;
    }
    /* TODO: add your own code here */

}

void ToggleLeds(void)
{
    R_BSP_PinAccessEnable ();
    if (toggle)
    {
        toggle = false;
        SetLed (PULSER_LED1, false);
        SetLed (PULSER_LED2, true);
    }
    else
    {
        toggle = true;
        SetLed (PULSER_LED1, true);
        SetLed (PULSER_LED2, false);
    }
    R_BSP_PinAccessDisable ();
}
void ClrLeds(void)
{
    R_BSP_PinAccessEnable ();
    R_BSP_PinWrite ((bsp_io_port_pin_t) PULSER_LED1, BSP_IO_LEVEL_HIGH);
    R_BSP_PinWrite ((bsp_io_port_pin_t) PULSER_LED2, BSP_IO_LEVEL_HIGH);
    R_BSP_PinAccessDisable ();
}
void SetLed(ioport_port_pin_t pin_num, bool STATE)
{
    R_BSP_PinAccessEnable ();

    if (pin_num == PULSER_LED1)
    {
        if (STATE)
            /* Write to this pin */
            R_BSP_PinWrite ((bsp_io_port_pin_t) PULSER_LED1, BSP_IO_LEVEL_LOW);

        //     HAL_GPIO_WritePin(LED_PORT, LED_1_PIN,GPIO_PIN_RESET);
        else
            R_BSP_PinWrite ((bsp_io_port_pin_t) PULSER_LED1, BSP_IO_LEVEL_HIGH);

        //      HAL_GPIO_WritePin(LED_PORT, LED_1_PIN,GPIO_PIN_SET);
    }
    else
    {
        if (STATE)
            R_BSP_PinWrite ((bsp_io_port_pin_t) PULSER_LED2, BSP_IO_LEVEL_LOW);

//       HAL_GPIO_WritePin(LED_PORT, LED_2_PIN,GPIO_PIN_RESET);
        else
            R_BSP_PinWrite ((bsp_io_port_pin_t) PULSER_LED2, BSP_IO_LEVEL_HIGH);

//       HAL_GPIO_WritePin(LED_PORT, LED_2_PIN,GPIO_PIN_SET);
    }
    R_BSP_PinAccessDisable ();

}
#if 0
void DelayYos(uint32_t TimeClock)
{
    uint32_t TimingDelaytmp = TimeClock;
    while (TimingDelaytmp--)
        ;

}
#endif

void EnterDefaultConfig(void)
{
    uint32_t *pDCU_id = 0xe000;
    memset ((uint8_t*) &RepeaterConfig, 0, sizeof(RepeaterConfigStruct));
 //   RepeaterConfig.DeviceInitalizaion=0;
#ifdef PROGRAME_BY_E2STUDIO
    RepeaterConfig.ExtendedAddress = DCU_id;

#else
    RepeaterConfig.ExtendedAddress = pDCU_id;
#endif

    RepeaterConfig.protocol = DLT_PROTOCOL; //DLMS_PROTOCOL;
    RepeaterConfig.MeterSerial.BaudRate = 19200;
    RepeaterConfig.MeterSerial.DataBit = 8;
    RepeaterConfig.MeterSerial.Parity = NONE;
    RepeaterConfig.MeterSerial.StopBit = 1;
    RepeaterConfig.GSMSerial.BaudRate = 115200;
    RepeaterConfig.GSMSerial.DataBit = 8;
    RepeaterConfig.GSMSerial.Parity = NONE;
    RepeaterConfig.GSMSerial.StopBit = 1;
    RepeaterConfig.DebugSerial.BaudRate = 9600;
    RepeaterConfig.DebugSerial.DataBit = 8;
    RepeaterConfig.DebugSerial.Parity = NONE;
    RepeaterConfig.DebugSerial.StopBit = 1;
    memcpy (RepeaterConfig.GsmConfig.APN, "internet", 8);
    // memcpy (RepeaterConfig.GsmConfig.Port ,"2334",4);
    // memcpy (RepeaterConfig.GsmConfig.ServerIP ,"2.55.107.55",11);
    //   memcpy (RepeaterConfig.GsmConfig.Port, "2339", 4);
      memcpy (RepeaterConfig.GsmConfig.Port, "2338", 4);

    //    memcpy (RepeaterConfig.GsmConfig.ServerIP, "31.154.8.241", 12);
    memcpy (RepeaterConfig.GsmConfig.ServerIP, "31.154.8.243", 13);
//  RepeaterConfig.Crc =  CrcBlockCalc((unsigned char*)&RepeaterConfig, sizeof(RepeaterConfigStruct)-2);
    WriteInternalFlashConfig ();
}
/* Callback function */
void Battery_low_callback(lvd_callback_args_t *p_args)
{
    /* TODO: add your own code here */
     if ((p_args->current_state)== LVD_CURRENT_STATE_BELOW_THRESHOLD)
         Event_status[0]|= LOW_BATTERY_DETECT;
}
#if 0
void StartWMP_lpm(void)
{
    fsp_err_t status;
             status = R_AGT_Stop (&Pulser_Timer0_Generic_ctrl); // y.f.  stop timer 0 before entering to sleep mode
             Uarts_Close ();
              R_BSP_PinAccessEnable ();
              /* Write to this pin */
              R_BSP_PinWrite ((bsp_io_port_pin_t) MODEM_POWER_KEY_PIN, BSP_IO_LEVEL_LOW);
              R_BSP_PinWrite ((bsp_io_port_pin_t) LTE_VCC_ENABLE, BSP_IO_LEVEL_LOW); // Power off ltu
              R_BSP_PinWrite ((bsp_io_port_pin_t) WAKE_UP_LTE, BSP_IO_LEVEL_LOW);
              R_BSP_PinWrite ((bsp_io_port_pin_t) DTR, BSP_IO_LEVEL_LOW);
              R_BSP_PinAccessDisable ();
              SetLed (PULSER_LED1, false);
              SetLed (PULSER_LED2, false);
              RTC_Alarm_Set();
              status = R_LPM_Open (&Meter_g_lpm0_ctrl, &Meter_g_lpm0_cfg);
              R_LPM_LowPowerModeEnter(&Meter_g_lpm0_ctrl);
            // wake up from sleep
              Uarts_Initialize ();
}

#endif

void HourProcess(int hour,int year,int month,int day)
{
  //  t.b.d.
}
