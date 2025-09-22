/**
 * @file ice04.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE04)
#include "drivers.h"
#include <stdio.h>
#include "leds.h"

char APP_DESCRIPTION[] = "ECE353: ICE 04 - PWM Buzzer";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
//initialize the PWM objects for the RGB LEDs
cyhal_pwm_t pwm_obj_red;
cyhal_pwm_t pwm_obj_green;
cyhal_pwm_t pwm_obj_blue;

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/


/**
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 */
void app_init_hw(void)
{
    cy_rslt_t rslt;

    console_init();
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    /* ADD CODE */
    //initialize the PWM peripherals used to control the LEDs
    rslt = leds_init_pwm(&pwm_obj_red, &pwm_obj_green, &pwm_obj_blue);
    if (rslt != CY_RSLT_SUCCESS) {
        printf("leds_init_pwm() failed: %lu\r\n", rslt);
        CY_ASSERT(0);
    }   

    //initialize the buttons
    rslt = buttons_init_gpio();
    if (rslt != CY_RSLT_SUCCESS) {
        printf("Error: buttons_init_gpio() failed\n\r");
        CY_ASSERT(0);
    }
    rslt = buttons_init_timer();
    if (rslt != CY_RSLT_SUCCESS) {
        printf("Error: buttons_init_timer() failed\n\r");
        CY_ASSERT(0);
    }
    
        
}

    
    



/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */
void app_main(void)
{
    while(1)
    {
        /* ADD CODE*/
        if (ECE353_Events.sw1)
        {
            //print sw1 was pressed
            printf("SW1 Pressed\r\n");
            //increase intensity by 10% till it reaches 100% then make it 0%
            //Print out the current intensity levels when any button is pressed
            static int intensity = 0;
            ECE353_Events.sw1 = 0;
            cyhal_pwm_stop(&pwm_obj_red); // Stop the PWM before changing the duty cycle5
            
            if (intensity < 100)
            {
                intensity += 10;
                printf("Intensity: %d%%\r\n", intensity);
            }
            else
            {
                intensity = 0;
                printf("Intensity: %d%%\r\n", intensity);
            }
            cyhal_pwm_set_duty_cycle(&pwm_obj_red, intensity, 1000);
            cyhal_pwm_start(&pwm_obj_red);

        }

         if (ECE353_Events.sw2)
        {
            //print sw1 was pressed
            printf("SW2 Pressed\r\n");
            //increase intensity by 10% till it reaches 100% then make it 0%
            //Print out the current intensity levels when any button is pressed
            static int intensity = 0;
            ECE353_Events.sw2 = 0;
            cyhal_pwm_stop(&pwm_obj_green); // Stop the PWM before changing the duty cycle5
            
            if (intensity < 100)
            {
                intensity += 10;
                printf("Intensity: %d%%\r\n", intensity);
            }
            else
            {
                intensity = 0;
                printf("Intensity: %d%%\r\n", intensity);
            }
            cyhal_pwm_set_duty_cycle(&pwm_obj_green, intensity, 1000);
            cyhal_pwm_start(&pwm_obj_green);

        }


         if (ECE353_Events.sw3)
        {
            //print sw1 was pressed
            printf("SW3 Pressed\r\n");
            //increase intensity by 10% till it reaches 100% then make it 0%
            //Print out the current intensity levels when any button is pressed
            static int intensity = 0;
            ECE353_Events.sw3 = 0;
            cyhal_pwm_stop(&pwm_obj_blue); // Stop the PWM before changing the duty cycle5
            
            if (intensity < 100)
            {
                intensity += 10;
                printf("Intensity: %d%%\r\n", intensity);
            }
            else
            {
                intensity = 0;
                printf("Intensity: %d%%\r\n", intensity);
            }
            cyhal_pwm_set_duty_cycle(&pwm_obj_blue, intensity, 1000);
            cyhal_pwm_start(&pwm_obj_blue);

        }


        

        /* END ADD CODE */
    }
}
#endif
