/**
 * @file task_joystick.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-14
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __TASK_JOYSTICK_H__
#define __TASK_JOYSTICK_H__
#include "main.h"
#include "drivers.h"
#include "rtos_events.h"
#include <complex.h>

#ifdef ECE353_FREERTOS
extern QueueHandle_t Queue_Joystick;

void task_joystick(void *arg);

bool task_joystick_init(void);

#endif
#endif /* __TASK_JOYSTICK_H__ */