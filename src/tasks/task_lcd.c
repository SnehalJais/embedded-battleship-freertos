/**
 * @file task_lcd.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-18
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "task_lcd.h"

#ifdef ECE353_FREERTOS
/* FreeRTOS Queue for LCD messages */
QueueHandle_t xQueue_LCD;

/* LCD Task */
void task_lcd(void *pvParameters)
{
    lcd_msg_t lcd_msg;
    (void)pvParameters; // Unused parameter

    while (1)
    {
        xQueueReceive(xQueue_LCD, &lcd_msg, portMAX_DELAY);
        lcd_cmd_status_t status;

        // Process the received LCD message
        switch (lcd_msg.command)
        {
        case LCD_CMD_CLEAR_SCREEN:
        {
            lcd_clear_screen(LCD_COLOR_BLACK);
            status = LCD_CMD_STATUS_SUCCESS;
             
            //All requests to the LCD gatekeeper will wait a maximum of 50mS for a response
            if (lcd_msg.response_queue != NULL)
            {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            if (status == LCD_CMD_STATUS_SUCCESS)
            {
                // Successfully cleared screen
                printf("Screen cleared successfully\n");
            }
            else
            {
                // Failed to clear screen
                printf("Failed to clear screen\n");
            }
            break;
        }

        case LCD_CONSOLE_DRAW_MESSAGE:
        {
            printf("Drawing console message: '%s' at (%d, %d)\n", 
                   lcd_msg.payload.console.message, 
                   lcd_msg.payload.console.x_offset, 
                   lcd_msg.payload.console.y_offset);
            
            // Map y_offset to line number to avoid overwriting
            uint16_t y = lcd_msg.payload.console.y_offset;
            uint8_t line = (uint8_t)(y / LCD_CONSOLE_LINE_HEIGHT);
            if (line >= LCD_CONSOLE_MAX_LINES) line = LCD_CONSOLE_MAX_LINES - 1;
            
            printf("Console draw: '%s' at x=%u line=%u\n", 
                   lcd_msg.payload.console.message,
                   lcd_msg.payload.console.x_offset,
                   line);
            
            if (!lcd_console_draw_string(&lcd_msg.payload.console, line))
            {
                // Failed to draw console message
                printf("Failed to draw console message: %s\n", lcd_msg.payload.console.message);
                status = LCD_CMD_STATUS_ERROR;
            }
            else
            {
                printf("Successfully drew console message\n");
                status = LCD_CMD_STATUS_SUCCESS;
            }
            
            // Send response if queue is provided
            if (lcd_msg.response_queue != NULL) {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            break;
        }
        case LCD_CMD_DRAW_BOARD:
        {
            // Clear screen first to ensure clean drawing
            lcd_clear_screen(LCD_COLOR_BLACK);
            
            // Use nested loops to iterate through rows and columns
            for (uint8_t row = 0; row < 10; row++)
            {
                for (uint8_t col = 0; col < 10; col++)
                {
                    // Calculate the x,y coordinates for each rectangle
                    uint16_t x = BATTLE_SHIP_LEFT_MARGIN + (col * BATTLESHIP_BOX_WIDTH);
                    uint16_t y = BATTLE_SHIP_TOP_MARGIN + (row * BATTLESHIP_BOX_HEIGHT);

                    // Draw the outer blue rectangle (border)
                    lcd_draw_rectangle(x, y, BATTLESHIP_BOX_WIDTH, BATTLESHIP_BOX_HEIGHT, LCD_COLOR_BLUE, false);
                    
                    // Draw the inner black rectangle (fill) to create border effect
                    lcd_draw_rectangle(x + BATTLESHIP_BORDER_WIDTH/2, 
                                     y + BATTLESHIP_BORDER_WIDTH/2, 
                                     BATTLESHIP_BOX_WIDTH - BATTLESHIP_BORDER_WIDTH, 
                                     BATTLESHIP_BOX_HEIGHT - BATTLESHIP_BORDER_WIDTH, 
                                     LCD_COLOR_BLACK, false);
                }
            }
            
            status = LCD_CMD_STATUS_SUCCESS;
            
            // Send response if queue is provided
            if (lcd_msg.response_queue != NULL) {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            break;
        }

        case LCD_CMD_DRAW_TILE:
        {
            // Calculate coordinates for the tile
            uint16_t x = BATTLE_SHIP_LEFT_MARGIN + (lcd_msg.payload.battleship.col * BATTLESHIP_BOX_WIDTH);
            uint16_t y = BATTLE_SHIP_TOP_MARGIN + (lcd_msg.payload.battleship.row * BATTLESHIP_BOX_HEIGHT);
            
            // Always draw the blue border first to maintain board structure
            lcd_draw_rectangle(x, y, BATTLESHIP_BOX_WIDTH, BATTLESHIP_BOX_HEIGHT, LCD_COLOR_BLUE, false);
            
            // Then draw the inner rectangle with the specified fill color
            lcd_draw_rectangle(x + BATTLESHIP_BORDER_WIDTH/2, 
                             y + BATTLESHIP_BORDER_WIDTH/2, 
                             BATTLESHIP_BOX_WIDTH - BATTLESHIP_BORDER_WIDTH, 
                             BATTLESHIP_BOX_HEIGHT - BATTLESHIP_BORDER_WIDTH, 
                             lcd_msg.payload.battleship.fill_color, false);
            
            status = LCD_CMD_STATUS_SUCCESS;
            
            // Send response if queue is provided
            if (lcd_msg.response_queue != NULL) {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            break;
        }

        case LCD_CMD_DRAW_SHIP:
        {
            // Gatekeeper validates coordinates and draws ship directly
            uint8_t ship_length = battleship_get_ship_length(lcd_msg.payload.battleship.type);
            bool valid = true;
            
            // Boundary validation
            if (lcd_msg.payload.battleship.horizontal) {
                if (lcd_msg.payload.battleship.col + ship_length > 10) {
                    printf("Invalid ship placement: exceeds horizontal bounds\n");
                    valid = false;
                }
            } else {
                if (lcd_msg.payload.battleship.row + ship_length > 10) {
                    printf("Invalid ship placement: exceeds vertical bounds\n");
                    valid = false;
                }
            }
            
            // Check if coordinates are within board
            if (lcd_msg.payload.battleship.col >= 10 || lcd_msg.payload.battleship.row >= 10) {
                printf("Invalid ship placement: starting coordinates out of bounds\n");
                valid = false;
            }
            
            if (valid) {
                // Draw each space of the ship
                for (uint8_t i = 0; i < ship_length; i++) {
                    uint8_t draw_col = lcd_msg.payload.battleship.col;
                    uint8_t draw_row = lcd_msg.payload.battleship.row;
                    
                    if (lcd_msg.payload.battleship.horizontal) {
                        draw_col += i;
                    } else {
                        draw_row += i;
                    }
                    
                    lcd_coord_t coord;
                    if (battleship_get_box_coordinates(&coord, draw_col, draw_row)) {
                        // Draw the outer rectangle (border)
                        lcd_draw_rectangle(coord.x, coord.y, BATTLESHIP_BOX_WIDTH, BATTLESHIP_BOX_HEIGHT, 
                                         lcd_msg.payload.battleship.border_color, false);
                        
                        // Draw the inner rectangle (fill) - use false for border-only to prevent overflow
                        lcd_draw_rectangle(coord.x + BATTLESHIP_BORDER_WIDTH/2, 
                                         coord.y + BATTLESHIP_BORDER_WIDTH/2, 
                                         BATTLESHIP_BOX_WIDTH - BATTLESHIP_BORDER_WIDTH, 
                                         BATTLESHIP_BOX_HEIGHT - BATTLESHIP_BORDER_WIDTH, 
                                         lcd_msg.payload.battleship.fill_color, false);
                    }
                }
                status = LCD_CMD_STATUS_SUCCESS;
            } else {
                // Invalid coordinates/dimensions
                status = LCD_CMD_STATUS_ERROR;
            }
            
            // Send response back to sender
            if (lcd_msg.response_queue != NULL) {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            
            break;
        }

        default:
        {
            break;
        }
        }
    }
}

/* LCD Task Initialization */
bool task_lcd_init(void)
{

    BaseType_t result;
    xQueue_LCD = xQueueCreate(10, sizeof(lcd_msg_t));
    if (xQueue_LCD == NULL)
    {
        return false;
    }

    result = xTaskCreate(
        task_lcd,                     // Task function
        "LCD Task",                   // Task name
        1024,                         // Stack size (≥ 1024 bytes as per spec)
        NULL,                         // Task parameters
        2,                            // Task priority (priority 2 as per spec)
        NULL                          // Task handle
    );

    if (result != pdPASS)
    {
        return false;
    }

    return true;
}
#endif