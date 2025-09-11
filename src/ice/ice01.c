/**
 * @file ice01.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-07-01
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE01)
#include "drivers.h"

char APP_DESCRIPTION[] = "ECE353: ICE 01 - Memory Mapped IO - GPIO";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

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
    /* Initialize the console for printf support */
    console_init();

    /* Initialize the buttons */
    buttons_init();

    /* Initialize the LEDs */
    leds_init();
    
    /* Ensure all LEDs start in OFF state */
    leds_set_state(LED_RED, LED_OFF);
    leds_set_state(LED_GREEN, LED_OFF);
    leds_set_state(LED_BLUE, LED_OFF);
    
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

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
    button_state_t sw1_state;
    button_state_t sw2_state;
    button_state_t sw3_state;
    //make all the states of the LEDs off initially
    leds_set_state(LED_RED, LED_OFF);
    leds_set_state(LED_GREEN, LED_OFF);
    leds_set_state(LED_BLUE, LED_OFF);


    while(1)
    {
        /* Check SW1 state - controls Red LED */
        sw1_state = buttons_get_state(BUTTON_SW1);
        if (sw1_state == BUTTON_STATE_FALLING_EDGE) {
            printf("SW1 has been pressed\n\r");
            leds_set_state(LED_RED, LED_ON);
        }
        else if (sw1_state == BUTTON_STATE_RISING_EDGE || sw1_state == BUTTON_STATE_HIGH) {
            if (sw1_state == BUTTON_STATE_RISING_EDGE) {
                printf("SW1 has been released\n\r");
            }
            leds_set_state(LED_RED, LED_OFF);
        }

        /* Check SW2 state - controls Green LED */
        sw2_state = buttons_get_state(BUTTON_SW2);
        if (sw2_state == BUTTON_STATE_FALLING_EDGE) {
            printf("SW2 has been pressed\n\r");
            leds_set_state(LED_GREEN, LED_ON);
        }
        else if (sw2_state == BUTTON_STATE_RISING_EDGE || sw2_state == BUTTON_STATE_HIGH) {
            if (sw2_state == BUTTON_STATE_RISING_EDGE) {
                printf("SW2 has been released\n\r");
            }
            leds_set_state(LED_GREEN, LED_OFF);
        }

        /* Check SW3 state - controls Blue LED */
        sw3_state = buttons_get_state(BUTTON_SW3);
        if (sw3_state == BUTTON_STATE_FALLING_EDGE) {
            printf("SW3 has been pressed\n\r");
            leds_set_state(LED_BLUE, LED_ON);
        }
        else if (sw3_state == BUTTON_STATE_RISING_EDGE) {
            printf("SW3 has been released\n\r");
            leds_set_state(LED_BLUE, LED_OFF);
        }

        /* Sleep for 100mS */
        cyhal_system_delay_ms(100);
    }
}
#endif
