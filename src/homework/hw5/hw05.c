/**
 * @file hw05.c
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

#if defined(HW05)
#include "drivers.h"
#include "devices.h"
#include "task_console.h"
#include "task_io_expander.h"
#include "task_light_sensor.h"
#include "task_temp_sensor.h"
#include "task_eeprom.h"
#include "task_imu.h"

char APP_DESCRIPTION[] = "ECE353: HW05 - FreeRTOS CLI";

/*****************************************************************************/
/* Macros                                                                    */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/
cyhal_i2c_t *I2C_Obj = NULL;
cyhal_spi_t *SPI_Obj = NULL;

SemaphoreHandle_t Semaphore_I2C = NULL;
SemaphoreHandle_t Semaphore_SPI = NULL;

QueueHandle_t Queue_System_Control_Responses = NULL;
QueueHandle_t Queue_Sensor_Responses;
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
    Semaphore_I2C = xSemaphoreCreateMutex();
    if (Semaphore_I2C == NULL)
    {
        return false;
    }

    /* Create the SPI Semaphore */
    Semaphore_SPI = xSemaphoreCreateMutex();
    if (Semaphore_SPI == NULL)
    {
        return false;
    }

    /* Create the System Control Task */
    if (xTaskCreate(
            task_system_control,
            "System Control Task",
            configMINIMAL_STACK_SIZE * 5,
            NULL,
            tskIDLE_PRIORITY + 1,
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
    task_console_printf("Starting System Control Task\r\n");

    /* Configure the IO Expander*/
    system_sensors_io_expander_write(NULL, IOXP_ADDR_CONFIG, 0x80); // Set P7 as input, all others as outputs

    /* Set the initial state of the LEDs*/
    system_sensors_io_expander_write(NULL, IOXP_ADDR_OUTPUT_PORT, 0x01); // Turn on LED0

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
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
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    /* Initialize the spi interface */
    SPI_Obj = spi_init(PIN_SPI_MOSI, PIN_SPI_MISO, PIN_SPI_SCK);
    if (SPI_Obj == NULL)
    {
        printf("SPI initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    /* Configure the chip select pins for the EEPROM and IMU*/
    cyhal_gpio_init(PIN_EEPROM_CS, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);
    cyhal_gpio_init(PIN_IMU_CS, CYHAL_GPIO_DIR_OUTPUT, CYHAL_GPIO_DRIVE_STRONG, true);
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

    if (!task_system_control_resources_init())
    {
        printf("System Control Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_console_init())
    {
        printf("Console initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_io_expander_resources_init(I2C_Obj, &Semaphore_I2C))
    {
        printf("IO Expander Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_light_sensor_resources_init(I2C_Obj, &Semaphore_I2C))
    {
        printf("Light Sensor Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_eeprom_resources_init(&Semaphore_SPI, SPI_Obj, PIN_EEPROM_CS))
    {
        printf("EEPROM Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
        CY_ASSERT(0);
    }

    if (!task_imu_resources_init(&Semaphore_SPI, SPI_Obj, PIN_IMU_CS))
    {
        printf("IMU Task initialization failed!\n\r");
        for (int i = 0; i < 10000; i++)
            ;
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