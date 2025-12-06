/**
 * @file task_joystick.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "main.h"

#ifdef ECE353_FREERTOS
#include "drivers.h"
#include "task_joystick.h"

QueueHandle_t Queue_Joystick = NULL;
QueueHandle_t Queue_position = NULL;

/* Message lookup table for joystick positions */
const char *const joystick_pos_names[] = {
    "Center",
    "Left",
    "Right",
    "Up",
    "Down",
    "Upper Left",
    "Upper Right",
    "Lower Left",
    "Lower Right"};

/**
 * @brief
 *  Task used to monitor the joystick
 * @param arg
 */
void task_joystick(void *arg)
{
    uint16_t x_value, y_value;
    joystick_position_t current_position;
    static joystick_position_t previous_position = JOYSTICK_POS_CENTER;
    (void)arg; // Unused parameter
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(10)); // Poll every 10ms for faster response
        x_value = joystick_read_x();
        y_value = joystick_read_y();

        float x_volatage = (x_value / 65535.0) * 3.3;
        float y_volatage = (y_value / 65535.0) * 3.3;

        // printf("X: %f, Y: %f\r\n", x_volatage, y_volatage);

        // Get the current joystick position
        current_position = joystick_get_pos();

        // Add series of if/else if statements to detect joystick direction
        if (current_position == JOYSTICK_POS_CENTER)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_CENTER]);
        }
        else if (current_position == JOYSTICK_POS_LEFT)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_LEFT]);
        }
        else if (current_position == JOYSTICK_POS_RIGHT)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_RIGHT]);
        }
        else if (current_position == JOYSTICK_POS_UP)
        {
            printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_UP]);
        }
        else if (current_position == JOYSTICK_POS_DOWN)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_DOWN]);
        }
        else if (current_position == JOYSTICK_POS_UPPER_LEFT)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_UPPER_LEFT]);
        }
        else if (current_position == JOYSTICK_POS_UPPER_RIGHT)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_UPPER_RIGHT]);
        }
        else if (current_position == JOYSTICK_POS_LOWER_LEFT)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_LOWER_LEFT]);
        }
        else if (current_position == JOYSTICK_POS_LOWER_RIGHT)
        {
            // printf("Joystick Position: %s\r\n", joystick_pos_names[JOYSTICK_POS_LOWER_RIGHT]);
        }

        // If the current position is not equal to the previous position, add the new position to the joystick queue
        if (current_position != previous_position)
        {
            previous_position = current_position;
            // Send position to queue for other tasks to use
            xQueueOverwrite(Queue_position, &current_position);
        }
    }
}

bool task_joystick_init(void)
{
    /* Create the Queue used to send Joystick Positions*/
    Queue_position = xQueueCreate(1, sizeof(joystick_position_t));
    if (Queue_position == NULL)
    {
        return false;
    }

    /* Create the joystick task */
    xTaskCreate(task_joystick, "Joystick Task", 5 * configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL);
    return true;
}
#endif