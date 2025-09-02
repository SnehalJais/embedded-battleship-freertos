/**
 * @file battleship.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-20
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef __BATTLESHIP_H__
 #define __BATTLESHIP_H__

 #include "main.h"

#ifdef ECE353_FREERTOS
#include "drivers.h"
#include "rtos_events.h"

#define BATTLESHIP_BOX_WIDTH   20
#define BATTLESHIP_BOX_HEIGHT  20
#define BATTLESHIP_BORDER_WIDTH 4

#define BATTLESHIP_PLAYER_0_COLOR LCD_COLOR_BLUE
#define BATTLESHIP_PLAYER_1_COLOR LCD_COLOR_RED
#define BATTLESHIP_CURSOR_COLOR LCD_COLOR_GREEN

#define BATTLE_SHIP_LEFT_MARGIN  10 
#define BATTLE_SHIP_TOP_MARGIN   20 

typedef enum {
    LCD_BOX_STATUS_HIT,
    LCD_BOX_STATUS_MISS,
    LCD_BOX_STATUS_EMPTY,
} battleship_box_status_t;

typedef struct{
    uint8_t row;            // Row (0-9)
    uint8_t col;            // Column (0-9)
    // ADD other fields as needed
} battleship_payload_t;

bool battleship_draw_game_board(uint8_t player_id);
bool battleship_draw_cursor(uint8_t col, uint8_t row);
bool battleship_clear_cursor(uint8_t col, uint8_t row, uint8_t player_id);

#endif // ECE353_FREERTOS

 #endif /* __BATTLESHIP_H__ */