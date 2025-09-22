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
#include "cyhal.h"
#include "cyhal_rtc.h"
#include "cyhal_timer.h"
#include "timer.h"
#include "lcd-io.h"
#include "lcd-fonts.h"
#include "console.h"
#include "buttons.h"
#include "buzzer.h"
#include "ece353-events.h"
#include "ece353-pins.h"
#include "hw01-images.h"
#include <stdio.h>

#if defined(HW01)

char APP_DESCRIPTION[] = "ECE353 F25 HW01 -- Alarm Clock";

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
bool speaker_icon_visible = false;
uint8_t counter = 0;
cyhal_timer_t alarm_timer;
cyhal_timer_cfg_t alarm_timer_cfg;

cyhal_timer_t timer_obj;
cyhal_timer_cfg_t timer_cfg;

alarm_clock_info_t alarm_info = {0};

static uint8_t counter_msec_0100 = 0;
static uint8_t counter_msec_1000 = 0;
static uint8_t counter_msec_500 = 0;
static uint8_t counter_msec_5000 = 0;

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
// Note: alarm variable will be used when implementing alarm functionality
/*************************************************
 * Handler used to time various events.
 *************************************************/
void handler_alarm_timer(void *arg, cyhal_timer_event_t event)
{
    // Set 100ms event flag every time this handler is called
    ECE353_Events.tmr_msec_0100 = 1;

    // Increment 100ms counter
    counter_msec_0100++;

    // Check for 500ms interval (5 * 100ms)
    if (counter_msec_0100 >= 5)
    {
        counter_msec_0100 = 0;
        ECE353_Events.tmr_msec_0500 = 1;

        // Increment 500ms counter
        counter_msec_500++;

        // Check for 5000ms interval (10 * 500ms)
        if (counter_msec_500 >= 10)
        {

            ECE353_Events.tmr_msec_5000 = 1;
            counter_msec_500 = 0;
        }
    }
}

