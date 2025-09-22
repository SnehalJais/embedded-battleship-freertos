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
// Note: alarm variable will be used when implementing alarm functionality
bool speaker_icon_visible = false; // Is the speaker icon currently visible
uint8_t counter = 0;               // Counter used for blinking icons
cyhal_timer_t alarm_timer;         // Timer used to create 100mS time base
cyhal_timer_cfg_t alarm_timer_cfg; // Configuration structure for alarm_timer

cyhal_timer_t timer_obj;     // Timer used to create 1 second time base
cyhal_timer_cfg_t timer_cfg; // Configuration structure for timer_obj

alarm_clock_info_t alarm_info = {0}; // Alarm clock information structure

static uint8_t counter_msec_0100 = 0; // Counter used to create 500mS and 5000mS time bases
static uint8_t counter_msec_1000 = 0; // Counter used to create 1 second time base
static uint8_t counter_msec_500 = 0;  // Counter used to create 500mS time base
static uint8_t counter_msec_5000 = 0; // Counter used to create 5000mS(5 seconds) time base

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
        // Clear 100ms event flag and set 500ms event flag
        counter_msec_0100 = 0;
        ECE353_Events.tmr_msec_0500 = 1;

        // Increment 500ms counter
        counter_msec_500++;

        // Check for 5000ms interval (10 * 500ms)
        if (counter_msec_500 >= 10)
        {
            // Clear 500ms event flag and set 5000ms event flag
            ECE353_Events.tmr_msec_5000 = 1;
            counter_msec_500 = 0;
        }
    }
}

/*************************************************
 * @brief  Implement the state used for setting the time.
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_set_time(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{

    static bool alarm_icon_visible = true; // Is the alarm icon currently visible

    draw_speaker(LCD_COLOR_GRAY);                                    // Draw speaker icon in gray when setting time
    lcd_draw_time(alarm_info->time_hours, alarm_info->time_minutes); // Draw the current time

    // The alarm clock icon shall blink on/off every 500mS
    if (events->tmr_msec_0500)
    {
        events->tmr_msec_0500 = 0;                // Clear the 500ms event flag
        alarm_icon_visible = !alarm_icon_visible; // Toggle visibility

        // Draw alarm clock in c when not blinking
        if (alarm_icon_visible)
        {
            draw_alarm_clock(LCD_COLOR_YELLOW);
        }
        else
        {
            erase_alarm_clock();
        }
        alarm_info->next_state = STATE_HW01_SET_TIME;       // Remain in current state
        alarm_info->current_state = alarm_info->next_state; // Update current state
        return;
    }

    // // Draw alarm clock in current state when not blinking
    // if (alarm_icon_visible)
    // {
    //     draw_alarm_clock(LCD_COLOR_YELLOW);
    // }
    // Handle button presses
    if (events->sw1)
    {
        events->sw1 = 0;                 // Clear SW1 event flag
        alarm_info->time_hours++;        // Increment hours
        if (alarm_info->time_hours > 23) // Wrap around if > 23
        {
            alarm_info->time_hours = 0; // Reset to 0
        }
        alarm_info->next_state = STATE_HW01_SET_TIME;       // Remain in current state
        alarm_info->current_state = alarm_info->next_state; // Update current state
        return;
    }

    // Handle button presses
    if (events->sw2)
    {
        events->sw2 = 0;                    // Clear SW2 event flag
        alarm_info->time_minutes++;         // Increment minutes
        if (alarm_info->time_minutes >= 60) // Wrap around if >= 60
        {
            alarm_info->time_minutes = 0; // Reset to 0
        }
        alarm_info->next_state = STATE_HW01_SET_TIME;       // Remain in current state
        alarm_info->current_state = alarm_info->next_state; // Update current state
        return;
    }

    // Handle button presses
    if (events->sw3)
    {
        events->sw3 = 0;                                    // Clear SW3 event flag
        lcd_clear_screen(LCD_COLOR_BLACK);                  // Clear screen when switching states
        alarm_info->next_state = STATE_HW01_SET_ALARM;      // Transition to SET_ALARM state
        alarm_info->current_state = alarm_info->next_state; //  Update current state
        return;
    }

    alarm_info->next_state = STATE_HW01_SET_TIME;       // Remain in current state
    alarm_info->current_state = alarm_info->next_state; // Update current state
    return;
}

/*************************************************
 * @brief Implement the state used for setting the alarm.
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_set_alarm(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{

    static bool speaker_icon_visible = true; // Is the speaker icon currently visible

    // Only draw alarm clock (doesn't blink) and time
    draw_alarm_clock(LCD_COLOR_GREEN);
    lcd_draw_time(alarm_info->alarm_hours, alarm_info->alarm_minutes);

    // The speaker icon shall blink on/off every 500mS
    if (events->tmr_msec_0500)
    {
        events->tmr_msec_0500 = 0;                    // Clear the 500ms event flag
        speaker_icon_visible = !speaker_icon_visible; // Toggle visibility
        // Draw speaker in yellow when not blinking
        if (speaker_icon_visible)
        {
            draw_speaker(LCD_COLOR_YELLOW);
        }
        else
        {
            erase_speaker();
        }
        alarm_info->next_state = STATE_HW01_SET_ALARM;      // Remain in current state
        alarm_info->current_state = alarm_info->next_state; // Update current state
        return;
    }

    // Handle button presses
    if (events->sw1)
    {

        alarm_info->alarm_hours++;        // Increment hours
        if (alarm_info->alarm_hours > 23) // Wrap around if > 23
        {
            alarm_info->alarm_hours = 0; // Reset to 0
        }
        events->sw1 = 0;                                    // Clear SW1 event flag
        alarm_info->next_state = STATE_HW01_SET_ALARM;      //  Remain in current state
        alarm_info->current_state = alarm_info->next_state; // Update current state
        return;
    }

    // Handle button presses
    if (events->sw2)
    {

        alarm_info->alarm_minutes++;        // Increment minutes
        if (alarm_info->alarm_minutes > 59) // Wrap around if > 59
        {
            alarm_info->alarm_minutes = 0; // Reset to 0
        }
        events->sw2 = 0;                                    // Clear SW2 event flag
        alarm_info->next_state = STATE_HW01_SET_ALARM;      // Remain in current state
        alarm_info->current_state = alarm_info->next_state; // Update current state
        return;
    }

    // Handle button presses
    if (events->sw3)
    {
        events->sw3 = 0;                                    // Clear SW3 event flag
        lcd_clear_screen(LCD_COLOR_BLACK);                  // Clear screen when switching states
        alarm_info->next_state = STATE_HW01_RUNNING;        // Transition to RUNNING state
        alarm_info->current_state = alarm_info->next_state; //  Update current state
        return;
    }

    alarm_info->next_state = STATE_HW01_SET_ALARM;      // Remain in current state
    alarm_info->current_state = alarm_info->next_state; // Update current state
    return;
}

/*************************************************
 * @brief Implement the state used for running the alarm clock.
 *
 * @param alarm_info
 * @param events
 ************************************************/
