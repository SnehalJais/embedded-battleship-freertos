/**
 * @file leds.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #include "leds.h"

 /**
 * @brief Initialize all three LEDs (Red, Green, Blue) as outputs
 * 
 * @return cy_rslt_t Returns CY_RSLT_SUCCESS if all LEDs were initialized successfully
 */
cy_rslt_t leds_init(void)
{
    cy_rslt_t rslt;

    /* Initialize RED LED */
    rslt = cyhal_gpio_init(PIN_LED_RED, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }

    /* Initialize GREEN LED */
    rslt = cyhal_gpio_init(PIN_LED_GREEN, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }

    /* Initialize BLUE LED */
    rslt = cyhal_gpio_init(PIN_LED_BLUE, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 0);
    return rslt;
}


/**
 * @brief Set the state of a specific LED
 * 
 * @param led The LED to control (RED, GREEN, or BLUE)
 * @param state The desired state of the LED (ON or OFF)
 */
void leds_set_state(ece353_led_t led, ece353_led_state_t state)
{
    /* Since LEDs are active low, we need to invert the state */

    
    /* Use switch for fastest execution - compiler will generate jump table */
    switch(led)
    {
        case LED_RED:
        if(state == LED_ON)
        {
            PORT_LED_RED->OUT |= MASK_LED_RED;
        }
        else
        {
            PORT_LED_RED->OUT &= ~MASK_LED_RED;
        }
            break;
        case LED_GREEN:
        if(state == LED_ON)
        {
            PORT_LED_GREEN->OUT |= MASK_LED_GREEN;
        }
        else
        {
            PORT_LED_GREEN->OUT &= ~MASK_LED_GREEN;
        }
            break;
        case LED_BLUE:
        if(state == LED_ON)
        {
            PORT_LED_BLUE->OUT |= MASK_LED_BLUE;
        }
        else
        {
            PORT_LED_BLUE->OUT &= ~MASK_LED_BLUE;
        }
            break;
        default:
            /* Invalid LED - do nothing */
            break;
    }
}
/**
 * @brief Configures the RGB LED pins to be controlled by PWM
 * 
 * @param pwm_obj_red Pointer to a cyhal_pwm_t object for the red LED
 * @param pwm_obj_green Pointer to a cyhal_pwm_t object for the green LED
 * @param pwm_obj_blue Pointer to a cyhal_pwm_t object for the blue LED
 * @return cy_rslt_t Returns CY_RSLT_SUCCESS if all PWM objects were initialized successfully
 */
cy_rslt_t leds_init_pwm(
    cyhal_pwm_t *pwm_obj_red,
    cyhal_pwm_t *pwm_obj_green,
    cyhal_pwm_t *pwm_obj_blue
){
    cy_rslt_t rslt;

    rslt = cyhal_pwm_init(pwm_obj_red, PIN_LED_RED, NULL);
    rslt = cyhal_pwm_start(pwm_obj_red);

    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }

    rslt = cyhal_pwm_init(pwm_obj_green, PIN_LED_GREEN, NULL);
    rslt = cyhal_pwm_start(pwm_obj_green);
    if (rslt != CY_RSLT_SUCCESS) {
        return rslt;
    }

    rslt = cyhal_pwm_init(pwm_obj_blue, PIN_LED_BLUE, NULL);
    rslt = cyhal_pwm_start(pwm_obj_blue);
    return rslt;
}
