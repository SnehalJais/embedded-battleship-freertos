/**
 * @file ece353-pins.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-07-01
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __ECE353_BSP_H__
#define __ECE353_BSP_H__

/***************************************************************************/
/* LEDs                                                                    */
/***************************************************************************/
#define PIN_LED_RED P9_0
#define PIN_LED_BLUE P8_0
#define PIN_LED_GREEN P9_2

#define PORT_LED_RED GPIO_PRT9
#define PORT_LED_BLUE GPIO_PRT8
#define PORT_LED_GREEN GPIO_PRT9

#define MASK_LED_RED (1 << 0)
#define MASK_LED_BLUE (1 << 0)
#define MASK_LED_GREEN (1 << 2)

/***************************************************************************/
/* Buttons                                                                 */
/***************************************************************************/
#define PIN_BUTTON_SW1 P6_3
#define PIN_BUTTON_SW2 P6_4
#define PIN_BUTTON_SW3 P6_5

#define PORT_BUTTON_SW1 GPIO_PRT6
#define PORT_BUTTON_SW2 GPIO_PRT6
#define PORT_BUTTON_SW3 GPIO_PRT6

#define MASK_BUTTON_PIN_SW1 (1 << 3)
#define MASK_BUTTON_PIN_SW2 (1 << 4)
#define MASK_BUTTON_PIN_SW3 (1 << 5)

/***************************************************************************/
/* LCD                                                                     */
/***************************************************************************/
#define PIN_LCD_D0 P2_0
#define PIN_LCD_D1 P2_1
#define PIN_LCD_D2 P2_2
#define PIN_LCD_D3 P2_3
#define PIN_LCD_D4 P2_4
#define PIN_LCD_D5 P2_5
#define PIN_LCD_D6 P2_6
#define PIN_LCD_D7 P2_7

#define PIN_LCD_CSX P1_0
#define PIN_LCD_DCX P1_1
#define PIN_LCD_WRX P1_2

#define mASK_LCD_CSX (1 << 0)
#define mASK_LCD_DCX (1 << 1)
#define mASK_LCD_WRX (1 << 2)

#define PORT_LCD_DATA GPIO_PRT2
#define PORT_LCD_CSX GPIO_PRT1
#define PORT_LCD_DCX GPIO_PRT1
#define PORT_LCD_WRX GPIO_PRT1
/***************************************************************************/
/* BUZZER                                                                  */
/***************************************************************************/
#define PIN_BUZZER P6_2
#define PORT_BUZZER GPIO_PRT6 // Port for Buzzer
#define MASK_BUZZER (1 << 2)  // Bit mask for Buzzer pin

/***************************************************************************/
/* Joystick                                                                */
/***************************************************************************/
#define PIN_ANALOG_JOY_X P10_6
#define PIN_ANALOG_JOY_Y P10_7

/***************************************************************************/
/* I2C                                                                     */
/***************************************************************************/
#define PIN_I2C_SDA P0_3 // I2C Data line
#define PIN_I2C_SCL P0_2 // I2C Clock line

/***************************************************************************/
/* SPI                                                                     */
/***************************************************************************/
#define PIN_SPI_MOSI P7_0 // SPI Master Out Slave In
#define PIN_SPI_MISO P7_1 // SPI Master In Slave Out
#define PIN_SPI_SCK P7_2  // SPI Clock

/***************************************************************************/
/*IMU                                                                      */
/***************************************************************************/
#define PIN_IMU_CS P7_4 // IMU Chip Select Pin

/***************************************************************************/
/*EEPROM                                                                  */
/***************************************************************************/
#define PIN_EEPROM_CS P7_3 // EEPROM Chip Select Pin

/***************************************************************************/
/* IPC UART                                                                */
/***************************************************************************/
#define PIN_IPC_TX P3_1 // UART_EXT_TX (pin 26)
#define PIN_IPC_RX P3_0 // UART_EXT_RX (pin 25)

#endif
