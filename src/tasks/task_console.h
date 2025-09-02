/**
 * @file task_console.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#ifndef TASK_CONSOLE_H
#define TASK_CONSOLE_H

#include <stdint.h>
#include <stdarg.h>
#include "main.h"

#ifdef ECE353_FREERTOS
#include "drivers.h"
#include "rtos_events.h"
#include "cyhal_uart.h"

#define INT_PRIORITY_CONSOLE 5 // Priority for console tasks and events
#define CONSOLE_MAX_MESSAGE_LENGTH 64 
#define CONSOLE_QUEUE_LENGTH 10

// Data structure to hold console message data when using 
// FreeRTOS and double buffering
typedef struct {
    char *data; // Buffer to hold the input string
    uint8_t index;    // Current index in the buffer
} console_buffer_t;

// Global variables used for receiving data
extern console_buffer_t console_buffer1;
extern console_buffer_t console_buffer2;
extern console_buffer_t *produce_console_buffer;
extern console_buffer_t *consume_console_buffer;
extern TaskHandle_t TaskHandle_Console_Rx;

// Global variable for transmitting data
extern circular_buffer_t *circular_buffer_tx;
extern QueueHandle_t xQueue_Console_Tx;
extern TaskHandle_t TaskHandle_Console_Tx;

// Handle for the Console UART
extern cyhal_uart_t cy_retarget_io_uart_obj;

bool task_console_resources_init_rx(void);
bool task_console_resources_init_tx(void);
bool task_console_init(void);
void task_console_printf(char *str_ptr, ...);


#endif
#endif /* TASK_CONSOLE_H */