void hw01_state_running(
    alarm_clock_info_t *alarm_info,
    volatile ece353_events_t *events)
{

    static bool alarm_icon_visible = true; // Is the alarm icon currently visible

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

    lcd_draw_time(alarm_info->time_hours, alarm_info->time_minutes); // Draw the current time

    // if sw1 is pressed, disable enable alarm setting state
    if (events->sw1)
    {
        events->sw1 = 0;
        alarm_info->alarm_enabled = !alarm_info->alarm_enabled; // Toggle alarm enabled status
        // Draw speaker based on alarm_enabled status
        if (alarm_info->alarm_enabled)
        {
            draw_speaker(LCD_COLOR_GREEN);
        }

        else
        {
            draw_speaker(LCD_COLOR_GRAY);
        }

        return;
    }

    // Check if alarm should trigger
    if (alarm_info->alarm_enabled && (alarm_info->time_hours == alarm_info->alarm_hours) && (alarm_info->time_minutes == alarm_info->alarm_minutes))
    {

        alarm_info->next_state = STATE_HW01_ALARM_TRIGGERED; //  Transition to ALARM_TRIGGERED state
        alarm_info->alarm_sounding = true;                   // Set flag to start buzzer
        alarm_info->current_state = alarm_info->next_state;  // Update current state
        return;
    }

    // Handle button presses
    if (events->sw3)
    {
        events->sw3 = 0;                                    // Clear SW3 event flag
        lcd_clear_screen(LCD_COLOR_BLACK);                  // Clear screen when switching states
        alarm_info->next_state = STATE_HW01_SET_TIME;       // Transition to SET_TIME state
        alarm_info->current_state = alarm_info->next_state; //  Update current state
        return;
    }

    // Update time display every 100ms
    if (events->tmr_msec_0100)
    {
        // increment time every 100ms
        alarm_info->time_minutes++;
        // Wrap around if >= 60
        if (alarm_info->time_minutes >= 60)
        {
            alarm_info->time_minutes = 0;
            // Increment hours and wrap around if >= 24
            alarm_info->time_hours++;
            if (alarm_info->time_hours >= 24)
            {
                alarm_info->time_hours = 0;
            }
        }
        events->tmr_msec_0100 = 0; // Clear the 100ms event flag

        return;
    }

    return;
}

