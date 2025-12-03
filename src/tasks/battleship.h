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

#define BATTLESHIP_BOX_WIDTH 20
#define BATTLESHIP_BOX_HEIGHT 20
#define BATTLESHIP_BORDER_WIDTH 4

#define BATTLESHIP_PLAYER_0_COLOR LCD_COLOR_BLUE
#define BATTLESHIP_PLAYER_1_COLOR LCD_COLOR_RED
#define BATTLESHIP_CURSOR_COLOR LCD_COLOR_GREEN

#define BATTLE_SHIP_LEFT_MARGIN 0
#define BATTLE_SHIP_TOP_MARGIN 10

typedef enum
{
    LCD_BOX_STATUS_HIT,
    LCD_BOX_STATUS_MISS,
    LCD_BOX_STATUS_EMPTY,
} battleship_box_status_t;

typedef enum
{
    BATTLESHIP_TYPE_CARRIER,    // 5 spaces
    BATTLESHIP_TYPE_BATTLESHIP, // 4 spaces
    BATTLESHIP_TYPE_CRUISER,    // 3 spaces
    BATTLESHIP_TYPE_SUBMARINE,  // 3 spaces
    BATTLESHIP_TYPE_DESTROYER,  // 2 spaces
    BATTLESHIP_TYPE_NONE
} battleship_type_t;

typedef struct
{
    uint8_t row;            // Row (0-9)
    uint8_t col;            // Column (0-9)
    uint16_t border_color;  // Border color
    uint16_t fill_color;    // Fill color
    battleship_type_t type; // Type of ship
    bool horizontal;        // Orientation of ship
} battleship_payload_t;

typedef struct
{
    uint8_t row;       // Row (0-9)
    uint8_t col;       // Column (0-9)
    uint8_t player_id; // Player ID (0 or 1)
    // ADD other fields as needed
} battleship_payload_cursor_t;

bool battleship_get_box_coordinates(lcd_coord_t *coord, uint8_t col, uint8_t row);
bool battleship_draw_game_board();
bool battleship_draw_cursor(uint8_t col, uint8_t row, uint16_t border_color, uint16_t fill_color);
bool battleship_clear_cursor(uint8_t col, uint8_t row, uint8_t player_id);

// Ship management functions
uint8_t battleship_get_ship_length(battleship_type_t type);
bool battleship_place_ship(uint8_t col, uint8_t row, battleship_type_t type, bool horizontal, uint8_t player_id);
bool battleship_check_overlap(uint8_t col, uint8_t row, battleship_type_t type, bool horizontal, uint8_t player_id);
void battleship_board_clear(void);

#endif // ECE353_FREERTOS

#endif /* __BATTLESHIP_H__ */