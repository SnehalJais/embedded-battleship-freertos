/**
 * @file buttons.h
 * @author  Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <stdio.h>
#include "cybsp.h"
#include "cyhal_gpio.h"
#include "cyhal_timer.h" 
#include "ece353-pins.h"
#include "ece353-events.h"
#include "timer.h"

typedef enum {
    BUTTON_SW1 = 0,
    BUTTON_SW2,
    BUTTON_SW3,
} ece353_button_t;

typedef enum {
    BUTTON_STATE_FALLING_EDGE = 0,
    BUTTON_STATE_LOW,
    BUTTON_STATE_HIGH,
    BUTTON_STATE_RISING_EDGE,
} button_state_t;

/**
 * @brief Initialize all three buttons (SW1, SW2, SW3) as inputs with pull-up resistors
 * 
 * @return cy_rslt_t Returns CY_RSLT_SUCCESS if all buttons were initialized successfully
 */
cy_rslt_t buttons_init(void);

// Function to initialize the timer for button debouncing
cy_rslt_t buttons_init_timer(void);

// Function to read the state of a specific button
button_state_t buttons_get_state(ece353_button_t button);



#endif