/*************************************************
 * @brief Implement the state used when the alarm is triggered.
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

    // Static variable to track speaker icon visibility
    if (first_entry)
    {
        first_entry = false;
        alarm_info->alarm_sounding = true; // ensure we're sounding
        buzzer_on();                       // Start the buzzer
    }

    //  Keep buzzer on while alarm is sounding
    if (alarm_info->alarm_sounding)
    {
        buzzer_on();
    }

    // Draw the display (only once per entry, not in the loop)
    lcd_draw_time(alarm_info->alarm_hours, alarm_info->alarm_minutes);
    draw_alarm_clock(LCD_COLOR_GREEN); // Draw alarm clock in green when alarm is triggered
    // Draw speaker in red when alarm is triggered
    if (events->tmr_msec_0100)
    {
        counter++;                 // Increment counter every 100ms
        events->tmr_msec_0100 = 0; // Clear the 100ms event flag

        // Continue incrementing current time even during alarm start
        alarm_info->time_minutes++;
        // Wrap around if >= 60
        if (alarm_info->time_minutes >= 60)
        {
            alarm_info->time_minutes = 0;
            // Increment hours and wrap around if >= 24
            alarm_info->time_hours++;
            if (alarm_info->time_hours >= 24)
            {
                alarm_info->time_hours = 0;
            }
        }

        // Update the time display
        lcd_draw_time(alarm_info->time_hours, alarm_info->time_minutes);

        speaker_icon_visible = !speaker_icon_visible; // Toggle visibility
                                                      // Draw speaker in red when alarm is triggered
        if (speaker_icon_visible)
        {
            draw_speaker(LCD_COLOR_RED);
        }
        else
        {
            erase_speaker();
        }
        // After 5 seconds or if SW2 is pressed, stop the alarm
        if (counter == 50 || events->sw2)
        {
            counter = 0;                                        // Reset counter
            buzzer_off();                                       // Stop the buzzer
            alarm_info->alarm_enabled = false;                  // Disable the alarm
            alarm_info->alarm_sounding = false;                 // Stop sounding
            alarm_info->alarm_minutes = 0;                      // Reset alarm time to prevent immediate re-triggering
            alarm_info->alarm_hours = 0;                        // Reset alarm hours
            alarm_info->next_state = STATE_HW01_RUNNING;        // Transition to RUNNING state
            alarm_info->current_state = alarm_info->next_state; //  Update current state
        }

        // Increment alarm time to prevent immediate re-triggering
        alarm_info->alarm_minutes++;
        // Wrap around if >= 60
        if (alarm_info->alarm_minutes >= 60)
        {
            alarm_info->alarm_minutes = 0;
            // Increment hours and wrap around if >= 24
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
 * @brief Implement the state used when an error is detected.
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
    cy_rslt_t rslt; // Variable used to capture return values

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

    // Initialize button timer
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

    // initialize cyhal_timer_start
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
            // Initial state
        case STATE_HW01_INIT:
            lcd_clear_screen(LCD_COLOR_BLACK);                // Clear screen when switching states
            alarm_info.next_state = STATE_HW01_SET_TIME;      // Transition to SET_TIME state
            alarm_info.current_state = alarm_info.next_state; // Update current state
            break;

            // State for setting the time
        case STATE_HW01_SET_TIME:
            hw01_state_set_time(&alarm_info, &ECE353_Events); // Handle SET_TIME state
            break;

            // State for setting the alarm
        case STATE_HW01_SET_ALARM:
            hw01_state_set_alarm(&alarm_info, &ECE353_Events); // Handle SET_ALARM state
            break;

            // State for running the alarm clock
        case STATE_HW01_RUNNING:
            hw01_state_running(&alarm_info, &ECE353_Events); // Handle RUNNING state
            break;

            // State for when the alarm is triggered
        case STATE_HW01_ALARM_TRIGGERED:
            hw01_state_alarm_triggered(&alarm_info, &ECE353_Events); // Handle ALARM_TRIGGERED state
            break;

        case STATE_HW01_ERROR:
            hw01_state_error(&alarm_info, &ECE353_Events); // Handle ERROR state
            break;

        default:
            alarm_info.current_state = STATE_HW01_ERROR; // Transition to ERROR state
            break;
        }
    }
}
#endif