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

 #ifdef ECE353_FREERTOS

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
    if( row < 10 && col < 10)
    {
        coord->x = BATTLE_SHIP_LEFT_MARGIN + (col * BATTLESHIP_BOX_WIDTH);
        coord->y = BATTLE_SHIP_TOP_MARGIN + (row * BATTLESHIP_BOX_HEIGHT);
        return true;
    }
    else {
        return false; // Invalid row or column
    }
}

/**
 * @brief 
 * Used to draw an empty game board for the specified player.
 * @param player_id 
 * @return true 
 * @return false 
 */
bool battleship_draw_game_board(uint8_t player_id)
{
    return true;
}

/**
 * @brief 
 * Draws the cursor for the currently active location by changing
 * the color of the box border.
 * @param col 
 * @param row 
 * @return true 
 * @return false 
 */
bool battleship_draw_cursor(uint8_t col, uint8_t row)
{
    return false;
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
    return false;
}
#endif