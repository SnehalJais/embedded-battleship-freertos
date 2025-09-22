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
#include "cyhal_timer.h"
#include "ece353-events.h"
#include "ece353-pins.h"
/* HAL GPIO objects for the 3 buttons */
cyhal_gpio_t button_sw1;
cyhal_gpio_t button_sw2;
cyhal_gpio_t button_sw3;

/**
 * @brief Initialize all three buttons (SW1, SW2, SW3) as inputs with pull-up resistors
 * 
 * @return cy_rslt_t Returns CY_RSLT_SUCCESS if all buttons were initialized successfully
 */
cy_rslt_t buttons_init_gpio(void)
{
    cy_rslt_t rslt;

    /* Initialize SW1 */
    rslt = cyhal_gpio_init(
        PIN_BUTTON_SW1,                    /* Pin */
        CYHAL_GPIO_DIR_INPUT,             /* Direction */
        CYHAL_GPIO_DRIVE_NONE,          /* Drive Mode */
        true                              /* Init State - High due to pull-up */
    );
    if (rslt != CY_RSLT_SUCCESS) return rslt;

    /* Initialize SW2 */
    rslt = cyhal_gpio_init(
        PIN_BUTTON_SW2,                    /* Pin */
        CYHAL_GPIO_DIR_INPUT,             /* Direction */
        CYHAL_GPIO_DRIVE_NONE,          /* Drive Mode */
        true                              /* Init State - High due to pull-up */
    );
    if (rslt != CY_RSLT_SUCCESS) return rslt;

    /* Initialize SW3 */
    rslt = cyhal_gpio_init(
        PIN_BUTTON_SW3,                    /* Pin */
        CYHAL_GPIO_DIR_INPUT,             /* Direction */
        CYHAL_GPIO_DRIVE_NONE,          /* Drive Mode */
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
    /* Static variables to store previous button states */
    static bool prev_button1 = true;
    static bool prev_button2 = true;
    static bool prev_button3 = true;

    static bool curr_button1 = true;
    static bool curr_button2 = true;
    static bool curr_button3 = true;

    bool current;
    bool prev;
    /* Get the current logic level of the specified button */
    switch(button) {
        case BUTTON_SW1:
            curr_button1 = cyhal_gpio_read(PIN_BUTTON_SW1);
            current = curr_button1;
            prev = prev_button1;
            break;
        case BUTTON_SW2:
            curr_button2 = cyhal_gpio_read(PIN_BUTTON_SW2);
            current = curr_button2;
            prev = prev_button2;
            break;
        case BUTTON_SW3:
            curr_button3 = cyhal_gpio_read(PIN_BUTTON_SW3);
            current = curr_button3;
            prev = prev_button3;
            break;
        default:
            return BUTTON_STATE_HIGH; // Default to high (unpressed) for invalid button
    }
    /* Get button state based on current and previous logic levels */
    button_state_t state;

    if (current == prev) {
        /* No change in state */
        state = current ? BUTTON_STATE_HIGH : BUTTON_STATE_LOW;
    } else {
        /* State changed */
        state = current ? BUTTON_STATE_RISING_EDGE : BUTTON_STATE_FALLING_EDGE;
    }
    
    /* Update previous state */
    switch(button) {
        case BUTTON_SW1:
            prev_button1 = current;
            break;
        case BUTTON_SW2:
            prev_button2 = current;
            break;
        case BUTTON_SW3:
            prev_button3 = current;
            break;
        default:
            break; // Should not reach here
    }
    return state;
}
cyhal_timer_t button_timer;
cyhal_timer_cfg_t button_timer_cfg;

static void button_timer_handler(void *arg, cyhal_timer_event_t event)
{
    static uint8_t button_counts[3] = {[0]=0, [1]=0, [2]=0};
    uint8_t sw1 = PORT_BUTTON_SW1->IN & MASK_BUTTON_PIN_SW1;
    uint8_t sw2 = PORT_BUTTON_SW2->IN & MASK_BUTTON_PIN_SW2;
    uint8_t sw3 = PORT_BUTTON_SW3->IN & MASK_BUTTON_PIN_SW3;
    if(sw1 == 0){
        button_counts[0]++;
        if(button_counts[0] == 5){
            ECE353_Events.sw1 = 1;
        }
    }
    else{
        button_counts[0] = 0;
    }
    if(sw2 == 0){
        button_counts[1]++;
        if(button_counts[1] == 5){
            ECE353_Events.sw2 = 1;
        }
    }
    else{
        button_counts[1] = 0;
    }
    if(sw3 == 0){
        button_counts[2]++;
        if(button_counts[2] == 5){
            ECE353_Events.sw3 = 1;
        }
    }
    else{
        button_counts[2] = 0;
    }

}

cy_rslt_t buttons_init_timer(void){
    return timer_init(&button_timer,&button_timer_cfg,500000,button_timer_handler);
}