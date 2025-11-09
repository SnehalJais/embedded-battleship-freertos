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

// allocate space for the transmit queue
QueueHandle_t xQueue_Console_Tx;
TaskHandle_t TaskHandle_Console_Tx;
// allocate space for the circular buffer
circular_buffer_t *circular_buffer_tx;


/**
 * @brief
 * This task is used to transmit characters to the UART
 * @param param
 */
void task_console_tx(void *param)
{
    (void)param; // Unused parameter
    console_buffer_t *tx_msg;

    while (1)
    {
        /* ADD CODE */

        // wait for console_buffer_t messages from the queue
        xQueueReceive(xQueue_Console_Tx, &tx_msg, portMAX_DELAY);

        // A for loop that examines the message and adds each byte to the circular buffer
        for (int i = 0; tx_msg->data[i] != '\0'; i++)
        {
            // if the circular buffer is full, vTaskDelay(5)
            while (circular_buffer_get_num_bytes(circular_buffer_tx) == circular_buffer_tx->max_size)
            {
                vTaskDelay(5);
            }

            taskENTER_CRITICAL();
            // add the next byte to the CB.
            circular_buffer_add(circular_buffer_tx, tx_msg->data[i]);

            taskEXIT_CRITICAL();
        }

        // enable the transmit empty interrupt
        cyhal_uart_enable_event(&cy_retarget_io_uart_obj, CYHAL_UART_IRQ_TX_EMPTY, INT_PRIORITY_CONSOLE, true);

        // free the data that was sent from console_buffer_t
        vPortFree(tx_msg->data);
        vPortFree(tx_msg);
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
    BaseType_t rslt = pdPASS;

    /* ADD CODE */
    // initialize the TX FREERTOS queue
    xQueue_Console_Tx = xQueueCreate(10, sizeof(console_buffer_t *));
    if (xQueue_Console_Tx == NULL)
    {
        rslt = pdFAIL;
    }

    //init the circular buffer
    circular_buffer_tx = circular_buffer_init(CONSOLE_BUFFER_SIZE);
    if (circular_buffer_tx == NULL)
    {
        rslt = pdFAIL;
    }

    // Create FreeRTOS Tx Task (gatekeeper)
    if (rslt == pdPASS)
    {
        rslt = xTaskCreate(task_console_tx,
                           "Console_Tx",
                           256,
                           NULL,
                           INT_PRIORITY_CONSOLE, 
                           &TaskHandle_Console_Tx);
    }

    return (rslt == pdPASS); // Resources initialized successfully
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
    console_buffer_t *console_buffer;
    char *message_buffer;
    char *task_name;
    uint32_t length = 0;
    va_list args;

    /* ADD CODE */
    /* Allocate memory for the message buffer */
    /* If the message buffer is NULL, this means the heap is exhausted */
    /* We will placed the calling task into the blocked state which should */
    /* allow other tasks (task_console_tx) to run and free up memory after */
    /* it has transmitted previous messages. Once memory is available, the */
    /* calling task will be unblocked and can try to allocate memory again. */
    do {
        message_buffer = pvPortMalloc(CONSOLE_MAX_MESSAGE_LENGTH);
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay to avoid busy waiting
    } while (message_buffer == NULL);
    
    console_buffer = pvPortMalloc(sizeof(console_buffer_t));

    if (message_buffer && console_buffer)
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
        console_buffer->data = message_buffer;
        console_buffer->index = strlen(message_buffer);

        /* ADD CODE */
        /* The receiver task is responsible to free the memory from here on */
        xQueueSendToBack(xQueue_Console_Tx, &console_buffer, portMAX_DELAY);

    }
    else
    {
        /* pvPortMalloc failed. Handle error */
        if (message_buffer) vPortFree(message_buffer);
        if (console_buffer) vPortFree(console_buffer);
        CY_ASSERT(0); // Halt the processor
    }
}
#endif