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

#if defined(ICE06)
#include "drivers.h"
#include "task_joystick.h"

char APP_DESCRIPTION[] = "ECE353: ICE 06 - FreeRTOS Queues";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
extern QueueHandle_t Queue_position;

/* Create a lookup table to print out the joystick positions*/
const char *Joystick_Pos_Strings[] = {
    [JOYSTICK_POS_CENTER] = "Center",
    [JOYSTICK_POS_LEFT] = "Left",
    [JOYSTICK_POS_RIGHT] = "Right",
    [JOYSTICK_POS_UP] = "Up",
    [JOYSTICK_POS_DOWN] = "Down",
    [JOYSTICK_POS_UPPER_LEFT] = "Upper Left",
    [JOYSTICK_POS_UPPER_RIGHT] = "Upper Right",
    [JOYSTICK_POS_LOWER_LEFT] = "Lower Left",
    [JOYSTICK_POS_LOWER_RIGHT] = "Lower Right"
};

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
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

}

void task_print_directions(void *arg)
{

    //wait indefinitedly for messages from the joystick task
    joystick_position_t position;
    (void)arg; // Unused parameter
    while(1)
    {
        //block until a message is received
        if (xQueueReceive(Queue_position, &position, portMAX_DELAY) == pdTRUE)
        {
            printf("Joystick Position Changed: %s\n\r", Joystick_Pos_Strings[position]);
        }
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
    /* Initialize joystick resources */
    joystick_init();
    
    /* Register the tasks with FreeRTOS*/
    task_joystick_init();
    
    /* Create the print directions task */
    xTaskCreate(task_print_directions, "Print Directions", 3*configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);

    /* Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}
#endif