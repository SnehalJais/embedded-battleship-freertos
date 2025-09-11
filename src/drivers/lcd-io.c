/**
 * @file lcd-io.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "lcd-io.h"

/*******************************************************************************
* Function Name: lcd_write_cmd_u8
********************************************************************************
* Summary: Writes an 8-bit command used to identify which characteristic of the
*          LCD will be modified
* Return:
*  Nothing
*******************************************************************************/
__STATIC_INLINE void lcd_write_cmd_u8(uint8_t DL)
{
  //set the CSX pin low to select the LCD
  PORT_LCD_CSX -> OUT_CLR = mASK_LCD_CSX;

  //Set the DCX pin low to indicate that we are writing a command
  PORT_LCD_DCX -> OUT_CLR = mASK_LCD_DCX;

  //write data to the data pins
  PORT_LCD_DATA ->OUT = DL;

  //set the wrx pin low to indicate that we are writing data
  PORT_LCD_WRX -> OUT_CLR = mASK_LCD_WRX;

  //set the wrx pin high to complete the write
  PORT_LCD_WRX -> OUT_SET = mASK_LCD_WRX;

  //set the DCX pin high to show we are done writing a command
  PORT_LCD_DCX -> OUT_SET = mASK_LCD_DCX;

  //set the CSX pin to high to show we are done writing to LCD
  PORT_LCD_CSX -> OUT_SET = mASK_LCD_CSX;
}

/*******************************************************************************
* Function Name: lcd_write_data_u8
********************************************************************************
* Summary: Writes 8-bits of data to the specified LCD characteristic
* Return:
*  Nothing
*******************************************************************************/
__STATIC_INLINE void  lcd_write_data_u8 (uint8_t x)
{
  //set the CSX pin low to select the LCD
  PORT_LCD_CSX -> OUT_CLR = mASK_LCD_CSX;

  //write data to the data pins
  PORT_LCD_DATA ->OUT = x;

  //set the wrx pin low to indicate that we are writing data
  PORT_LCD_WRX -> OUT_CLR = mASK_LCD_WRX;

  //set the wrx pin high to complete the write
  PORT_LCD_WRX -> OUT_SET = mASK_LCD_WRX;


  //set the CSX pin to high to show we are done writing to LCD
  PORT_LCD_CSX -> OUT_SET = mASK_LCD_CSX;
}

/*******************************************************************************
* Function Name: lcd_write_data_u16
********************************************************************************
* Summary:
* Return:
*  Nothing
*******************************************************************************/
//write  data word
__STATIC_INLINE void  lcd_write_data_u16(uint16_t y)
{
  uint8_t upper_byte = (y >> 8) & 0xFF;
  uint8_t lower_byte = y & 0xFF;

  //set the CSX pin low to select the LCD
  PORT_LCD_CSX -> OUT_CLR = mASK_LCD_CSX;

  //write data to the data pins
  PORT_LCD_DATA ->OUT = upper_byte;

  //set the wrx pin low to indicate that we are writing data
  PORT_LCD_WRX -> OUT_CLR = mASK_LCD_WRX;

  //set the wrx pin high to complete the write
  PORT_LCD_WRX -> OUT_SET = mASK_LCD_WRX;

  //write data to the data pins
  PORT_LCD_DATA ->OUT = lower_byte;

  //set the wrx pin low to indicate that we are writing data
  PORT_LCD_WRX -> OUT_CLR = mASK_LCD_WRX;

  //set the wrx pin high to complete the write
  PORT_LCD_WRX -> OUT_SET = mASK_LCD_WRX;

  //set the CSX pin to high to show we are done writing to LCD
  PORT_LCD_CSX -> OUT_SET = mASK_LCD_CSX;
}

/*******************************************************************************
* Function Name: lcd_set_pos
********************************************************************************
* Summary: Sets the boundaries of the active portion of the screen.  When data
*          is written to the LCD, it will be written at addr of x0,y0.
*
* Return:
*  Nothing
*******************************************************************************/
void lcd_set_pos(uint16_t x0,uint16_t x1,uint16_t y0,uint16_t y1)
{
   lcd_write_cmd_u8(LCD_CMD_SET_COLUMN_ADDR);
  lcd_write_data_u16(x0);
  lcd_write_data_u16(x1);
  lcd_write_cmd_u8(LCD_CMD_SET_PAGE_ADDR);
  lcd_write_data_u16(y0);
  lcd_write_data_u16(y1);
  lcd_write_cmd_u8(LCD_CMD_MEMORY_WRITE);//LCD_WriteCMD(GRAMWR);
}

