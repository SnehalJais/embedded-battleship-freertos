/**
 * @file task_buzzer.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-13
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef __TASK_BUZZER_H__
 #define __TASK_BUZZER_H__

 #include "main.h"

 #ifdef ECE353_FREERTOS

 #include "drivers.h"
 #include "rtos_events.h"

 void task_buzzer(void *arg);
 #endif

 #endif // __TASK_BUZZER_H__    
