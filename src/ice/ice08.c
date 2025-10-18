/**
 * @file ice08.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE08)
#include "drivers.h"        
#include "rtos_events.h"
#include "task_buttons.h"
#include "task_lcd.h"
#include "task_joystick.h"

char APP_DESCRIPTION[] = "ECE353: ICE 08 - FreeRTOS LCD Gatekeeper";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
void task_system_control(void *pvParameters)
{
    (void)pvParameters; // Unused parameter
    lcd_msg_t lcd_msg;
    
    // Clear the LCD screen by sending message to gatekeeper
    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    xQueueSend(xQueue_LCD, &lcd_msg, portMAX_DELAY);
    
    // Draw the game board
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    xQueueSend(xQueue_LCD, &lcd_msg, portMAX_DELAY);
    
    // Initialize cursor position
    uint8_t cursor_row = 0;
    uint8_t cursor_col = 0;
    
    while(1)
    {
        // Send cursor draw command to gatekeeper
        lcd_msg.command = LCD_CMD_DRAW_CURSOR;
        lcd_msg.payload.battleship.row = cursor_row;
        lcd_msg.payload.battleship.col = cursor_col;
        lcd_msg.payload.battleship.border_color = BATTLESHIP_CURSOR_COLOR;
        lcd_msg.payload.battleship.fill_color = LCD_COLOR_BLACK;
        xQueueSend(xQueue_LCD, &lcd_msg, portMAX_DELAY);
        
        // Sleep for 100 ms
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Clear current cursor by redrawing original blue rectangle
        lcd_msg.command = LCD_CMD_DRAW_CURSOR;
        lcd_msg.payload.battleship.row = cursor_row;
        lcd_msg.payload.battleship.col = cursor_col;
        lcd_msg.payload.battleship.border_color = LCD_COLOR_BLUE;
        lcd_msg.payload.battleship.fill_color = LCD_COLOR_BLACK;
        xQueueSend(xQueue_LCD, &lcd_msg, portMAX_DELAY);
        
        // Update cursor position (move left to right, wrap to next row)
        cursor_col++;
        if (cursor_col >= 10) {
            cursor_col = 0;
            cursor_row++;
            if (cursor_row >= 10) {
                cursor_row = 0; // Restart from beginning
            }
        }
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

    rslt = lcd_initialize();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("LCD initialization failed!\n\r");
        for(int i = 0; i < 100000; i++) {}
        CY_ASSERT(0);
    }

    rslt = buttons_init_gpio();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Buttons initialization failed!\n\r");
        for(int i = 0; i < 100000; i++) {}
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

    EventGroupHandle_t ECE353_RTOS_Events = xEventGroupCreate();

    /* Initialize LCD resources */
    if (!task_lcd_init())
    {
        printf("Failed to initialize joystick task\n\r");
        for(int i = 0; i < 100000; i++) {}
       CY_ASSERT(0); // If the task initialization fails, assert
    }

    if(!task_button_init())
    {
        printf("Failed to initialize button task\n\r");
        for(int i = 0; i < 100000; i++) {}
        CY_ASSERT(0); // If the task initialization fails, assert
    }

    /* Start the buttons task*/
    xTaskCreate(
        task_buttons, 
        "Task Buttons", 
        configMINIMAL_STACK_SIZE, 
        NULL, 
        tskIDLE_PRIORITY + 1, 
        NULL
    );

    xTaskCreate(
        task_system_control, 
        "Task System Control", 
        configMINIMAL_STACK_SIZE*5, 
        NULL, 
        tskIDLE_PRIORITY + 1, 
        NULL
    );

    /* Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}
#endif
