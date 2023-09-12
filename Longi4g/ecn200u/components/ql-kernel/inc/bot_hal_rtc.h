#ifndef __BOT_HAL_RTC_H__
#define __BOT_HAL_RTC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* rtc dev describe */
typedef struct {
    unsigned char  port;   /* rtc port */
    void          *priv;   /* priv data */
} bot_rtc_dev_t;

/*
 * RTC time
 */
typedef struct {
    unsigned char sec;      /* sconds, 0-59  */
    unsigned char min;      /* minutes, 0-59 */
    unsigned char hour;     /* hours, 0-23   */
    unsigned char weekday;  /* wekday, 1-7   */
    unsigned char date;     /* day, 1-31     */
    unsigned char month;    /* month, 1-12   */
    unsigned char year;     /* year, 0-127(2000-2127)*/
} bot_rtc_time_t;

/**
 * This function will initialize the on board CPU real time clock
 *
 *
 * @param[in]  rtc  rtc device
 *
 * @return  0 : on success, otherwise will be failed
 */
int bot_hal_rtc_init(bot_rtc_dev_t *rtc);

/**
 * This function will return the value of time read from the on board CPU real time clock.
 *
 * @param[in]   rtc   rtc device
 * @param[out]  time  pointer to a time structure
 *
 * @return  0 : on success,  otherwise will be failed
 */
int bot_hal_rtc_get_time(bot_rtc_dev_t *rtc, bot_rtc_time_t *time);

/**
 * This function will set MCU RTC time to a new value.
 *
 * @param[in]   rtc   rtc device
 * @param[in]   time  pointer to a time structure
 *
 * @return  0 : on success,  otherwise will be failed
 */
int bot_hal_rtc_set_time(bot_rtc_dev_t *rtc, const bot_rtc_time_t *time);

/**
 * De-initialises an RTC interface, Turns off an RTC hardware interface
 *
 * @param[in]  RTC  the interface which should be de-initialised
 *
 * @return  0 : on success,  otherwise will be failed
 */
int bot_hal_rtc_deinit(bot_rtc_dev_t *rtc);

#ifdef __cplusplus
}
#endif

#endif /* __BOT_HAL_RTC_H__ */
