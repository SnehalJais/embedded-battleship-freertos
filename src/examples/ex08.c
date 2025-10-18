/**
 * @file ex03.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-06-30
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "main.h"

#if defined(EX08)
#include "drivers.h"
#include "rtos_events.h"
#include "task_buttons.h"
#include "task_lcd.h"
#include "task_joystick.h"

char APP_DESCRIPTION[] = "ECE353: Example 08 - FreeRTOS LCD Gatekeeper";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
EventGroupHandle_t ECE353_RTOS_Events;

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
void task_system_control(void *pvParameters)
{
    (void)pvParameters; // Unused parameter

    int8_t row = 0;
    int8_t col = 0;
    int8_t button_presses = 0;
    EventBits_t events;

    lcd_msg_t lcd_msg;

    // Clear the screen
    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    xQueueSend(xQueue_LCD, &lcd_msg, portMAX_DELAY);

    // Request 20 bytes to store a string
    lcd_msg.payload.console.message = pvPortMalloc(20 * sizeof(char));
    if (lcd_msg.payload.console.message == NULL)
    {
        // Failed to allocate memory
        printf("Failed to allocate memory for LCD message\n\r");
        CY_ASSERT(0);
    }

    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.payload.console.x_offset = 160;
    snprintf(lcd_msg.payload.console.message, 20, "SW1: %d", button_presses);
    lcd_msg.payload.console.length = strlen(lcd_msg.payload.console.message);
    xQueueSend(xQueue_LCD, &lcd_msg, portMAX_DELAY);

    // Print the number of button presses to the LCD

    while (1)
    {
        // Wait for SW1 events
        // Wait for an event to occur
        events = xEventGroupWaitBits(
            ECE353_RTOS_Events,
            SW1_PRESSED,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);

        if (events & SW1_PRESSED)
        {
            button_presses++;
            printf("SW1 Pressed - Count: %d\n\r", button_presses);

            // Request 20 bytes to store a string
            lcd_msg.payload.console.message = pvPortMalloc(20 * sizeof(char));
            if (lcd_msg.payload.console.message == NULL)
            {
                // Failed to allocate memory
                printf("Failed to allocate memory for LCD message\n\r");
                CY_ASSERT(0);
            }

            lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
            lcd_msg.payload.console.x_offset = 160;
            snprintf(lcd_msg.payload.console.message, 20, "SW1: %d", button_presses);
            lcd_msg.payload.console.length = strlen(lcd_msg.payload.console.message);
            xQueueSend(xQueue_LCD, &lcd_msg, portMAX_DELAY);
        }

        // Update the button press count

        // Print the number of button presses to the LCD
    }
}

/**
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 */
void app_init_hw(void)
{
    cy_rslt_t rslt;

    console_init();
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    rslt = buttons_init_gpio();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Buttons initialization failed!\n\r");
        for (int i = 0; i < 100000; i++)
        {
        }
        CY_ASSERT(0);
    }

    rslt = lcd_initialize();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("LCD initialization failed!\n\r");
        for (int i = 0; i < 100000; i++)
        {
        }
        CY_ASSERT(0);
    }
}

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void)
{

    /* Register the tasks with FreeRTOS*/

    ECE353_RTOS_Events = xEventGroupCreate();

    /* Initialize the Button Task resources */
    if (!task_button_init())
    {
        printf("Failed to initialize button task\n\r");
        for (int i = 0; i < 100000; i++)
        {
        }
        CY_ASSERT(0); // If the task initialization fails, assert
    }

    /* Initialize LCD resources */
    if (!task_lcd_init())
    {
        printf("Failed to initialize lcd task\n\r");
        for (int i = 0; i < 100000; i++)
        {
        }
        CY_ASSERT(0); // If the task initialization fails, assert
    }

    xTaskCreate(
        task_system_control,
        "Task System Control",
        configMINIMAL_STACK_SIZE * 10,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    /* Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}
#endif