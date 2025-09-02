/**
 * @file task_console.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #include "main.h"

#ifdef ECE353_FREERTOS
#include "drivers.h"
#include "task_console.h"
#include "cyhal_uart.h"
/**
 * @brief
 * This file contains the implementation of the console transmit (Tx) task.
 * The task is responsible for sending characters to the UART.
 * 
 * Tasks can print messages by sending the string to task_console_tx() using
 * a FreeRTOS queue.
 *
 * task_console_tx() will add the characters to a circular buffer that is
 * accessed by the UART interrupt service routine (ISR).
 *
 */

/* ADD CODE*/
/* Global Variables */


/**
 * @brief 
 * This task is used to transmit characters to the UART
 * @param param 
 */
void task_console_tx(void *param)
{
    (void)param; // Unused parameter
    console_buffer_t tx_msg;

    while (1)
    {
        /* ADD CODE */
    }
}

/**
 * @brief 
 * This function initializes the resources for the console Tx task. 
 * @return true  if initialization is successful
 * @return false if initialization fails
 * @return false 
 */
bool task_console_resources_init_tx(void)
{
    BaseType_t rslt;

    /* ADD CODE */

    if (rslt != pdPASS)
    {
        return false; // Initialization failed
    }

    return true; // Resources initialized successfully
}

/**
 * @brief
 * This function sends formatted messages to task_console_tx. It acts as a wrapper around the FreeRTOS queue
 * to send messages so other tasks can use it easily.
 *
 * Example usage: 
 * task_console_printf("Send Message");
 * task_console_printf("Formatted number: %d", 42);
 *
 * @param str_ptr Pointer to the format string.
 * @param ...     Additional arguments for formatting.
 */
void task_console_printf(char *str_ptr, ...)
{
    console_buffer_t console_buffer;
    char *message_buffer;
    char *task_name;
    uint32_t length = 0;
    va_list args;

    /* ADD CODE */
    /* Allocate the message buffer */

    if (message_buffer)
    {
        va_start(args, str_ptr);
        task_name = pcTaskGetName(xTaskGetCurrentTaskHandle());
        length = snprintf(message_buffer, CONSOLE_MAX_MESSAGE_LENGTH, "%-16s : ",
                              task_name);

        vsnprintf((message_buffer + length), (CONSOLE_MAX_MESSAGE_LENGTH - length),
                  str_ptr, args);

        va_end(args);

        /* ADD CODE */
        /* Initialize the console buffer */

        /* ADD CODE */
        /* The receiver task is responsible to free the memory from here on */

    }
    else
    {
        /* pvPortMalloc failed. Handle error */
        CY_ASSERT(0); // Halt the processor
    }
}
#endif