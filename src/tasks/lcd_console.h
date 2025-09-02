/**
 * @file lcd_console.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef LCD_CONSOLE_H
 #define LCD_CONSOLE_H

 #include "main.h"

 #ifdef ECE353_FREERTOS

 #include "drivers.h"
#include "rtos_events.h"

 #include <stdint.h>
 #include <stdbool.h>

 #define LCD_CONSOLE_MAX_LINES 6
 #define LCD_CONSOLE_LINE_HEIGHT 40 // Height of each line in pixels

 typedef struct {
     uint16_t x_offset;
     uint16_t y_offset;
     char *message;
     uint16_t length;
 } lcd_console_payload_t;

 bool lcd_console_draw_string(lcd_console_payload_t *payload, uint8_t line);

 #endif /* ECE353_FREERTOS */

 #endif /* LCD_CONSOLE_H */
