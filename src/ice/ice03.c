/**
 * @file ice02.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-07-01
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "main.h"

#if defined(ICE03)
#include "drivers.h"
#include <stdio.h>

char APP_DESCRIPTION[] = "ECE353: ICE 03 - Timer Interrupts/Debounce Buttons";

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
    cy_rslt_t rslt;

    console_init();
    printf("\x1b[2J\x1b[;H");
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    // enable the pins connected to the RGB leds as output
    rslt = buttons_init_gpio();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Error: buttons_init_gpio() failed\n\r");
    }

    // enable the pins SW1, SW2, SW3 as inputs
    rslt = buttons_init_timer();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Error: buttons_init_timer() failed\n\r");
    }
    rslt = leds_init();
    if (rslt != CY_RSLT_SUCCESS)
    {
        printf("Error: led_init() failed\n\r");
    }
}

/*****************************************************************************/
/* Application Code                                                          */
/*****************************************************************************/
/**
 * @brief
 * This function implements the behavioral requirements for the ICE
 */

typedef enum
{
    STATE_INIT = 0,
    STATE_SW1_DET,
    STATE_SW2_DET_1,
    STATE_SW2_DET_2,
    STATE_SW3_DET,
} ice03_state_t;
    static ice03_state_t state = STATE_INIT;

void app_main(void)
{
    while (1)
    {
            switch (state)
            {
                case STATE_INIT:
                leds_set_state(LED_RED, LED_ON);
                leds_set_state(LED_GREEN, LED_OFF);
                leds_set_state(LED_BLUE, LED_OFF);
                if (ECE353_Events.sw1)
                {
                    ECE353_Events.sw1 = 0;
                    state = STATE_SW1_DET;
                }
                else if (ECE353_Events.sw2)
                {
                    ECE353_Events.sw2 = 0;
                    state = STATE_INIT;
                }
                else if (ECE353_Events.sw3)
                {
                    ECE353_Events.sw3 = 0;
                    state = STATE_INIT;
                }
                break;

                case STATE_SW1_DET:
                leds_set_state(LED_RED, LED_ON);
                leds_set_state(LED_GREEN, LED_OFF);
                leds_set_state(LED_BLUE, LED_ON);
                if (ECE353_Events.sw1)
                {
                    ECE353_Events.sw1 = 0;
                    state = STATE_INIT;
                }
                else if (ECE353_Events.sw2)
                {
                    ECE353_Events.sw2 = 0;
                    state = STATE_SW2_DET_1;
                }
                else if (ECE353_Events.sw3)
                {
                    ECE353_Events.sw3 = 0;
                    state = STATE_INIT;
                }
                break;
                case STATE_SW2_DET_1:
                leds_set_state(LED_RED, LED_OFF);
                leds_set_state(LED_GREEN, LED_OFF);
                leds_set_state(LED_BLUE, LED_ON);
                if (ECE353_Events.sw1)
                {
                    ECE353_Events.sw1 = 0;
                    state = STATE_INIT;
                }
                else if (ECE353_Events.sw2)
                {
                    ECE353_Events.sw2 = 0;
                    state = STATE_SW2_DET_2;
                }
                else if (ECE353_Events.sw3)
                {
                    ECE353_Events.sw3 = 0;
                    state = STATE_INIT;
                }
                break;
                case STATE_SW2_DET_2:
                leds_set_state(LED_RED, LED_OFF);
                leds_set_state(LED_GREEN, LED_ON);
                leds_set_state(LED_BLUE, LED_ON);
                if (ECE353_Events.sw1)
                {
                    ECE353_Events.sw1 = 0;
                    state = STATE_INIT;
                }
                else if (ECE353_Events.sw2)
                {
                    ECE353_Events.sw2 = 0;
                    state = STATE_INIT;
                }
                else if (ECE353_Events.sw3)
                {
                    ECE353_Events.sw3 = 0;
                    state = STATE_SW3_DET;
                }
                break;
                case STATE_SW3_DET:
                leds_set_state(LED_RED, LED_OFF);
                leds_set_state(LED_GREEN, LED_ON);
                leds_set_state(LED_BLUE, LED_OFF);
                if (ECE353_Events.sw1)
                {
                    ECE353_Events.sw1 = 0;
                    state = STATE_INIT;
                }
                else if (ECE353_Events.sw2)
                {
                    ECE353_Events.sw2 = 0;
                    state = STATE_INIT;
                }
                else if (ECE353_Events.sw3)
                {
                    ECE353_Events.sw3 = 0;
                    state = STATE_INIT;
                }
                break;
                default:
                    printf("ICE03: Unknown State!\n");
                    state = STATE_INIT;
            }

        }
    }

#endif
