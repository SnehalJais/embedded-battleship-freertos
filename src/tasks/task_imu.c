/**
 * @file task_imu.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-09-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "task_imu.h"

#if defined(ECE353_FREERTOS)
#include "imu.h"
#include "task_console.h"

#define TASK_IMU_STACK_SIZE (configMINIMAL_STACK_SIZE * 5)
#define TASK_IMU_PRIORITY (tskIDLE_PRIORITY + 1)

static SemaphoreHandle_t *SPI_Semaphore = NULL;
static cyhal_spi_t *imu_spi_obj = NULL;
static cyhal_gpio_t imu_cs_pin = NC;

/**
 * @brief
 * This function will create the IMU task for reading data from the IMU sensor.
 * It assumes that you have already created a semaphore for SPI access and initialized
 * the SPI peripheral.  This function does NOT initialize the SPI peripheral OR CS Pin
 * because the SPI peripheral is shared between multiple tasks (e.g. IMU, EEPROM, etc.).
 * @param spi_semaphore
 * @return true
 * @return false
 */
bool task_imu_resources_init(void *spi_semaphore, cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin)
{
  SPI_Semaphore = (SemaphoreHandle_t *)spi_semaphore;
  imu_spi_obj = spi_obj;
  imu_cs_pin = cs_pin;

  // create the IMU task
  if (xTaskCreate(
          task_imu,            // Function that implements the task.
          "IMU Task",          // Text name for the task.
          TASK_IMU_STACK_SIZE, // Stack size in words, not bytes.
          NULL,                // Parameter passed into the task.
          TASK_IMU_PRIORITY,   // Priority at which the task is created.
          NULL                 // Handle to the created task.
          ) != pdPASS)
  {
    return false;
  }

  return true;
}

void task_imu(void *arg)
{
  (void)arg;

  // array to store the raw acceleration data
  int16_t accel_data[3];

  // take the SPI semaphore before initializing the IMU
  xSemaphoreTake(*SPI_Semaphore, portMAX_DELAY);

  // initialize the IMU
  if (!imu_init(imu_spi_obj, imu_cs_pin))
  {
    printf("IMU Initialization Failed!\r\n");
    vTaskSuspend(NULL);
  }
  else
  {
    printf("IMU Initialization Succeeded!\r\n");
  }

  // give the SPI semaphore after initializing the IMU
  xSemaphoreGive(*SPI_Semaphore);

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(250));

    xSemaphoreTake(*SPI_Semaphore, portMAX_DELAY);

    imu_read_registers(imu_spi_obj, imu_cs_pin, IMU_REG_OUTX_L_XL, (uint8_t *)accel_data, 6);

    xSemaphoreGive(*SPI_Semaphore);

    // print raw acceleration data to console
    printf("Accel X: %d, Accel Y: %d\r\n",
           accel_data[0], accel_data[1]);
  }
}
#endif /* ECE353_FREERTOS */