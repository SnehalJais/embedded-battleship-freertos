/**
 * @file task_buttons.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef __TASK_BUTTONS_H__
 #define __TASK_BUTTONS_H__

 #include "main.h"

#ifdef ECE353_FREERTOS

 #include "drivers.h"
 #include "rtos_events.h"

 void task_buttons(void *arg);
 bool task_button_init(void);
 #endif

#endif // __TASK_BUTTONS_H__
