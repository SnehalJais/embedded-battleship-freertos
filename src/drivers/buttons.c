/**
 * @file buttons.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #include "buttons.h"

/* HAL GPIO objects for the 3 buttons */
cyhal_gpio_t button_sw1;
cyhal_gpio_t button_sw2;
cyhal_gpio_t button_sw3;

/* Static variables to store previous button states */
static bool prev_button_states[3] = {true, true, true};  // Initialize to true (pulled up)

/**
 * @brief Initialize all three buttons (SW1, SW2, SW3) as inputs with pull-up resistors
 * 
 * @return cy_rslt_t Returns CY_RSLT_SUCCESS if all buttons were initialized successfully
 */
cy_rslt_t buttons_init(void)
{
    cy_rslt_t rslt;

    /* Initialize SW1 */
    rslt = cyhal_gpio_init(
        PIN_BUTTON_SW1,                    /* Pin */
        CYHAL_GPIO_DIR_INPUT,             /* Direction */
        CYHAL_GPIO_DRIVE_PULLUP,          /* Drive Mode */
        true                              /* Init State - High due to pull-up */
    );
    if (rslt != CY_RSLT_SUCCESS) return rslt;

    /* Initialize SW2 */
    rslt = cyhal_gpio_init(
        PIN_BUTTON_SW2,                    /* Pin */
        CYHAL_GPIO_DIR_INPUT,             /* Direction */
        CYHAL_GPIO_DRIVE_PULLUP,          /* Drive Mode */
        true                              /* Init State - High due to pull-up */
    );
    if (rslt != CY_RSLT_SUCCESS) return rslt;

    /* Initialize SW3 */
    rslt = cyhal_gpio_init(
        PIN_BUTTON_SW3,                    /* Pin */
        CYHAL_GPIO_DIR_INPUT,             /* Direction */
        CYHAL_GPIO_DRIVE_PULLUP,          /* Drive Mode */
        true                              /* Init State - High due to pull-up */
    );

    return rslt;
}

/**
 * @brief Get the current state of a button based on current and previous logic levels
 * 
 * @param button The button to check (BUTTON_SW1, BUTTON_SW2, or BUTTON_SW3)
 * @return button_state_t The current state of the button
 */
button_state_t buttons_get_state(ece353_button_t button)
{
    bool current_state;
    
    /* Get the current logic level of the specified button */
    switch(button) {
        case BUTTON_SW1:
            current_state = cyhal_gpio_read(PIN_BUTTON_SW1);
            break;
        case BUTTON_SW2:
            current_state = cyhal_gpio_read(PIN_BUTTON_SW2);
            break;
        case BUTTON_SW3:
            current_state = cyhal_gpio_read(PIN_BUTTON_SW3);
            break;
        default:
            return BUTTON_STATE_HIGH; // Default to high (unpressed) for invalid button
    }
    
    /* Get button state based on current and previous logic levels */
    button_state_t state;
    
    if (current_state == prev_button_states[button]) {
        /* No change in state */
        state = current_state ? BUTTON_STATE_HIGH : BUTTON_STATE_LOW;
    } else {
        /* State changed */
        state = current_state ? BUTTON_STATE_RISING_EDGE : BUTTON_STATE_FALLING_EDGE;
    }
    
    /* Update previous state */
    prev_button_states[button] = current_state;
    
    return state;
}