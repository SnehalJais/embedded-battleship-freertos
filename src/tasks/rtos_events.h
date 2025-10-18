/**
 * @file rtos_events.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #ifndef __RTOS_EVENTS_H__
 #define __RTOS_EVENTS_H__
 #include "main.h"
 
#ifdef ECE353_FREERTOS

/*******************************************************************************
* Event Group for system events.
 ******************************************************************************/
extern EventGroupHandle_t ECE353_RTOS_Events;

/*******************************************************************************
* Macros used to define the system events
******************************************************************************/
//define macros used to test SW1, SW2, and SW3 events. To be used with FreeRTOS event groups
#define SW1_PRESSED    (1 << 0)  // Event bit for SW1 pressed
#define SW2_PRESSED    (1 << 1)  // Event bit for SW2 pressed
#define SW3_PRESSED    (1 << 2)  // Event bit for SW3 pressed




#endif // ECE353_FREERTOS

#endif // __RTOS_EVENTS_H__