/*******************************************************************************
* Function Name: lcd_clear_screen
********************************************************************************
* Summary: clears the entire screen with the provided color.
*
* Return:
*  Nothing
*******************************************************************************/
void lcd_clear_screen(uint16_t bColor)
{
  uint16_t i,j;
  lcd_set_pos(0,LCD_ROWS - 1, 0,LCD_COLS - 1);

  for (i=0;i< LCD_COLS ;i++)
  {
        for(j= 0; j < LCD_ROWS; j++)
        {
            lcd_write_data_u16(bColor);
        }
  }
}


/**
 * @brief 
 * Used to determine coordinates of the bounding box of an image/shape
 * @param x_start 
 * @param x_len 
 * @param y_start 
 * @param y_len 
 * @param x0 
 * @param x1 
 * @param y0 
 * @param y1 
 * @param centered 
 */
static void lcd_get_coordinates(
  uint16_t  x_start,
  uint16_t  y_start,
  uint16_t  x_len,
  uint16_t  y_len,
  uint16_t  *x0, 
  uint16_t  *x1, 
  uint16_t  *y0, 
  uint16_t  *y1,
  bool      centered)
{
  if(centered)
  {
    // Set the left Col to be the center point minus half the width
    *x0 = x_start - (x_len/2);

    // Set the Right Col to be the center point plus half the width
    *x1 = x_start + (x_len/2);

    // Account for a width that is an even number
    if( (x_len & 0x01) == 0x00)
    {
        *x1 = *x1 - 1;
    }

    // Set the upper Row to be the center point minus half the height
    *y0 = y_start  - (y_len/2);

    // Set the upper Row to be the center point minus half the height
    *y1 = y_start  + (y_len/2) ;

    // Account for a height that is an  number
    if( (y_len & 0x01) == 0x00)
    {
        *y1 = *y1 - 1;
    }

  }
  else 
  {
    *x0 = x_start;
    *x1 = x_start + x_len -1;
    *y0 = y_start;
    *y1 = y_start + y_len -1;
  }
 
}

/*******************************************************************************
* Function Name: lcd_draw_rectangle
********************************************************************************
* Summary: Draws a rectangle starting at x_start,y_start.
* Returns:
*  Nothing
*******************************************************************************/
void lcd_draw_rectangle
  (
  uint16_t x_start,                 // X coordinate starting address
  uint16_t y_start,                 // Y coordinate starting address
  uint16_t width_pixels,            // image width
  uint16_t height_pixels,           // image height
  uint16_t fColor,                  // foreground color
  bool centered                     // Center the image on the provided coordinates
)
{
  uint16_t i;
  uint16_t j;
  uint16_t x0;
  uint16_t x1;
  uint16_t y0;
  uint16_t y1;

  lcd_get_coordinates(
    x_start, 
    y_start, 
    width_pixels, 
    height_pixels, 
    &x0, 
    &x1, 
    &y0, 
    &y1,
    centered
  ); 

  lcd_set_pos(x0,x1, y0, y1);

  for (i=0;i< height_pixels ;i++)
  {
    for(j= 0; j < width_pixels; j++)
    {
      lcd_write_data_u16(fColor);
    }
  }
}

/*******************************************************************************
* Function Name: lcd_draw_pixel
********************************************************************************
* Summary: Draws a single pixel at x,y
* Returns:
*  Nothing
*******************************************************************************/
void lcd_draw_pixel
(
  uint16_t x,
  uint16_t y,
  uint16_t fg_color
)
{
  lcd_set_pos(x,x, y, y);
  lcd_write_data_u16(fg_color);
}


