/**
 * @file task_buzzer.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#ifdef ECE353_FREERTOS

#include "task_buzzer.h"
#include "rtos_events.h"

/**
 * @brief 
 * Task used to control the buzzer based on button events.
 * 
 * SW1 -- Turn buzzer on
 * SW2 -- Turn buzzer off
 *
 * @param arg 
 * Unused parameter
 */
void task_buzzer(void *arg)
{
    (void)arg; // Unused parameter
    while (1)
    {
        // Wait for either SW1 or SW2 button press events
        EventBits_t events = xEventGroupWaitBits(
            ECE353_RTOS_Events,                                    // Event group handle
            ECE353_RTOS_EVENTS_SW1 | ECE353_RTOS_EVENTS_SW2,     // Bits to wait for (SW1 OR SW2)
            pdTRUE,                                               // Clear bits on exit
            pdFALSE,                                              // Wait for ANY bit (not all)
            portMAX_DELAY                                         // Wait indefinitely
        );
        
        if (events & ECE353_RTOS_EVENTS_SW1)
        {
            printf("SW1 Pressed - Buzzer ON\n\r");
            buzzer_on();
        }
        if (events & ECE353_RTOS_EVENTS_SW2)
        {
            printf("SW2 Pressed - Buzzer OFF\n\r");
            buzzer_off();
        }
    }
}
#endif