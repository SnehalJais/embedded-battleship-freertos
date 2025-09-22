/**
 * @file leds.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __LEDS_H__
#define __LEDS_H__

#include <stdio.h>
#include "cybsp.h"
#include "cyhal_gpio.h"
#include "ece353-pins.h"
#include "cyhal_pwm.h"

/* Enumerated type for selecting which LED to control */
typedef enum {
    LED_RED = 0,    /* Red LED */
    LED_GREEN,      /* Green LED */
    LED_BLUE,       /* Blue LED */
} ece353_led_t;

/* Enumerated type for LED states */
typedef enum {
    LED_OFF = 0,    /* LED is turned off */
    LED_ON          /* LED is turned on */
} ece353_led_state_t;

/**
 * @brief Initialize all three LEDs (Red, Green, Blue) as outputs
 * 
 * @return cy_rslt_t Returns CY_RSLT_SUCCESS if all LEDs were initialized successfully
 */
cy_rslt_t leds_init(void);

/**
 * @brief Set the state of a specific LED
 * 
 * @param led The LED to control (RED, GREEN, or BLUE)
 * @param state The desired state of the LED (ON or OFF)
 */
void leds_set_state(ece353_led_t led, ece353_led_state_t state);

// Function that configures the RGB LED pins to be controlled by PWM
cy_rslt_t leds_init_pwm(
cyhal_pwm_t *pwm_obj_red, 
cyhal_pwm_t *pwm_obj_green, 
cyhal_pwm_t *pwm_obj_blue
);


#endif