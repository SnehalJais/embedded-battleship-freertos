 /**
 * @file hw02.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-10-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef __HW02_H__
 #define __HW02_H__

 #include "main.h"

 #if defined(HW02)

#include "drivers.h"
#include "rtos_events.h"
#include "task_lcd.h"
#include "task_buttons.h"
#include "task_joystick.h"

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
void task_hw02_system_control(void *pvParameters);


 #endif

 #endif /* __HW02_H__ */