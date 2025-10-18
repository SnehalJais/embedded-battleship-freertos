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
            ECE353_RTOS_Events,           // Event group handle
            SW1_PRESSED | SW2_PRESSED,    // Bits to wait for (SW1 OR SW2)
            pdTRUE,                       // Clear bits on exit
            pdFALSE,                      // Wait for ANY bit (not all)
            portMAX_DELAY                 // Wait indefinitely
        );
        
        if (events & SW1_PRESSED)
        {
            printf("SW1 Pressed - Buzzer ON\n\r");
            buzzer_on();
        }
        if (events & SW2_PRESSED)
        {
            printf("SW2 Pressed - Buzzer OFF\n\r");
            buzzer_off();
        }
    }
}
#endif