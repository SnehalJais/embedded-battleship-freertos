/**
 * @file main.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include "cybsp.h"
#include "cyhal_hw_types.h"
#include "cyhal_gpio.h"
#include "cyhal_timer.h"
#include "cyhal_pwm.h"
#include "cyhal_adc.h"
#include "cyhal_uart.h"
#include "cyhal_i2c.h"
#include "cyhal_spi.h"
#include "cy_retarget_io.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/* Uncomment the line below to enable FreeRTOS in your applications*/
//#define ECE353_FREERTOS

#if defined(ECE353_FREERTOS)
/* FreeRTOS Includes */
#include <FreeRTOS.h>
#include <event_groups.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
#endif

/* This macro is used to determine if we are building an executable for example code or ICE code.
 * Only a single line should be uncommented at one time.  
*/
//#define EX01         /* Example Code C Basics*/
//#define EX02         /* Example Code LCD */
//#define EX03         /* Example Code Timer Interrupts */
//#define EX04         /* Example Code Timer Interrupts */
//#define EX05         /* Example Code FreeRTOS Tasks */
//#define EX06         /* Example Code FreeRTOS Tasks */
//#define EX08         /* Example Code FreeRTOS LCD Gatekeeper */

#define ICE01       /* In-Class Exercise -- Memory Mapped IO - GPIO */
//#define ICE02       /* In-Class Exercise -- LCD 8080 Driver */
//#define ICE03       /* In-Class Exercise -- Timer Interrupts/Debounce Buttons */
//#define ICE04       /* In-Class Exercise -- PWM Buzzer */
//#define ICE05       /* In-Class Exercise -- FreeRTOS Event Groups */
//#define ICE06       /* In-Class Exercise -- FreeRTOS Queues */
//#define ICE08       /* In-Class Exercise -- FreeRTOS LCD Gatekeeper */
//#define ICE09       /* In-Class Exercise -- FreeRTOS UART Rx IRQ + Buffering  */
//#define ICE10       /* In-Class Exercise -- FreeRTOS UART Tx IRQ + Buffering  */

//#define HW01
//#define HW02
//#define HW03

extern char NAME[];
extern char APP_DESCRIPTION[];

/**
 * @brief 
 * This function will initialize all of the hardware resources for
 * the ICE.  
 * 
 * This function is implemented in the iceXX.c file for the ICE you are
 * working on.
 */
void app_init_hw(void);

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief 
 * This function implements the behavioral requirements for the ICE
 * 
 * This function is implemented in the iceXX.c file for the ICE you are 
 * working on.
 */
void app_main(void);

#endif