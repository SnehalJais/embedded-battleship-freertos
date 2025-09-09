/**
 * @file ex03.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-06-30
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "main.h"

#if defined(ICE05)
#include "drivers.h"

char APP_DESCRIPTION[] = "ECE353: ICE 05 - FreeRTOS Event Groups";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/
// ADD CODE for Event Group Bit Definitions

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
// ADD CODE for Event Group Handle

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/
void task_button_sw1(void *arg);
void task_button_sw2(void *arg);
void task_buzzer(void *arg);

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
void task_button_sw1(void *arg)
{
    (void)arg; // Unused parameter

    while (1)
    {
        // ADD CODE to detect button SW1 presses
    }
}

void task_button_sw2(void *arg)
{
    (void)arg; // Unused parameter

    while (1)
    {
        // ADD CODE to detect button SW2 presses
    }
}

void task_buzzer(void *arg)
{
    (void)arg; // Unused parameter
    
    while (1)
    {
        // ADD CODE to handle buzzer events
    }
}

/**
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 */
void app_init_hw(void)
{
    cy_rslt_t rslt;

    console_init();
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    /* ADD CODE Initialize the buttons */

    /* ADD CODE Initialize the buzzer */
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
    /* ADD CODE Create the event group */

    /* ADD CODE Register the tasks with FreeRTOS*/
    
    /* ADD CODE Start the scheduler*/

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}
#endif