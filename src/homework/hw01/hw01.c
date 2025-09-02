/**
 * @file hw01.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #include "hw01.h"

#if defined(HW01)

char APP_DESCRIPTION[] = "ECE353 F25 HW01 -- Alarm Clock";

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/

/*************************************************
* Handler used to time various events. 
*************************************************/
void handler_alarm_timer(void *arg, cyhal_timer_event_t event)
{
}

/*************************************************
 * @brief 
 * 
 * @param alarm_info 
 * @param events 
 ************************************************/
void hw01_state_set_time(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events
)
{
}

/*************************************************
 * @brief 
 * 
 * @param alarm_info 
 * @param events 
 ************************************************/
void hw01_state_set_alarm(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events
)
{
}

/*************************************************
 * @brief 
 * 
 * @param alarm_info 
 * @param events 
 ************************************************/
void hw01_state_running(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events
)
{
}

/*************************************************
 * @brief 
 * 
 * @param alarm_info 
 * @param events 
 ************************************************/
void hw01_state_alarm_triggered(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events
)
{
}

/*************************************************
 * @brief 
 * 
 * @param alarm_info 
 * @param events 
 ************************************************/
void hw01_state_error(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events
)
{
    printf("State: Error \r\n");

    for(int i = 0; i < 100000; i++); // Delay
    CY_ASSERT(0);

    /* Will never reach this line*/
}

/*************************************************
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 ************************************************/
void app_init_hw(void)
{
    cy_rslt_t rslt;

    console_init();
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");
}

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void)
{
    while(1)
    {
    }
}
#endif