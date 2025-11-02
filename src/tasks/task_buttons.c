/**
 * @file task_buttons.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-13
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "task_buttons.h"
#include "task_console.h"

#ifdef ECE353_FREERTOS

// External reference to the global event group created in ice11.c
extern EventGroupHandle_t ECE353_RTOS_Events;
/**
 * @brief
 * Task used to debounce button presses (SW1, SW2, SW3).
 * The falling edge of the button press is detected by de-bouncing
 * the button for 30mS. Each button should be sampled every 15mS.
 *
 * When a button press is detected, the corresponding event is set in
 * in the event group ECE353_RTOS_Events.
 *
 * @param arg
 * Unused parameter
 */
void task_buttons(void *arg)
{
    (void)arg; // Unused parameter
    
    // Debounce counters for each button (need 2 consecutive samples for 30ms debounce)
    uint32_t sw1_count = 0;
    uint32_t sw2_count = 0;
    uint32_t sw3_count = 0;

    // Previous button states for edge detection (true = not pressed)
    bool prev_sw1_pressed = false;
    bool prev_sw2_pressed = false;
    bool prev_sw3_pressed = false;

    while (1)
    {
        // Current button states (0 = pressed)
        bool sw1_pressed = ((PORT_BUTTON_SW1->IN & MASK_BUTTON_PIN_SW1) == 0);
        bool sw2_pressed = ((PORT_BUTTON_SW2->IN & MASK_BUTTON_PIN_SW2) == 0);
        bool sw3_pressed = ((PORT_BUTTON_SW3->IN & MASK_BUTTON_PIN_SW3) == 0);

        // Monitor button SW1 with debouncing
        if (sw1_pressed)
        {
            sw1_count++;
            if (sw1_count >= 2 && !prev_sw1_pressed) // Falling edge after debounce
            {
                // Set the SW1_PRESSED event in the event group
                xEventGroupSetBits(ECE353_RTOS_Events, ECE353_RTOS_EVENTS_SW1);
                vTaskDelay(pdMS_TO_TICKS(30)); // Delay for debounce
                prev_sw1_pressed = true;
            }
        }
        else
        {
            sw1_count = 0;
            prev_sw1_pressed = false;
        }

        // Monitor button SW2 with debouncing
        if (sw2_pressed)
        {
            sw2_count++;
            if (sw2_count >= 2 && !prev_sw2_pressed) // Falling edge after debounce
            {
                // Set the SW2_PRESSED event in the event group
                xEventGroupSetBits(ECE353_RTOS_Events, ECE353_RTOS_EVENTS_SW2);

                vTaskDelay(pdMS_TO_TICKS(30)); // Delay for debounce
                prev_sw2_pressed = true;
            }
        }
        else
        {
            sw2_count = 0;
            prev_sw2_pressed = false;
        }

        // Monitor button SW3 with debouncing
        if (sw3_pressed)
        {
            sw3_count++;
            if (sw3_count >= 2 && !prev_sw3_pressed) // Falling edge after debounce
            {
                // Set the SW3_PRESSED event in the event group
                xEventGroupSetBits(ECE353_RTOS_Events, ECE353_RTOS_EVENTS_SW3);

                vTaskDelay(pdMS_TO_TICKS(30)); // Delay for debounce
                prev_sw3_pressed = true;
            }
        }
        else
        {
            sw3_count = 0;
            prev_sw3_pressed = false;
        }

        // Sample every 15ms (2 samples = 30ms debounce time)
        vTaskDelay(pdMS_TO_TICKS(15));
    }
} /* Button Task Initialization */
bool task_button_init(void)
{

    BaseType_t result;
    // cy_rslt_t rslt;

    // // initialize the buttons
    // rslt = buttons_init_gpio();
    // if (rslt != CY_RSLT_SUCCESS)
    // {
    //     return false;
    // }

    // Create the button task
    result = xTaskCreate(
        task_buttons,
        "Button Task",
        configMINIMAL_STACK_SIZE,
        NULL,
        tskIDLE_PRIORITY + 1,
        NULL);

    if (result != pdPASS)
    {
        return false;
    }

    return true;
}
#endif