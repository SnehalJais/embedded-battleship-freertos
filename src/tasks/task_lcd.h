/**
 * @file task_lcd.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-18
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef TASK_LCD_H
#define TASK_LCD_H

#include "main.h"

#ifdef ECE353_FREERTOS
#include "drivers.h"
#include "rtos_events.h"
#include "battleship.h"
#include "lcd_console.h"



typedef enum {
    LCD_CMD_CLEAR_SCREEN,
    LCD_CMD_DRAW_BOARD,
    LCD_CMD_DRAW_TILE,
    LCD_CMD_DRAW_SHIP,
    LCD_CONSOLE_DRAW_MESSAGE,
} lcd_command_t;

typedef enum
{
    LCD_CMD_STATUS_SUCCESS,
    LCD_CMD_STATUS_ERROR,
} lcd_cmd_status_t;

typedef struct
{
    lcd_command_t command;        // Command to execute
    QueueHandle_t response_queue; // Queue for sending responses
    union
    {
        lcd_console_payload_t console;   // Console payload
        battleship_payload_t battleship; // Battleship game payload
    } payload;                           // Payload for the command
} lcd_msg_t;

/* FreeRTOS Queue for LCD messages */
extern QueueHandle_t xQueue_LCD;

/* LCD Task */
void task_lcd(void *pvParameters);

/* LCD Task Initialization */
bool task_lcd_init(void);

#endif
#endif /* TASK_LCD_H */