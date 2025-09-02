/**
 * @file task_lcd.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-08-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #include "task_lcd.h"

#ifdef ECE353_FREERTOS  
/* FreeRTOS Queue for LCD messages */
QueueHandle_t xQueue_LCD;

/* LCD Task */
void task_lcd(void *pvParameters)
{
    (void)pvParameters; // Unused parameter


    while(1)
    {
    }
}

/* LCD Task Initialization */
bool task_lcd_init(void){

    BaseType_t result;
    xQueue_LCD = xQueueCreate(10, sizeof(lcd_msg_t));
    if(xQueue_LCD == NULL)
    {
        return false;
    }

    result= xTaskCreate(
        task_lcd,                       // Task function
        "LCD Task",                     // Task name
        5*configMINIMAL_STACK_SIZE,    // Stack size
        NULL,                           // Task parameters
        tskIDLE_PRIORITY+1,              // Task priority
        NULL                            // Task handle
    );

    if(result != pdPASS)
    {
        return false;
    }   

    return true;
}
#endif