/*************************************************
 * @brief
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_set_time(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{

    static bool alarm_icon_visible = true;

    draw_speaker(LCD_COLOR_GRAY);
    lcd_draw_time(alarm_info->time_hours, alarm_info->time_minutes);

    // The alarm clock icon shall blink on/off every 500mS
    if (events->tmr_msec_0500)
    {
        events->tmr_msec_0500 = 0;
        alarm_icon_visible = !alarm_icon_visible;

        if (alarm_icon_visible)
        {
            draw_alarm_clock(LCD_COLOR_YELLOW);
        }
        else
        {
            erase_alarm_clock();
        }
        alarm_info->next_state = STATE_HW01_SET_TIME;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    // Draw alarm clock in current state when not blinking
    if (alarm_icon_visible)
    {
        draw_alarm_clock(LCD_COLOR_YELLOW);
    }

    if (events->sw1)
    {
        events->sw1 = 0;
        alarm_info->time_hours++;
        if (alarm_info->time_hours > 23)
        {
            alarm_info->time_hours = 0;
        }
        alarm_info->next_state = STATE_HW01_SET_TIME;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    if (events->sw2)
    {
        events->sw2 = 0;
        alarm_info->time_minutes++;
        if (alarm_info->time_minutes >= 60)
        {
            alarm_info->time_minutes = 0;
        }
        alarm_info->next_state = STATE_HW01_SET_TIME;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    if (events->sw3)
    {
        events->sw3 = 0;
        lcd_clear_screen(LCD_COLOR_BLACK);
        alarm_info->next_state = STATE_HW01_SET_ALARM;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    alarm_info->next_state = STATE_HW01_SET_TIME;
    alarm_info->current_state = alarm_info->next_state;
    return;
}

/*************************************************
 * @brief
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_set_alarm(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{

    static bool speaker_icon_visible = true;

    // Only draw alarm clock (doesn't blink) and time
    draw_alarm_clock(LCD_COLOR_GREEN);
    lcd_draw_time(alarm_info->alarm_hours, alarm_info->alarm_minutes);

    // The speaker icon shall blink on/off every 500mS
    if (events->tmr_msec_0500)
    {
        events->tmr_msec_0500 = 0;
        speaker_icon_visible = !speaker_icon_visible;
        if (speaker_icon_visible)
        {
            draw_speaker(LCD_COLOR_YELLOW);
        }
        else
        {
            erase_speaker();
        }
        alarm_info->next_state = STATE_HW01_SET_ALARM;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    // Handle button presses
    if (events->sw1)
    {
        
        alarm_info->alarm_hours++;
        if (alarm_info->alarm_hours > 23)
        {
            alarm_info->alarm_hours = 0;
        }
        events->sw1 = 0;
        alarm_info->next_state = STATE_HW01_SET_ALARM;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }
    if (events->sw2)
    {
        
        alarm_info->alarm_minutes++;
        if (alarm_info->alarm_minutes > 59)
        {
            alarm_info->alarm_minutes = 0;
        }
        events->sw2 = 0;
        alarm_info->next_state = STATE_HW01_SET_ALARM;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    if (events->sw3)
    {
        events->sw3 = 0;
        lcd_clear_screen(LCD_COLOR_BLACK);
        alarm_info->next_state = STATE_HW01_RUNNING;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    alarm_info->next_state = STATE_HW01_SET_ALARM;
    alarm_info->current_state = alarm_info->next_state;
    return;
}

/*************************************************
 * @brief
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_running(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{


    static bool alarm_icon_visible = true;

    // make alarm clock green in running state
    draw_alarm_clock(LCD_COLOR_GREEN);

    // Draw speaker based on alarm_enabled status
    if (alarm_info->alarm_enabled)
    {
        draw_speaker(LCD_COLOR_GREEN);
    }
    else
    {
        draw_speaker(LCD_COLOR_GRAY);
    }

    lcd_draw_time(alarm_info->time_hours, alarm_info->time_minutes);
    // if sw1 is pressed, disable enable alarm setting state

    if (events->sw1)
    {

        events->sw1 = 0;
        alarm_info->alarm_enabled = !alarm_info->alarm_enabled;
        // if (alarm_info->alarm_enabled)
        // {
        //    draw_speaker(LCD_COLOR_GREEN);
        // }

        // else
        // {
        //     draw_speaker(LCD_COLOR_GRAY);
        // }

        // return;
    }

    if (alarm_info->alarm_enabled && (alarm_info->time_hours == alarm_info->alarm_hours) && (alarm_info->time_minutes == alarm_info->alarm_minutes))
    {

        alarm_info->next_state = STATE_HW01_ALARM_TRIGGERED;
        alarm_info->alarm_sounding = true; // Set flag to start buzzer
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    if (events->sw3)
    {
        events->sw3 = 0;
        lcd_clear_screen(LCD_COLOR_BLACK);
        alarm_info->next_state = STATE_HW01_SET_TIME;
        alarm_info->current_state = alarm_info->next_state;
        return;
    }

    // Update time display every 100ms
    if (events->tmr_msec_0100)
    {
        // increment time every 100ms
        alarm_info->time_minutes++;
        if (alarm_info->time_minutes >= 60)
        {
            alarm_info->time_minutes = 0;
            alarm_info->time_hours++;
            if (alarm_info->time_hours >= 24)
            {
                alarm_info->time_hours = 0;
            }
        }
        events->tmr_msec_0100 = 0;

        return;
    }

    return;
}

/*************************************************
 * @brief
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_alarm_triggered(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{
    // Run-once setup when we first enter this state
    static bool first_entry = true;

    if (first_entry)
    {
        first_entry = false;
        alarm_info->alarm_sounding = true; // ensure we're sounding
        buzzer_on();
    }
    // new
    //  Keep buzzer on while alarm is sounding
    if (alarm_info->alarm_sounding)
    {
        buzzer_on();
    }
    // new

    // Draw the display (only once per entry, not in the loop)
    lcd_draw_time(alarm_info->alarm_hours, alarm_info->alarm_minutes);
    draw_alarm_clock(LCD_COLOR_GREEN);
    if (events->tmr_msec_0100)
    {
        counter++;
        events->tmr_msec_0100 = 0;

        // Continue incrementing current time even during alarm start
        alarm_info->time_minutes++;
        if (alarm_info->time_minutes >= 60)
        {
            alarm_info->time_minutes = 0;
            alarm_info->time_hours++;
            if (alarm_info->time_hours >= 24)
            {
                alarm_info->time_hours = 0;
            }
        }
        // end

        // Update the time display
        lcd_draw_time(alarm_info->time_hours, alarm_info->time_minutes);

        speaker_icon_visible = !speaker_icon_visible;
        if (speaker_icon_visible)
        {
            draw_speaker(LCD_COLOR_RED);
        }
        else
        {
            erase_speaker();
        }
        if (counter == 50 || events->sw2)
        {
            counter = 0;
            buzzer_off();
            alarm_info->alarm_enabled = false;
            alarm_info->alarm_sounding = false;
            alarm_info->alarm_minutes = 0;
            alarm_info->alarm_hours = 0;
            alarm_info->next_state = STATE_HW01_RUNNING;
            alarm_info->current_state = alarm_info->next_state;
        }

        // Increment alarm time to prevent immediate re-triggering
        alarm_info->alarm_minutes++;
        if (alarm_info->alarm_minutes >= 60)
        {
            alarm_info->alarm_minutes = 0;
            alarm_info->alarm_hours++;
            if (alarm_info->alarm_hours >= 24)
            {
                alarm_info->alarm_hours = 0;
            }
        }
    }

    return;
}

/*************************************************
 * @brief
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_error(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{
    printf("State: Error \r\n");

    for (int i = 0; i < 100000; i++)
        ; // Delay
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

    // Initialize LCD
    printf("Initializing LCD...\r\n");
    rslt = lcd_initialize();
    printf("LCD setup complete\r\n");
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("LCD initialization failed: %lx\r\n", rslt);
        CY_ASSERT(0);
    }

    // Initialize buttons
    printf("Initializing buttons...\r\n");
    rslt = buttons_init_gpio();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Button GPIO init failed: %lx\r\n", rslt);
    }
    else
    {
        printf("Button GPIO init successful\r\n");
    }

    rslt = buttons_init_timer();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Button timer init failed: %lx\r\n", rslt);
    }
    else
    {
        printf("Button timer init successful\r\n");
    }

    // Initialize buzzer
    printf("Initializing buzzer...\r\n");
    rslt = buzzer_init(0.5f, 3500); // 50% duty cycle, 3.5KHz
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Buzzer init failed: %lx\r\n", rslt);
    }

    // Initialize timer for 100ms intervals
    printf("Initializing timer...\r\n");
    rslt = timer_init(&timer_obj, &timer_cfg, 10000000, handler_alarm_timer); // 100ms = 100MHz/1M
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Timer init failed: %lx\r\n", rslt);
    }
    else
    {
        printf("Timer initialized successfully!\r\n");
    }

    // //initialize cyhal_timer_start
    cyhal_timer_start(&timer_obj);

    // Clear LCD screen to start with clean display
    lcd_clear_screen(LCD_COLOR_BLACK);

    printf("Hardware initialization complete\r\n");
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

   

    while (1)
    {
       

        // Process current state
        switch (alarm_info.current_state)
        {

        case STATE_HW01_INIT:
            printf("Alarm system initialized. Transitioning to Set Time state.\r\n");
            lcd_clear_screen(LCD_COLOR_BLACK);
            alarm_info.next_state = STATE_HW01_SET_TIME;
            alarm_info.current_state = alarm_info.next_state;
            break;

        case STATE_HW01_SET_TIME:
            hw01_state_set_time(&alarm_info, &ECE353_Events);
            break;

        case STATE_HW01_SET_ALARM:
            hw01_state_set_alarm(&alarm_info, &ECE353_Events);
            break;

        case STATE_HW01_RUNNING:
            hw01_state_running(&alarm_info, &ECE353_Events);
            break;

        case STATE_HW01_ALARM_TRIGGERED:
            hw01_state_alarm_triggered(&alarm_info, &ECE353_Events);
            break;

        case STATE_HW01_ERROR:
            hw01_state_error(&alarm_info, &ECE353_Events);
            break;

        default:
            alarm_info.current_state = STATE_HW01_ERROR;
            break;
        }
    }
}
#endif