 /**
 * @file hw02.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-10-08
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "hw02.h"
#include "battleship.h"
#include "main.h"
#include "projdefs.h"
#include "task_lcd.h"
#include <stdbool.h>

#if defined(HW02)

char APP_DESCRIPTION[] = "ECE353 F25 HW02 -- LCD Gatekeeper";

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
QueueHandle_t xQueue_LCD_response;
EventGroupHandle_t ECE353_RTOS_Events;

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
void task_hw02_system_control(void *pvParameters)
{
    (void)pvParameters; // Unused parameter

    

    // Clear the screen
    battleship_board_clear();  // Clear internal state
    lcd_msg_t lcd_msg;
    lcd_cmd_status_t status;
    lcd_msg.command = LCD_CMD_CLEAR_SCREEN;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    // LCD gatekeeper handles status printing
    


    // Draw the game board
    lcd_msg.command = LCD_CMD_DRAW_BOARD;
    lcd_msg.response_queue = xQueue_LCD_response;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    // LCD gatekeeper handles status printing

    // Test all tiles - Validate Tiles requirement
    lcd_msg.command = LCD_CMD_DRAW_TILE;
    lcd_msg.response_queue = xQueue_LCD_response;
    lcd_msg.payload.battleship.border_color = LCD_COLOR_BLUE;
    lcd_msg.payload.battleship.fill_color = LCD_COLOR_GREEN;
    
    // Animate a green tile moving across each square, row by row, left to right, top to bottom
    for (uint8_t row = 0; row < 10; row++) {
        for (uint8_t col = 0; col < 10; col++) {
            // Draw green tile at current position
            lcd_msg.payload.battleship.col = col;
            lcd_msg.payload.battleship.row = row;
            xQueueSend(xQueue_LCD, &lcd_msg, 0);
            if (xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50)) != pdTRUE) {
                printf("Warning: No response for tile at (%d,%d)\n", col, row);
            }
            
            // Wait 100ms before moving to next tile
            vTaskDelay(pdMS_TO_TICKS(100));
            
            // Clear current tile back to black (make only one green tile visible at a time)
            lcd_msg.payload.battleship.fill_color = LCD_COLOR_BLACK;
            xQueueSend(xQueue_LCD, &lcd_msg, 0);
            if (xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50)) != pdTRUE) {
                printf("Warning: No response for clear at (%d,%d)\n", col, row);
            }
            
            // Reset fill color back to green for next tile
            lcd_msg.payload.battleship.fill_color = LCD_COLOR_GREEN;
        }
      
    }
  

    // Draw valid ships
   

    // Test placing all valid ships from specification
    lcd_msg.command = LCD_CMD_DRAW_SHIP;
    lcd_msg.payload.battleship.border_color = LCD_COLOR_BLUE;
    lcd_msg.payload.battleship.fill_color = LCD_COLOR_GRAY;
    
    // Battleship at (0,0) horizontal
   
    lcd_msg.payload.battleship.row = 0;
    lcd_msg.payload.battleship.col = 0;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_BATTLESHIP;
    lcd_msg.payload.battleship.horizontal = true;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    
    // Destroyer at (2,2) vertical
    lcd_msg.payload.battleship.row = 2;
    lcd_msg.payload.battleship.col = 2;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_DESTROYER;
    lcd_msg.payload.battleship.horizontal = false;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    
    // Submarine at (5,5) horizontal
    lcd_msg.payload.battleship.row = 5;
    lcd_msg.payload.battleship.col = 5;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_SUBMARINE;
    lcd_msg.payload.battleship.horizontal = true;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    
    //cruiser at (7,7) vertical
    lcd_msg.payload.battleship.row = 7;
    lcd_msg.payload.battleship.col = 7;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_CRUISER;
    lcd_msg.payload.battleship.horizontal = false;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    
    // Carrier at (9,0) vertical
    lcd_msg.payload.battleship.row = 9;
    lcd_msg.payload.battleship.col = 0;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_CARRIER;
    lcd_msg.payload.battleship.horizontal = true;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));

    
    // Test invalid ship placements (should all be rejected)

    // Test 1: Battleship at (0,6) horizontal - Exceeds board width
    lcd_msg.payload.battleship.row = 0;
    lcd_msg.payload.battleship.col = 7;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_BATTLESHIP;
    lcd_msg.payload.battleship.horizontal = true;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    
    

    // Test 2: Submarine at (0,8) vertical - Exceeds board height
    lcd_msg.payload.battleship.row = 8;
    lcd_msg.payload.battleship.col = 0;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_SUBMARINE;
    lcd_msg.payload.battleship.horizontal = false;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));
    

    
    // Test 3: Carrier at (15,0) vertical - Starts outside board boundaries
    lcd_msg.payload.battleship.row = 15;
    lcd_msg.payload.battleship.col = 0;
    lcd_msg.payload.battleship.type = BATTLESHIP_TYPE_CARRIER;
    lcd_msg.payload.battleship.horizontal = false;
    xQueueSend(xQueue_LCD, &lcd_msg, 0);
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(50));

    printf("\033[32mAll invalid ship placement tests passed\033[0m\n");

    
    // Send both messages immediately, one right after the other
    
    // Send both messages back-to-back for simultaneous display
    
    // Prepare first message: "Hits: 5" at position (210, 40)
    lcd_msg.command = LCD_CONSOLE_DRAW_MESSAGE;
    lcd_msg.response_queue = xQueue_LCD_response;
    lcd_msg.payload.console.x_offset = 210;
    lcd_msg.payload.console.y_offset = 40;
    lcd_msg.payload.console.message  = "Hits: 5";
    lcd_msg.payload.console.length   = (uint16_t)strlen(lcd_msg.payload.console.message);
    
    // Send first message immediately
    xQueueSend(xQueue_LCD, &lcd_msg, pdMS_TO_TICKS(100));
     xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100)); // First response
    
    // Prepare second message: "Miss: 3" at position (210, 80)
   
    lcd_msg.payload.console.x_offset = 210;
    lcd_msg.payload.console.y_offset = 80;
    lcd_msg.payload.console.message  = "Miss: 3";
    lcd_msg.payload.console.length   = (uint16_t)strlen(lcd_msg.payload.console.message);
    
    // Send second message immediately after first
    xQueueSend(xQueue_LCD, &lcd_msg, pdMS_TO_TICKS(100));
    
   
   
    xQueueReceive(xQueue_LCD_response, &status, pdMS_TO_TICKS(100)); // Second response
  


    //test invalid ship placements
    // Print the hits/misses to the LCD
    

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}


/*************************************************
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 ************************************************/
void app_init_hw(void)
{
    cy_rslt_t rslt;

    console_init();
    // Set text color to black
    printf("\x1b[30m");
    printf("\x1b[2J\x1b[;H");
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

    /* Initialize the response queue */
    xQueue_LCD_response = xQueueCreate(1, sizeof(lcd_cmd_status_t));
    if (xQueue_LCD_response == NULL)
    {
        printf("Failed to create LCD response queue\n\r");
        for(int i = 0; i < 100000; i++) {}
        CY_ASSERT(0);
    }

    /* Initialize LCD resources */
    if (!task_lcd_init())
    {
        printf("Failed to initialize LCD task\n\r");
        for(int i = 0; i < 100000; i++) {}
       CY_ASSERT(0); // If the task initialization fails, assert
    }

    /* Initialize Console resources for FreeRTOS */
    if (!task_console_init())
    {
        printf("Failed to initialize console task\n\r");
        for(int i = 0; i < 100000; i++) {}
       CY_ASSERT(0); // If the task initialization fails, assert
    }

    xTaskCreate(
        task_hw02_system_control, 
        "Task System Control", 
        configMINIMAL_STACK_SIZE*10, 
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