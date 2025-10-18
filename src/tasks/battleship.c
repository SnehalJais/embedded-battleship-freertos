/**
 * @file battleship.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-20
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "battleship.h"
#include "task_lcd.h"
#include "task_console.h"

#ifdef ECE353_FREERTOS

// Game board to track ship positions (0 = empty, 1 = occupied)
// [row][col]
static uint8_t game_board[10][10] = {0};


uint8_t battleship_get_ship_length(battleship_type_t type)
{
    switch(type) {
        case BATTLESHIP_TYPE_CARRIER:    return 5;
        case BATTLESHIP_TYPE_BATTLESHIP: return 4;
        case BATTLESHIP_TYPE_CRUISER:    return 3;
        case BATTLESHIP_TYPE_SUBMARINE:  return 3;
        case BATTLESHIP_TYPE_DESTROYER:  return 2;
        case BATTLESHIP_TYPE_NONE:       return 0;
        default:                         return 0;
    }
}

/**
 * @brief Get the coordinates of a box on the LCD screen.
 *
 * @param coord Pointer to the lcd_coord_t structure to store the coordinates.
 * @param col The column of the box (0-9).
 * @param row The row of the box (0-9).
 * @return true if the coordinates were successfully calculated, false otherwise.
 */
bool battleship_get_box_coordinates(lcd_coord_t *coord, uint8_t col, uint8_t row)
{
    if (row < 10 && col < 10)
    {
        coord->x = BATTLE_SHIP_LEFT_MARGIN + (col * BATTLESHIP_BOX_WIDTH);
        coord->y = BATTLE_SHIP_TOP_MARGIN + (row * BATTLESHIP_BOX_HEIGHT);
        return true; // Success
    }
    else
    {
        return false; // Invalid row or column
    }
}

/**
 * @brief
 * Draw a 10x10 grid of blue Battleship rectangles.
 * Each rectangle is 20 pixels by 20 pixels.
 * @param player_id
 * @return true
 * @return false
 */
bool battleship_draw_game_board()
{
    lcd_coord_t coord;

    for (uint8_t row = 0; row < 10; row++)
    {
        for (uint8_t col = 0; col < 10; col++)
        {
            if (!battleship_get_box_coordinates(&coord, col, row))
            {
                return false; // Failed to get coordinates
            }

            lcd_draw_rectangle(coord.x, coord.y, BATTLESHIP_BOX_WIDTH, BATTLESHIP_BOX_HEIGHT, LCD_COLOR_BLUE, true);
            lcd_draw_rectangle(coord.x, coord.y, BATTLESHIP_BOX_WIDTH - BATTLESHIP_BORDER_WIDTH, BATTLESHIP_BOX_HEIGHT - BATTLESHIP_BORDER_WIDTH, LCD_COLOR_BLACK, true);
        }
    }
    return true;
}

/**
 * @brief
 *  Draw a single Battleship rectangle
 * @return true
 * @return false
 */
bool battleship_draw_cursor(
    uint8_t col,
    uint8_t row,
    uint16_t border_color,
    uint16_t fill_color)
{
    lcd_coord_t coord;

    if (!battleship_get_box_coordinates(&coord, col, row))
    {
        return false; // Failed to get coordinates
    }

    // Draw the outer rectangle (border)
    lcd_draw_rectangle(coord.x, coord.y, BATTLESHIP_BOX_WIDTH, BATTLESHIP_BOX_HEIGHT, border_color, false);
    
    // Draw the inner rectangle (fill) - smaller to create border effect
    lcd_draw_rectangle(coord.x + BATTLESHIP_BORDER_WIDTH/2, 
                      coord.y + BATTLESHIP_BORDER_WIDTH/2, 
                      BATTLESHIP_BOX_WIDTH - BATTLESHIP_BORDER_WIDTH, 
                      BATTLESHIP_BOX_HEIGHT - BATTLESHIP_BORDER_WIDTH, 
                      fill_color, true);
    
    return true;
}

/**
 * @brief
 *  Restores a box border to the color of the game board
 * @param col
 * @param row
 * @param player_id
 * @return true
 * @return false
 */
