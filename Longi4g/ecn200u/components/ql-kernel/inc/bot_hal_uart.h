
#ifndef __BOT_HAL_UART_H__
#define __BOT_HAL_UART_H__

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup hal_uart UART
 *  uart hal API.
 *
 *  @{
 */

#include <stdint.h>
#include "bot_system.h"

/* Define the wait forever timeout macro */
#define HAL_WAIT_FOREVER 0xFFFFFFFFU

/*
 * UART logic port
 */
typedef enum {
    UART_PORT_AT = 0,
    UART_PORT_GNSS,
    UART_PORT_MAX
} bot_uart_logic_port_e;

/*
 * UART data width
 */
typedef enum {
    DATA_WIDTH_5BIT,
    DATA_WIDTH_6BIT,
    DATA_WIDTH_7BIT,
    DATA_WIDTH_8BIT,
    DATA_WIDTH_9BIT
} bot_uart_data_width_e;

/*
 * UART stop bits
 */
typedef enum {
    STOP_BITS_1,
    STOP_BITS_2
} bot_uart_stop_bits_e;

/*
 * UART flow control
 */
typedef enum {
    FLOW_CONTROL_DISABLED,  /**< Flow control disabled */
    FLOW_CONTROL_CTS,       /**< Clear to send, yet to send data */
    FLOW_CONTROL_RTS,       /**< Require to send, yet to receive data */
    FLOW_CONTROL_CTS_RTS    /**< Both CTS and RTS flow control */
} bot_uart_flow_control_e;

/*
 * UART parity
 */
typedef enum {
    NO_PARITY,      /**< No parity check */
    ODD_PARITY,     /**< Odd parity check */
    EVEN_PARITY     /**< Even parity check */
} bot_uart_parity_e;

/*
 * UART mode
 */
typedef enum {
    MODE_TX,        /**< Uart in send mode */
    MODE_RX,        /**< Uart in receive mode */
    MODE_TX_RX      /**< Uart in send and receive mode */
} bot_uart_mode_e;

/*
 * UART configuration
 */
typedef struct {
    unsigned int            baud_rate;      /**< Uart baud rate */
    bot_uart_data_width_e   data_width;     /**< Uart data width */
    bot_uart_parity_e       parity;         /**< Uart parity check mode */
    bot_uart_stop_bits_e    stop_bits;      /**< Uart stop bit mode */
    bot_uart_flow_control_e flow_control;   /**< Uart flow control mode */
    bot_uart_mode_e         mode;           /**< Uart send/receive mode */
} bot_uart_config_t;

/*
 * UART dev handle
 */
typedef struct {
    unsigned char      port;   /**< uart port */
    bot_uart_config_t  config; /**< uart config */
    void              *priv;   /**< priv data */
} bot_uart_dev_t;

/* UART data receive callback */

typedef void (*bot_uart_rx_cb)(int port, void* data, unsigned int len, void* arg);


/**
 * Initialises a UART interface
 *
 *
 * @param[in]  uart  uart handle
 *
 * @return  0 : on success, otherwise: fail.
 */
int bot_hal_uart_init(bot_uart_dev_t *uart);

/**
 * Transmit data on a UART interface
 *
 * @param[in]  uart     uart handle
 * @param[in]  data     pointer to the start of data
 * @param[in]  size     number of bytes to transmit
 * @param[in]  timeout  timeout in miliseconds, set this value to HAL_WAIT_FOREVER
 *                      if you want to wait forever
 *
 * @return  0 : on success, otherwise: fail.
 */
int bot_hal_uart_send(bot_uart_dev_t *uart, const void *data, unsigned int size, unsigned int timeout);

/**
 * Receive data on a UART interface
 *
 * @param[in]   uart         uart handle
 * @param[out]  data         pointer to the buffer which will store incoming data
 * @param[in]   expect_size  number of bytes to receive
 * @param[out]  recv_size    number of bytes trully received
 * @param[in]   timeout      timeout in miliseconds, set this value to HAL_WAIT_FOREVER
 *                           if you want to wait forever
 *
 * @return  0 : on success, otherwise: fail.
 */
int bot_hal_uart_recv(bot_uart_dev_t *uart, void *data, unsigned int expect_size,
                         unsigned int *recv_size, unsigned int timeout);

/*
 * Register UART data receive call back
 *
 * @param [in]   uart         uart handle
 * @param [in]   cb           Non-zero pointer is the rx callback handler;
 *                            NULL pointer for rx_cb unregister operation
 *                            uart in rx_cb must be the same pointer with uart pointer passed to hal_uart_recv_cb_reg
 *                            driver must notify upper layer by calling cb if data is available in UART's hw or rx buffer
 * @param [in]   args         user args for rx_cb
 *
 * @return 0: on success, otherwise: fail.
 */
int bot_hal_uart_recv_cb_register(bot_uart_dev_t *uart, bot_uart_rx_cb cb, void *args);

/**
 * Deinitialises a UART interface
 *
 * @param[in]  uart  uart handle
 *
 * @return  0 : on success, otherwise: fail.
 */
int bot_hal_uart_deinit(bot_uart_dev_t *uart);


/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __BOT_HAL_UART_H__ */

