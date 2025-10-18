/**
 * @file lcd_console.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-20
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "lcd_console.h"

#ifdef ECE353_FREERTOS

/**
 * @brief Erases a line from the LCD console
 *
 * @param x_offset
 */
static void lcd_console_erase_line(uint8_t x_offset, uint8_t line)
{

    uint16_t y_offset = line * LCD_CONSOLE_LINE_HEIGHT;
    lcd_draw_rectangle(
        x_offset,
        y_offset,
        320 - x_offset,
        LCD_CONSOLE_LINE_HEIGHT,
        LCD_COLOR_BLACK,
        false); // Fill the rectangle with black color
}

/**
 * @brief Draw a character to the LCD.  The character will only be displayed if it will be
 * fully rendered on the screen.
 *
 * @param x
 * @param y
 * @param c
 * @param color_fg
 * @param color_bg
 * @return true
 * @return false
 */
static bool lcd_console_draw_char(uint16_t *x, uint16_t y, char c, uint16_t color_fg, uint16_t color_bg)
{
    // Verify that the x pointer is not NULL
    if (x == NULL)
    {
        return false; // Invalid x offset
    }

    uint16_t char_index = c - Consolas_20ptFontInfo.start_char;           // Calculate the index of the character in the font
    uint16_t bitmap_height_ = Consolas_20ptFontInfo.height;               // Get the bitmap height
    uint16_t bitmap_offset = Consolas_20ptDescriptors[char_index].offset; // Get the bitmap offset
    uint16_t bitmap_width = Consolas_20ptDescriptors[char_index].width;   // Get the bitmap width
    uint8_t *bitmap = (uint8_t *)&Consolas_20ptBitmaps[bitmap_offset];    // Get the bitmap pointer

    // Check to see if the width of the charater bitmap plus the offset is greater than 320
    if (*x + bitmap_width > 320)
    {
        return false; // Character does not fit on the screen
    }

    // Draw the image
    lcd_draw_image(
        *x,
        y,
        bitmap_width,
        bitmap_height_,
        bitmap,
        color_fg,
        color_bg,
        false);

    *x += bitmap_width; // Move the x offset for the next character

    return true;
}

bool lcd_console_draw_string(lcd_console_payload_t *payload, uint8_t line)
{
    if (payload == NULL || payload->message == NULL || payload->length == 0)
    {
        return false; // Invalid payload
    }
    uint16_t x_offset = payload->x_offset;
    lcd_console_erase_line(x_offset, line);

for (uint16_t i = 0; i < payload->length; i++)
{
    if (!lcd_console_draw_char(&x_offset, line * LCD_CONSOLE_LINE_HEIGHT, payload->message[i], LCD_COLOR_WHITE, LCD_COLOR_BLACK))
    {
        /* Don't free string literals - only free if dynamically allocated */
        // vPortFree((void *) payload->message);
        return false; // Failed to draw character
    }
}

/* Don't free string literals - only free if dynamically allocated */
// vPortFree((void *) payload->message);
return true;
}
#endif // ECE353_FREERTOS