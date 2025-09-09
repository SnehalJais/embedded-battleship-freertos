/*
 * adc.h
 *
 *  Created on: 4/20/2023
 *      Author: Joe Krachey
 */

#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#include "cyhal.h"
#include "cybsp.h"
#include "cy_result.h"
#include "cy_retarget_io.h"
#include "ece353-pins.h"
#include "cyhal_adc.h"

#include <ctype.h>
#include <stdio.h>

#define JOYSTICK_THRESH_X_LEFT_2P475V   0x0000
#define JOYSTICK_THRESH_X_RIGHT_0P825V  0x0000
#define JOYSTICK_THRESH_Y_UP_2P475V     0x0000
#define JOYSTICK_THRESH_Y_DOWN_0P825V   0x0000

#define JOYSTICK_THRESH_X_LEFT   JOYSTICK_THRESH_X_LEFT_2P475V 
#define JOYSTICK_THRESH_X_RIGHT  JOYSTICK_THRESH_X_RIGHT_0P825V
#define JOYSTICK_THRESH_Y_UP     JOYSTICK_THRESH_Y_UP_2P475V   
#define JOYSTICK_THRESH_Y_DOWN   JOYSTICK_THRESH_Y_DOWN_0P825V 


/* Custom Data Typtes */
typedef enum {
    JOYSTICK_POS_CENTER,
    JOYSTICK_POS_LEFT,
    JOYSTICK_POS_RIGHT,
    JOYSTICK_POS_UP,
    JOYSTICK_POS_DOWN,
    JOYSTICK_POS_UPPER_LEFT,
    JOYSTICK_POS_UPPER_RIGHT,
    JOYSTICK_POS_LOWER_LEFT,
    JOYSTICK_POS_LOWER_RIGHT
}joystick_position_t;

/* Public Global Variables */

/* Public API */

/** Initialize the ADC channels connected to the Joystick
 *
 * @param - None
 */
cy_rslt_t joystick_init(void);

/** Read X direction of Joystick 
 *
 * @param - None
 */
uint16_t  joystick_read_x(void);

/** Read Y direction of Joystick 
 *
 * @param - None
 */
uint16_t  joystick_read_y(void);

/**
 * @brief 
 * Returns the current position of the joystick 
 * @return joystick_position_t 
 */
joystick_position_t joystick_get_pos(void);

/**
 * @brief 
 * Prints out a string to the console based on parameter passed 
 * @param position 
 * The enum value to be printed.
 */
void joystick_print_pos(joystick_position_t position);

#endif /* __JOYSTICK_H__ */
