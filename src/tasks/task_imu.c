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
#include "devices.h"

#define TASK_IMU_STACK_SIZE (configMINIMAL_STACK_SIZE * 5)
#define TASK_IMU_PRIORITY (tskIDLE_PRIORITY + 1)

// Global Variables
QueueHandle_t Queue_IMU_Requests;
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

  // create the IMU Requests Queue
  Queue_IMU_Requests = xQueueCreate(1, sizeof(device_request_msg_t));
  if (!Queue_IMU_Requests)
  {
    return false;
  }
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
  device_request_msg_t request_packet;
  device_response_msg_t response_packet;
  // array to store the raw acceleration data
  int16_t accel_data[3];

  task_console_printf("Starting IMU Task\r\n");

  // take the SPI semaphore before initializing the IMU
  xSemaphoreTake(*SPI_Semaphore, portMAX_DELAY);

  // initialize the IMU
  if (!imu_init(imu_spi_obj, imu_cs_pin))
  {
    vTaskSuspend(NULL);
  }
  else
  {
    
  }

  // give the SPI semaphore after initializing the IMU
  xSemaphoreGive(*SPI_Semaphore);

  while (1)
  {
    // wait for a request from IMU queue
    xQueueReceive(Queue_IMU_Requests, &request_packet, portMAX_DELAY);
    if (request_packet.operation == DEVICE_OP_READ)
    {
      // take the SPI semaphore before reading from the IMU
      xSemaphoreTake(*SPI_Semaphore, portMAX_DELAY);

      // read the acceleration data from the IMU
      imu_read_registers(imu_spi_obj, imu_cs_pin, IMU_REG_OUTX_L_XL, (uint8_t *)accel_data, 6);

      // give the SPI semaphore after reading from the IMU
      xSemaphoreGive(*SPI_Semaphore);

      // prepare the response packet
      response_packet.device = DEVICE_IMU;
      response_packet.status = DEVICE_OPERATION_STATUS_READ_SUCCESS;
      response_packet.payload.imu[0] = accel_data[0]; // field names must match devices.h
      response_packet.payload.imu[1] = accel_data[1];
      response_packet.payload.imu[2] = accel_data[2];

      // send the response back if a return queue is provided
      if (request_packet.response_queue != NULL)
      {
        xQueueSend(request_packet.response_queue, &response_packet, portMAX_DELAY);
      }
    }
    else if (request_packet.operation == DEVICE_OP_WRITE)
    {
      // take the SPI semaphore before writing to the IMU
      xSemaphoreTake(*SPI_Semaphore, portMAX_DELAY);

      // write to the IMU register
      imu_write_reg(imu_spi_obj, imu_cs_pin, request_packet.address, request_packet.value);

      // give the SPI semaphore after writing to the IMU
      xSemaphoreGive(*SPI_Semaphore);

      // prepare the response packet
      response_packet.device = DEVICE_IMU;
      response_packet.status = DEVICE_OPERATION_STATUS_WRITE_SUCCESS;

      // send the response back if a return queue is provided
      if (request_packet.response_queue != NULL)
      {
        xQueueSend(request_packet.response_queue, &response_packet, portMAX_DELAY);
      }
    }
  }
}

/**
 * @brief
 * This function acts as the published interface to read data from the IMU.
 * It will format the device request message and send it to the IMU task.
 *
 * If the user provides a valid return queue, this function will wait for
 * the response from the IMU task before returning. If the user provides
 * a NULL return queue, this function will return immediately after sending
 * the request.
 *
 * @param return_queue Queue to receive the response (can be NULL)
 * @param imu_data Pointer to array to store IMU data [x, y, z]
 * @return true if operation was successful, false otherwise
 */
bool system_sensors_imu_read(QueueHandle_t return_queue, uint16_t *imu_data)
{
  bool status = false;
  device_request_msg_t request_packet;
  device_response_msg_t response_packet;

  // Setup the request packet
  request_packet.device = DEVICE_IMU;
  request_packet.operation = DEVICE_OP_READ;
  request_packet.address = 0; // Not used for IMU
  request_packet.value = 0;   // Not used for IMU
  request_packet.response_queue = return_queue;

  // Send the request to the IMU task
  if (xQueueSend(Queue_IMU_Requests, &request_packet, portMAX_DELAY) == pdTRUE)
  {
    // If return queue provided, wait for response
    if (return_queue != NULL && imu_data != NULL)
    {
      // Wait for the response from the IMU task
      if (xQueueReceive(return_queue, &response_packet, pdMS_TO_TICKS(100)) == pdTRUE)
      {
        // Check the status of the response packet
        if (response_packet.status == DEVICE_OPERATION_STATUS_READ_SUCCESS)
        {
          imu_data[0] = response_packet.payload.imu[0];
          imu_data[1] = response_packet.payload.imu[1];
          imu_data[2] = response_packet.payload.imu[2];
          status = true;
        }
      }
    }
    else
    {
      // No return queue provided, assume success
      status = true;
    }
  }

  return status;
}

/**
 * @brief
 * This function acts as the published interface to write data to the IMU.
 * It will format the device request message and send it to the IMU task.
 *
 * If the user provides a valid return queue, this function will wait for
 * the response from the IMU task before returning. If the user provides
 * a NULL return queue, this function will return immediately after sending
 * the request.
 *
 * @param return_queue Queue to receive the response (can be NULL)
 * @param address IMU register address to write to
 * @param value Value to write to the register
 * @return true if operation was successful, false otherwise
 */
bool system_sensors_imu_write(QueueHandle_t return_queue, uint16_t address, uint8_t value)
{
  bool status = false;
  device_request_msg_t request_packet;
  device_response_msg_t response_packet;

  // Setup the request packet
  request_packet.device = DEVICE_IMU;
  request_packet.operation = DEVICE_OP_WRITE;
  request_packet.address = address;
  request_packet.value = value;
  request_packet.response_queue = return_queue;

  // Send the request to the IMU task
  if (xQueueSend(Queue_IMU_Requests, &request_packet, portMAX_DELAY) == pdTRUE)
  {
    // If return queue provided, wait for response
    if (return_queue != NULL)
    {
      // Wait for the response from the IMU task
      if (xQueueReceive(return_queue, &response_packet, pdMS_TO_TICKS(100)) == pdTRUE)
      {
        // Check the status of the response packet
        if (response_packet.status == DEVICE_OPERATION_STATUS_WRITE_SUCCESS)
        {
          status = true;
        }
      }
    }
    else
    {
      // No return queue provided, assume success
      status = true;
    }
  }

  return status;
}
#endif /* ECE353_FREERTOS */