/*******************************************************************************
* Function Name: lcd_draw_image
********************************************************************************
* Summary: Prints an image centered at the coordinates set by x_start, y_start
* Returns:
*  Nothing
*******************************************************************************/
void lcd_draw_image(
  uint16_t x_start,                 // X coordinate starting address
  uint16_t y_start,                 // Y coordinate starting address
  uint16_t width_pixels,            // image width
  uint16_t height_pixels,           // image height
  const uint8_t *image,             // bitmap of the image
  uint16_t fColor,                  // foreground color
  uint16_t bColor,                  // background color
  bool centered                     // Center the image on the provided coordinates
)
{
  uint16_t i,j;
  uint8_t data= 0;
  uint16_t byte_index;
  uint16_t bytes_per_row;
  uint16_t x0;
  uint16_t x1;
  uint16_t y0;
  uint16_t y1;

  lcd_get_coordinates(
    x_start, 
    y_start, 
    width_pixels, 
    height_pixels, 
    &x0, 
    &x1, 
    &y0, 
    &y1,
    centered
  ); 

  lcd_set_pos(x0, x1, y0, y1);

  bytes_per_row = width_pixels / 8;
  if( (width_pixels % 8) != 0)
  {
    bytes_per_row++;
  }

  for (i=0;i< height_pixels ;i++)
  {
        for(j= 0; j < width_pixels; j++)
        {
            if( (j %8) == 0)
            {
              byte_index = (i*bytes_per_row) + j/8;
              data = image[byte_index];
            }
            if ( data & 0x80)
            {
                lcd_write_data_u16(fColor);
            }
            else
            {
                lcd_write_data_u16(bColor);
            }
            data  = data << 1;
        }
  }
}

/*******************************************************************************
* Function Name: lcd_config_screen
********************************************************************************
* Summary: After the configuration of the gpio pins, this function configures
*          the internal LCD controller chip.
* Returns:
*  Nothing
*******************************************************************************/
void lcd_config_screen(void)