bool battleship_clear_cursor(uint8_t col, uint8_t row, uint8_t player_id)
{
    // Simply redraw the box with default game board colors
    return battleship_draw_cursor(col, row, LCD_COLOR_BLUE, LCD_COLOR_BLACK);
}


//place battledhip with error checking
bool battleship_place_ship(uint8_t col, uint8_t row, battleship_type_t type, bool horizontal, uint8_t player_id)
{
    // Implementation for placing ships with error checking can be added here
    // Check for valid placement, overlaps, boundaries, etc.
    uint8_t ship_length = battleship_get_ship_length(type);
    
    // Boundary checks
    if (horizontal)
    {
        if (col + ship_length > 10)
        {
            return false; // Ship goes out of bounds
        }
    }
    else
    {
        if (row + ship_length > 10)
        {
            return false; // Ship goes out of bounds
        }
    }
    
    // Check for overlaps with existing ships
    if (battleship_check_overlap(col, row, type, horizontal, player_id))
    {
        return false; // Ship would overlap with existing ship
    }
    
    // If all checks pass, place the ship and update the game board
    for (uint8_t i = 0; i < ship_length; i++)
    {
        uint8_t place_col, place_row;
        
        if (horizontal)
        {
            place_col = col + i;
            place_row = row;
        }
        else
        {
            place_col = col;
            place_row = row + i;
        }
        
        // Mark this space as occupied in the game board
        game_board[place_row][place_col] = 1;
        
        // Draw the ship space in grey
        battleship_draw_cursor(place_col, place_row, LCD_COLOR_BLUE, LCD_COLOR_GRAY);
    }

    return true;
}

//overlapping ship check
bool battleship_check_overlap(uint8_t col, uint8_t row, battleship_type_t type, bool horizontal, uint8_t player_id)
{
    uint8_t ship_length = battleship_get_ship_length(type);
    
    // Check each space the ship would occupy
    for (uint8_t i = 0; i < ship_length; i++)
    {
        uint8_t check_col, check_row;
        
        if (horizontal)
        {
            check_col = col + i;
            check_row = row;
        }
        else
        {
            check_col = col;
            check_row = row + i;
        }
        
        // Guard against out-of-bounds access - treat OOB as conflict
        if (check_row >= 10 || check_col >= 10) {
            return true; // Treat OOB as overlap/invalid
        }
        
        // Check if this space is already occupied
        if (game_board[check_row][check_col] != 0)
        {
            return true; // Overlap detected
        }
    }
    
    return false; // No overlap
}

// Clear the internal game board array 
void battleship_board_clear(void)
{
    for (uint8_t r = 0; r < 10; r++) {
        for (uint8_t c = 0; c < 10; c++) {
            game_board[r][c] = 0;
        }
    }
}

// // Test function to place ships according to the configuration
// bool battleship_place_test_ships()
// {
//     // Clear the internal game board data (LCD task handles visual clearing)
//     battleship_clear_game_board();
    
//     // Place ships according to the test configuration
//     // Note: Position (x,y) maps to (col, row)
    
//     // Carrier at (9,0) Horizontal - length 5
//     if (!battleship_place_ship(9, 0, BATTLESHIP_TYPE_CARRIER, true))
//         return false;
    
//     // Battleship at (0,0) Horizontal - length 4  
//     if (!battleship_place_ship(0, 0, BATTLESHIP_TYPE_BATTLESHIP, true))
//         return false;
    
//     // Destroyer at (2,2) Vertical - length 2
//     if (!battleship_place_ship(2, 2, BATTLESHIP_TYPE_DESTROYER, false))
//         return false;
    
//     // Submarine at (5,5) Horizontal - length 3
//     if (!battleship_place_ship(5, 5, BATTLESHIP_TYPE_SUBMARINE, true))
//         return false;
    
//     // Cruiser at (7,7) Vertical - length 3
//     if (!battleship_place_ship(7, 7, BATTLESHIP_TYPE_CRUISER, false))
//         return false;
    
//     return true; // All ships placed successfully
// }
#endif