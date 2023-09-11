/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"
#define ARDUINO_A0_MIKROBUS_AN (IOPORT_PORT_00_PIN_00)
#define MODEM_POWER_KEY_PIN (IOPORT_PORT_00_PIN_01)
#define WAKE_UP_LTE (IOPORT_PORT_00_PIN_03)
#define ARDUINO_A4 (IOPORT_PORT_00_PIN_14)
#define ARDUINO_A5 (IOPORT_PORT_00_PIN_15)
#define ARDUINO_RX_MIKROBUS_RX (IOPORT_PORT_01_PIN_00)
#define ARDUINO_TX_MIKROBUS_TX (IOPORT_PORT_01_PIN_01)
#define MODEM_RESET_KEY_IN (IOPORT_PORT_01_PIN_02)
#define WRITE_DISABLE_LTE (IOPORT_PORT_01_PIN_04)
#define ARDUINO_D2 (IOPORT_PORT_01_PIN_05)
#define ARDUINO_D3 (IOPORT_PORT_01_PIN_11)
#define ARDUINO_SS_MIKCRBUS_SS (IOPORT_PORT_02_PIN_05)
#define PULSER_LED1 (IOPORT_PORT_02_PIN_06)
#define PULSER_LED2 (IOPORT_PORT_02_PIN_07)
#define PMOD1_SS3 (IOPORT_PORT_03_PIN_02)
#define LTE_VCC_ENABLE (IOPORT_PORT_03_PIN_03)
#define ARDUINO_D7 (IOPORT_PORT_03_PIN_04)
#define LED3 (IOPORT_PORT_04_PIN_00)
#define RADIO_RESET (IOPORT_PORT_04_PIN_02)
#define USB_VBUS (IOPORT_PORT_04_PIN_07)
#define RX_DEBUG_PIN (IOPORT_PORT_04_PIN_08)
#define TX_DEBUG_PIN (IOPORT_PORT_04_PIN_09)
#define RX_LTE_PIN (IOPORT_PORT_04_PIN_10)
#define TX_LTE_PIN (IOPORT_PORT_04_PIN_11)
#define DTR (IOPORT_PORT_05_PIN_00)
extern const ioport_cfg_t g_bsp_pin_cfg; /* RA4M3 EK */

void BSP_PinConfigSecurityInit();
#endif /* BSP_PIN_CFG_H_ */
