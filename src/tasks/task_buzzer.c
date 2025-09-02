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
    }
}
#endif