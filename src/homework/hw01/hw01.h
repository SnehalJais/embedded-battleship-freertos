/**
 * @file hw01.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-12
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef __HW01_H__
 #define __HW01_H__

 #include "main.h"
 #include "drivers.h"
 #include "hw01-images.h"

 /*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

 /*****************************************************************************/
/* States -- DO NOT ADD ANY ADDITIONAL STATES                                */ 
/*****************************************************************************/
typedef enum {
    STATE_HW01_INIT = 0,
    STATE_HW01_SET_TIME,
    STATE_HW01_SET_ALARM,
    STATE_HW01_RUNNING,
    STATE_HW01_ALARM_TRIGGERED,
    STATE_HW01_ERROR
} hw01_state_t;

/*****************************************************************************/
/* Structs                                                                   */
/*****************************************************************************/

// Alarm Clock State -- ADD/Remove any fields as necessary
typedef struct {
    uint8_t time_hours;
    uint8_t time_minutes;
    uint8_t alarm_hours;
    uint8_t alarm_minutes;
    hw01_state_t current_state;
    hw01_state_t next_state;
} alarm_clock_info_t;


/*****************************************************************************/
/* Function Declarations                                                     */
/* You are allowed to add additional function declarations here.             */
/* You can modify the function declarations as needed.                       */
/*****************************************************************************/

/**
 * @brief 
 * Implement the state used for setting the time.
 * @param state 
 * @param events 
 */
void hw01_state_set_time(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events
);

/**
 * @brief 
 * Implement the state used for setting the alarm.
 * @param alarm_info 
 * @param events 
 */
void hw01_state_set_alarm(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events 
);

/**
 * @brief 
 * Implement the state used for running the alarm clock.
 * @param alarm_info 
 * @param events 
 */
void hw01_state_running(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events 
);

/**
 * @brief 
 * Implement the state used for alarm triggered.
 * @param alarm_info 
 * @param events 
 */
void hw01_state_alarm_triggered(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events 
);

/**
 * @brief 
 * Implement the state used for error handling.
 * @param alarm_info 
 * @param events 
 */
void hw01_state_error(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events
);

/**
 * @brief 
 *  Handler for the alarm timer.
 * @param arg 
 * @param event 
 */
void handler_alarm_timer(void *arg, cyhal_timer_event_t event);


#endif /* __HW01_H__ */