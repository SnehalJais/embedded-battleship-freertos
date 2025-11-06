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

#if defined(ICE12)
#define EEPROM_TEST_SIZE 4
#include "drivers.h"
#include "task_console.h"
#include "task_eeprom.h"

#define TASK_SYSTEM_CONTROL_STACK_SIZE (configMINIMAL_STACK_SIZE * 5)
#define TASK_SYSTEM_CONTROL_PRIORITY   (tskIDLE_PRIORITY + 1)


char APP_DESCRIPTION[] = "ECE353: ICE 12 - FreeRTOS EEPROM";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
SemaphoreHandle_t SPI_Semaphore = NULL;
QueueHandle_t Queue_EEPROM_Response = NULL;
cyhal_spi_t *SPI_Obj = NULL;
cyhal_trng_t mTRNG;

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
    // Create a queue to receive the response from the EEPROM task
    Queue_EEPROM_Response = xQueueCreate(1, sizeof(device_response_msg_t));
    if(Queue_EEPROM_Response == NULL)
    {
        printf("Failed to create response queue!\r\n");
        vTaskDelay(pdMS_TO_TICKS(100));
        CY_ASSERT(0);
    }
    
    /* Create the SPI Semaphore */
    SPI_Semaphore = xSemaphoreCreateBinary();
    if(SPI_Semaphore == NULL)
    {
        printf("Failed to create SPI semaphore!\n\r");
        for(int i = 0; i < 10000; i++);
        CY_ASSERT(0);
    }

    /* Need to give the semaphore so that the first task that attempts to take
       the semaphore is successful */
    xSemaphoreGive(SPI_Semaphore);

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
 * Function used to implement the behavior of the System Control Task
 * @param arg 
 */
void task_system_control(void *arg)
{
    (void)arg; // Unused parameter
    uint8_t write_data[] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t read_data[4];
    bool return_status = false;

    device_request_msg_t request_packet;
    device_response_msg_t response_packet;

    printf("Starting System Control Task\r\n");

    printf("Writing to EEPROM\r\n");
    for(int i = 0; i < EEPROM_TEST_SIZE; i++)
    {
        return_status = system_sensors_eeprom_write(Queue_EEPROM_Response, i, write_data[i]);
        if(!return_status)
        {
            printf("EEPROM write request failed at address %d\r\n", i);
        }   
    }

    printf("Reading from EEPROM\r\n");
    
    for(int i = 0; i < EEPROM_TEST_SIZE; i++)
    {
        return_status = system_sensors_eeprom_read(Queue_EEPROM_Response, i, &read_data[i]);
        if(!return_status)
        {
            printf("EEPROM read request failed at address %d\r\n", i);
        }
    }

    // If there is a mismatch, print an error message to the console
    if (memcmp(write_data, read_data, EEPROM_TEST_SIZE) != 0)
    {
        printf("EEPROM data mismatch!\r\n");
    }
    else
    {
        printf("EEPROM data verified successfully.\r\n");
    }


    printf("System Control Task Completed!\r\n");
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
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

    /* Initialize the SPI interface */
    SPI_Obj = spi_init(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
    if(SPI_Obj == NULL)
    {
        printf("SPI initialization failed!\n\r");
        for(int i = 0; i < 10000; i++);
        CY_ASSERT(0);
    }

    /* Initialize the CS pin for the EEPROM */
    cyhal_gpio_init(PIN_SPI_EEPROM_CS, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, 1);  

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

    if(!task_eeprom_resources_init(&SPI_Semaphore, SPI_Obj, PIN_SPI_EEPROM_CS))
    {
        printf("EEPROM Task initialization failed!\n\r");
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