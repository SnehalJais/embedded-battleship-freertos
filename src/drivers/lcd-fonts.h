/**
 * @file lcd-fonts.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __LCD_FONTS_H__
#define __LCD_FONTS_H__

#include <stdint.h>
#include "lcd-io.h"

#define LCD_VERTICAL_CENTER         (240/2)
#define COLON_X_LOCATION            (320/2)
#define MINUTES_LSD_X_LOCATION      COLON_X_LOCATION - 30
#define MINUTES_MSD_X_LOCATION      MINUTES_LSD_X_LOCATION - 30
#define SECONDS_MSD_X_LOCATION      COLON_X_LOCATION + 30
#define SECONDS_LSD_X_LOCATION      SECONDS_MSD_X_LOCATION + 30

typedef struct
{
    uint8_t width;          // Character width in bits
    uint8_t height;         // Character height in bits
    uint16_t offset;        // Offset into the bitmap array
} FONT_CHAR_INFO;

typedef struct
{
    uint8_t height;          // Character height in bits
    uint8_t start_char;      // The first character in the font (e.g. in charInfo)
    uint8_t end_char;        // The last character in the font
    uint8_t space_width;     // Width of space character
    const FONT_CHAR_INFO *char_info; // Pointer to array of char information
    const uint8_t *bitmap;   // Pointer to generated array of character bitmaps
} FONT_INFO;

// Font data for Large Numeric Display
extern const uint8_t            FONT_NUM_LARGE_BITMAPS[] ;
extern const FONT_CHAR_INFO     FONT_CHAR_INFO_LARGE_NUMBERS[];

extern const uint8_t Consolas_20ptBitmaps[];
extern const FONT_CHAR_INFO Consolas_20ptDescriptors[];
extern const FONT_INFO Consolas_20ptFontInfo;



/* Used to print out the current time centered on the LCD screen*/
void lcd_draw_time(uint8_t minutes, uint8_t seconds);


#endif