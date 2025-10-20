/**
 * @file task_console_rx.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-21
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
 * This file contains the implementation of the console receive (Rx) task.
 * The task is responsible for processing incoming console commands and
 * controlling the state of the LEDs accordingly.
 *
 * The task uses a double buffer to process the incoming console commands.
 * The supported commands will be "RED_ON" and "RED_OFF" to control the red LED.
 */

/* ADD CODE */
/* Global Variables */
console_buffer_t console_buffer1;
console_buffer_t console_buffer2;

// Pointers to the produce and consume buffers
console_buffer_t *produce_console_buffer;
console_buffer_t *consume_console_buffer;

// Allocate task handle for the console Rx task
TaskHandle_t TaskHandle_Console_Rx;

/**
 * @brief
 * This function is the bottom half task for receiving console input.
 *
 * It waits for a task notification from the ISR indicating that a new
 * command has been received. The task then processes the command and
 * controls the state of the LEDs accordingly.
 *
 * @param param Unused parameter
 */
void task_console_rx(void *param)
{
    (void)param; // Unused parameter
    //printf("DEBUG: Console RX task started and waiting for notifications\n\r");
    
    while (1)
    {
        /* ADD CODE */
        //wait indefinitely for a task notification
        // printf("DEBUG: Console RX task waiting for notification...\n\r");
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Debug: Print what command was received
        printf("DEBUG: Received command: '%s'\n\r", consume_console_buffer->data);

        //processthe data pointed to by the console buffer pointer
        //if "RED_ON" turn on red LED
        if (strcmp(consume_console_buffer->data, "RED_ON") == 0)
        {
            printf("Turning on RED LED\r\n");
            leds_set_state(LED_RED, LED_ON);
            
        }
        //if "RED_OFF" turn off the LED
        else if (strcmp(consume_console_buffer->data, "RED_OFF") == 0)
        {
            printf("Turning off RED LED\r\n");
            leds_set_state(LED_RED, LED_OFF);   
        }


        

        
         //commands ignored
        

    }
}

/**
 * @brief
 * This function initializes the resources for the console Rx task.
 * @return true if resources were initialized successfully
 * @return false if resource initialization failed
 */
bool task_console_resources_init_rx(void)
{
    BaseType_t rslt;

    /* ADD CODE */
    // allocate an array from heap for the two console buffers
    console_buffer1.data = (char *)pvPortMalloc(CONSOLE_MAX_MESSAGE_LENGTH);
    console_buffer2.data = (char *)pvPortMalloc(CONSOLE_MAX_MESSAGE_LENGTH);

    produce_console_buffer = &console_buffer1;
    consume_console_buffer = &console_buffer2;

    // set initial lengths to zero
    produce_console_buffer->index = 0;
    consume_console_buffer->index = 0;

    // Check if memory allocation was successful
    if (console_buffer1.data == NULL || console_buffer2.data == NULL)
    {
        return false; // Memory allocation failed
    }

    // Create the console Rx task
    rslt = xTaskCreate(
        task_console_rx,
        "Console Rx",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        &TaskHandle_Console_Rx);

    if (rslt != pdPASS)
    {
        printf("ERROR: Console RX task creation failed!\n\r");
        return false; // Task creation failed
    }

    //printf("DEBUG: Console RX task created successfully\n\r");
    return true; // Resources initialized successfully
}
#endif