{
  lcd_write_cmd_u8(LCD_CMD_SOFTWARE_RESET); //software reset
  Cy_SysLib_Delay(50);

  lcd_write_cmd_u8(LCD_CMD_SLEEP_OUT);
  Cy_SysLib_Delay(120);
   lcd_write_cmd_u8(0xCF);
  lcd_write_data_u8(0x00);
  lcd_write_data_u8(0x83);
  lcd_write_data_u8(0X30);

   lcd_write_cmd_u8(0xED);
  lcd_write_data_u8(0x64);
  lcd_write_data_u8(0x03);
  lcd_write_data_u8(0X12);
  lcd_write_data_u8(0X81);

   lcd_write_cmd_u8(0xE8);
  lcd_write_data_u8(0x85);
  lcd_write_data_u8(0x01);
  lcd_write_data_u8(0x79);

   lcd_write_cmd_u8(0xCB);
  lcd_write_data_u8(0x39);
  lcd_write_data_u8(0x2C);
  lcd_write_data_u8(0x00);
  lcd_write_data_u8(0x34);
  lcd_write_data_u8(0x02);

   lcd_write_cmd_u8(0xF7);
  lcd_write_data_u8(0x20);

   lcd_write_cmd_u8(0xEA);
  lcd_write_data_u8(0x00);
  lcd_write_data_u8(0x00);

   lcd_write_cmd_u8(LCD_CMD_POWER_CONTROL_2);    //Power control
  lcd_write_data_u8(0x11);   //SAP[2:0];BT[3:0]

   lcd_write_cmd_u8(LCD_CMD_VCOMM_CONTROL_1);    //VCM control 1
  lcd_write_data_u8(0x34);
  lcd_write_data_u8(0x3D);

   lcd_write_cmd_u8(LCD_CMD_VCOMM_CONTROL_2);    //VCM control 2
  lcd_write_data_u8(0xC0);

   lcd_write_cmd_u8(LCD_CMD_MEMORY_ACCESS_CONTROL);    // Memory Access Control
  lcd_write_data_u8(0x68);

   lcd_write_cmd_u8(LCD_CMD_PIXEL_FORMAT_SET);      // Pixel format
  lcd_write_data_u8(0x55);  //16bit

   lcd_write_cmd_u8(LCD_CMD_FRAME_CONTROL_NORMAL_MODE);        // Frame rate
  lcd_write_data_u8(0x00);
  lcd_write_data_u8(0x1D);  //61Hz

   lcd_write_cmd_u8(0xB6);    // Display Function Control
  lcd_write_data_u8(0x0A);
  lcd_write_data_u8(0xA2);
  lcd_write_data_u8(0x27);
  lcd_write_data_u8(0x00);

  lcd_write_cmd_u8(LCD_CMD_ENTRY_MODE_SET); //Entry mode
  lcd_write_data_u8(0x07);


   lcd_write_cmd_u8(LCD_CMD_DISPLAY_FUNCTION_CONTROL);    // 3Gamma Function Disable
  lcd_write_data_u8(0x08);

   lcd_write_cmd_u8(LCD_CMD_GAMMA_SET);    //Gamma curve selected
  lcd_write_data_u8(0x01);


  lcd_write_cmd_u8(LCD_CMD_POSITIVE_GAMMA_CORRECTION); //positive gamma correction
  lcd_write_data_u8(0x1f);
  lcd_write_data_u8(0x1a);
  lcd_write_data_u8(0x18);
  lcd_write_data_u8(0x0a);
  lcd_write_data_u8(0x0f);
  lcd_write_data_u8(0x06);
  lcd_write_data_u8(0x45);
  lcd_write_data_u8(0x87);
  lcd_write_data_u8(0x32);
  lcd_write_data_u8(0x0a);
  lcd_write_data_u8(0x07);
  lcd_write_data_u8(0x02);
  lcd_write_data_u8(0x07);
  lcd_write_data_u8(0x05);
  lcd_write_data_u8(0x00);

  lcd_write_cmd_u8(LCD_CMD_NEGATIVE_GAMMA_CORRECTION); //negamma correction
  lcd_write_data_u8(0x00);
  lcd_write_data_u8(0x25);
  lcd_write_data_u8(0x27);
  lcd_write_data_u8(0x05);
  lcd_write_data_u8(0x10);
  lcd_write_data_u8(0x09);
  lcd_write_data_u8(0x3a);
  lcd_write_data_u8(0x78);
  lcd_write_data_u8(0x4d);
  lcd_write_data_u8(0x05);
  lcd_write_data_u8(0x18);
  lcd_write_data_u8(0x0d);
  lcd_write_data_u8(0x38);
  lcd_write_data_u8(0x3a);
  lcd_write_data_u8(0x1f);

   lcd_write_cmd_u8(LCD_CMD_SLEEP_OUT);    //Exit Sleep
   Cy_SysLib_Delay(120);
   lcd_write_cmd_u8(LCD_CMD_DISPLAY_ON);    //Display on
   Cy_SysLib_Delay(50);

}

/*******************************************************************************
* Function Name: lcd_config_gpio
********************************************************************************
* Summary: Configures the GPIO pins used to implement the 8080 interface on the
*          LCD display
*
* Parameters
*
* Return:
*  Nothing
*******************************************************************************/
cy_rslt_t lcd_config_gpio(void)
{
  cy_rslt_t rslt;

  rslt = cyhal_gpio_init(PIN_LCD_D0, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_D1, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;   

  rslt = cyhal_gpio_init(PIN_LCD_D2, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_D3, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_D4, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_D5, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_D6, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_D7, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_CSX, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_DCX, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS) return rslt;
  rslt = cyhal_gpio_init(PIN_LCD_WRX, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);
  if (rslt != CY_RSLT_SUCCESS)return rslt;
    
 
}

/*******************************************************************************
* Function Name:lcd_initialize
********************************************************************************
* Summary: Configures the LCD screen and displays a default image              *
* Parameters
*
* Return:
*  Nothing
*******************************************************************************/
cy_rslt_t lcd_initialize(void)
{
  cy_rslt_t result;

  result = lcd_config_gpio();

  if(result == CY_RSLT_SUCCESS)
  {
    lcd_config_screen();
	  lcd_clear_screen(LCD_COLOR_BLACK);
  }
  else
  {
    printf("LCD GPIO configuration failed with error: %d\n", result);
    return result;
  }

  return result;
}
