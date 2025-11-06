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
#include "cyhal_hw_types.h"
#include "main.h"

#if defined(ICE14)
#include "drivers.h"
#include "task_console.h"
#include "task_io_expander.h"

#define TASK_SYSTEM_CONTROL_STACK_SIZE (configMINIMAL_STACK_SIZE * 5)
#define TASK_SYSTEM_CONTROL_PRIORITY   (tskIDLE_PRIORITY + 1)

char APP_DESCRIPTION[] = "ECE353: ICE 14 - FreeRTOS IO Expander";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
cyhal_i2c_t *I2C_Obj = NULL;
SemaphoreHandle_t I2C_Semaphore = NULL;
QueueHandle_t Queue_IO_Expander_Responses = NULL;

/*****************************************************************************/
/* Function Declarations                                                     */
/*****************************************************************************/
void task_system_control(void *arg);

/*****************************************************************************/
/* Function Definitions                                                      */
/*****************************************************************************/
/**
 * @brief 
 * This function will initialize all of the software resources for the 
 * System Control Task
 * @return true 
 * @return false 
 */
bool task_system_control_resources_init(void)
{
    /* Create the I2C Semaphore */
    I2C_Semaphore = xSemaphoreCreateMutex();
    if (I2C_Semaphore == NULL)
    {
        return false;
    }

    /* Create the System Control Task */
    if (xTaskCreate(
            task_system_control,
            "System Control Task",
            TASK_SYSTEM_CONTROL_STACK_SIZE,
            NULL,
            TASK_SYSTEM_CONTROL_PRIORITY,
            NULL) != pdPASS)
    {
        return false;
    }

    return true;
}

/**
 * @brief 
 * This function implements the behavioral requirements for the ICE 
 * @param arg 
 */
void task_system_control(void *arg)
{
    (void)arg; // Unused parameter
    uint8_t led_value = 0x01;
    task_console_printf("Starting System Control Task\r\n");

    /* Configure the IO Expander*/
    system_sensors_io_expander_write(NULL, IOXP_ADDR_CONFIG, 0x80); //Set P7 as input, all others as outputs
    
    /* Set the initial state of the LEDs*/
    system_sensors_io_expander_write(NULL, IOXP_ADDR_OUTPUT_PORT, 0x01); //Turn on LED0

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        
        /* Update the LED being enabled */
        if(led_value == 0x80)
        {
            led_value = 0x01;
        }
        else
        {
            led_value = led_value << 1;
        }

        // Write the new LED value
        system_sensors_io_expander_write(NULL, IOXP_ADDR_OUTPUT_PORT, led_value);

    }
}

/**
 * @brief
 * This function will initialize all of the hardware resources for
 * the ICE
 */
void app_init_hw(void)
{
    console_init();
    printf("**************************************************\n\r");
    printf("* %s\n\r", APP_DESCRIPTION);
    printf("* Date: %s\n\r", __DATE__);
    printf("* Time: %s\n\r", __TIME__);
    printf("* Name:%s\n\r", NAME);
    printf("**************************************************\n\r");

    /* Initialize the i2c interface */
    I2C_Obj = i2c_init(PIN_I2C_SDA, PIN_I2C_SCL);
    if (I2C_Obj == NULL)
    {
        printf("I2C initialization failed!\n\r");
        for(int i = 0; i < 10000; i++);
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

    if(!task_system_control_resources_init())
    {
        printf("System Control Task initialization failed!\n\r");
        for(int i = 0; i < 10000; i++);
        CY_ASSERT(0);
    }

    if(!task_console_init())
    {
        printf("Console initialization failed!\n\r");
        for(int i = 0; i < 10000; i++);
        CY_ASSERT(0);
    }

    if(!task_io_expander_resources_init(I2C_Obj, &I2C_Semaphore))
    {
        printf("IO Expander Task initialization failed!\n\r");
        for(int i = 0; i < 10000; i++);
        CY_ASSERT(0);
    }

    /* Start the scheduler*/
    vTaskStartScheduler();

    /* Will never reach this loop once the scheduler starts */
    while (1)
    {
    }
}

#endif