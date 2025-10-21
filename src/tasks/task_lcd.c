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
#include "battleship.h"

#ifdef ECE353_FREERTOS
/* FreeRTOS Queue for LCD messages */
QueueHandle_t xQueue_LCD;

// Helper to convert ship type enum to string
static const char *battleship_type_to_str(battleship_type_t type)
{
    switch (type)
    {
    case BATTLESHIP_TYPE_CARRIER:
        return "carrier";
    case BATTLESHIP_TYPE_BATTLESHIP:
        return "battleship";
    case BATTLESHIP_TYPE_CRUISER:
        return "cruiser";
    case BATTLESHIP_TYPE_SUBMARINE:
        return "submarine";
    case BATTLESHIP_TYPE_DESTROYER:
        return "destroyer";
    default:
        return "unknown";
    }
}

/* LCD Task */
void task_lcd(void *pvParameters)
{
    lcd_msg_t lcd_msg;
    (void)pvParameters; // Unused parameter

    while (1)
    {
        // Wait for an LCD message from the queue
        xQueueReceive(xQueue_LCD, &lcd_msg, portMAX_DELAY);
        lcd_cmd_status_t status;

        // Process the received LCD message
        switch (lcd_msg.command)
        {
            // Handle different LCD commands
        case LCD_CMD_CLEAR_SCREEN: // Clear the LCD screen
        {
            lcd_clear_screen(LCD_COLOR_BLACK);
            status = LCD_CMD_STATUS_SUCCESS; // Assume success

            // All requests to the LCD gatekeeper will wait a maximum of 50mS for a response
            if (lcd_msg.response_queue != NULL)
            {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            break;
        }

        case LCD_CONSOLE_DRAW_MESSAGE: // Draw a message on the LCD console
        {
            // Map y_offset to line number to avoid overwriting
            uint16_t y = lcd_msg.payload.console.y_offset;
            uint8_t line = (uint8_t)(y / LCD_CONSOLE_LINE_HEIGHT);
            // Clamp line number to valid range
            if (line >= LCD_CONSOLE_MAX_LINES)
                line = LCD_CONSOLE_MAX_LINES - 1;

            // Draw the console message
            if (!lcd_console_draw_string(&lcd_msg.payload.console, line))
            {
                // Failed to draw console message
                printf("Failed to draw console message: %s\n", lcd_msg.payload.console.message);
                status = LCD_CMD_STATUS_ERROR;
            }
            else
            {
                // Successfully drew console message
                status = LCD_CMD_STATUS_SUCCESS;
            }

            // Send response if queue is provided
            if (lcd_msg.response_queue != NULL)
            {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            break;
        }
        case LCD_CMD_DRAW_BOARD:
        {
            // Draw the Battleship game board
            if (battleship_draw_game_board())
            {
                printf("Game board drawn successfully\n");
                status = LCD_CMD_STATUS_SUCCESS; // Assume success
            }
            else
            {
                printf("Failed to draw game board\n");
                status = LCD_CMD_STATUS_ERROR; // Failed to draw board
            }

            // Send response if queue is provided
            if (lcd_msg.response_queue != NULL)
            {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            break;
        }

        case LCD_CMD_DRAW_TILE:
        {
            // Gatekeeper draws tile directly based on provided coordinates
            if (lcd_msg.payload.battleship.col >= 10 || lcd_msg.payload.battleship.row >= 10)
            {
                status = LCD_CMD_STATUS_ERROR; // Invalid coordinates
                // Send response if queue is provided
                if (lcd_msg.response_queue)
                    xQueueSend(lcd_msg.response_queue, &status, 0);
                break;
            }
            // Calculate coordinates for the tile
            uint16_t x = BATTLE_SHIP_LEFT_MARGIN + (lcd_msg.payload.battleship.col * BATTLESHIP_BOX_WIDTH);
            uint16_t y = BATTLE_SHIP_TOP_MARGIN + (lcd_msg.payload.battleship.row * BATTLESHIP_BOX_HEIGHT);

            // Always draw the blue border first to maintain board structure
            lcd_draw_rectangle(x, y, BATTLESHIP_BOX_WIDTH, BATTLESHIP_BOX_HEIGHT, LCD_COLOR_BLUE, false);

            // Then draw the inner rectangle with the specified fill color
            lcd_draw_rectangle(x + BATTLESHIP_BORDER_WIDTH / 2,
                               y + BATTLESHIP_BORDER_WIDTH / 2,
                               BATTLESHIP_BOX_WIDTH - BATTLESHIP_BORDER_WIDTH,
                               BATTLESHIP_BOX_HEIGHT - BATTLESHIP_BORDER_WIDTH,
                               lcd_msg.payload.battleship.fill_color, false);

            status = LCD_CMD_STATUS_SUCCESS;
            // Send response if queue is provided
            if (lcd_msg.response_queue != NULL)
            {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }
            break;
        }

        case LCD_CMD_DRAW_SHIP:
        {
            // Gatekeeper validates coordinates and draws ship directly
            uint8_t ship_length = battleship_get_ship_length(lcd_msg.payload.battleship.type);
            // Assume valid until proven otherwise
            bool valid = true;

            // Boundary validation
            if (lcd_msg.payload.battleship.horizontal)
            {
                // Check if ship exceeds board width
                if (lcd_msg.payload.battleship.col + ship_length > 10)
                {
                    printf("Correctly detected invalid ship placement (too far right)\n");
                    valid = false;
                }
            }
            else
            {
                // Check if ship exceeds board height
                if (lcd_msg.payload.battleship.row + ship_length > 10)
                {
                    printf("Correctly detected invalid ship placement (too far down)\n");
                    valid = false;
                }
            }

            // Check if coordinates are within board
            if (lcd_msg.payload.battleship.col >= 10 || lcd_msg.payload.battleship.row >= 10)
            {
                printf("Correctly detected invalid ship placement (invalid coordinates)\n");
                valid = false;
            }

            if (valid)
            {
                // Draw each space of the ship
                for (uint8_t i = 0; i < ship_length; i++)
                {
                    uint8_t draw_col = lcd_msg.payload.battleship.col;
                    uint8_t draw_row = lcd_msg.payload.battleship.row;

                    // Increment column or row based on orientation
                    if (lcd_msg.payload.battleship.horizontal)
                    {
                        draw_col += i;
                    }
                    else
                    {
                        draw_row += i;
                    }

                    lcd_coord_t coord;
                    // Get the top-left coordinates for this box
                    if (battleship_get_box_coordinates(&coord, draw_col, draw_row))
                    {
                        // Draw the outer rectangle (border)
                        lcd_draw_rectangle(coord.x, coord.y, BATTLESHIP_BOX_WIDTH, BATTLESHIP_BOX_HEIGHT,
                                           lcd_msg.payload.battleship.border_color, false);

                        // Draw the inner rectangle (fill) - use false for border-only to prevent overflow
                        lcd_draw_rectangle(coord.x + BATTLESHIP_BORDER_WIDTH / 2,
                                           coord.y + BATTLESHIP_BORDER_WIDTH / 2,
                                           BATTLESHIP_BOX_WIDTH - BATTLESHIP_BORDER_WIDTH,
                                           BATTLESHIP_BOX_HEIGHT - BATTLESHIP_BORDER_WIDTH,
                                           lcd_msg.payload.battleship.fill_color, false);
                    }
                }
                // Successfully drew the ship
                printf("Drew %s successfully at (%d, %d)\n",
                       battleship_type_to_str(lcd_msg.payload.battleship.type),
                       lcd_msg.payload.battleship.row,
                       lcd_msg.payload.battleship.col);
                status = LCD_CMD_STATUS_SUCCESS;
            }
            else
            {
                // Invalid coordinates/dimensions
                status = LCD_CMD_STATUS_ERROR;
            }

            // Send response back to sender
            if (lcd_msg.response_queue != NULL)
            {
                xQueueSend(lcd_msg.response_queue, &status, 0);
            }

            break;
        }

        default:
        {
            // Unknown command
            printf("Unknown LCD command %d\n", lcd_msg.command);
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
        task_lcd,   // Task function
        "LCD Task", // Task name
        1024,       // Stack size (≥ 1024 bytes as per spec)
        NULL,       // Task parameters
        2,          // Task priority (priority 2 as per spec)
        NULL        // Task handle
    );

    if (result != pdPASS)
    {
        return false;
    }

    return true;
}
#endif