/**
 * @file task_temp_sensor.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-09-18
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "main.h"

#if defined(ECE353_FREERTOS)
#include "cy_result.h"
#include "drivers.h"
#include "task_temp_sensor.h"
#include "task_console.h"

#define TASK_TEMP_SENSOR_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define TASK_TEMP_SENSOR_PRIORITY (tskIDLE_PRIORITY + 1)

/******************************************************************************/
/* Function Declarations                                                      */
/******************************************************************************/

/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/
/* I2C Object Handle */
static cyhal_i2c_t *I2C_Obj;

/* I2C Semaphore */
static SemaphoreHandle_t *I2C_Semaphore = NULL;

/* Queue used to send commands used to temp sensor */
QueueHandle_t Queue_Temp_Sensor_Requests;

/******************************************************************************/
/* Static Function Definitions                                                */
/******************************************************************************/

/** Read the value of the input port
 *
 * @param reg The reg address to read
 *
 */
static float LM75_get_temp(void)
{
	float temp = 0.0f;
	uint16_t raw_value = 0;
	cy_rslt_t rslt;

	// read two bytes from the temperature register
	rslt = i2c_read_u16(I2C_Obj, LM75_SUBORDINATE_ADDR, LM75_TEMP_REG, (uint16_t *)&raw_value);
	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("LM75: Failed to read temperature register\r\n");
		return temp;
	}
	// need to format the raw value into a temperature
	// the temperature is a 9-bit two's complement value
	temp = (float)(raw_value >> 7); // shift off the unused bits

	// convert raw value to celsius
	// each bit is 0.5 degrees celsius
	temp *= 0.5f;

	return temp;
}

static uint8_t LM75_get_product_id(void)
{
	uint8_t prod_id = 0;
	cy_rslt_t rslt;
	// read the product ID register
	rslt = i2c_read_u8(I2C_Obj, LM75_SUBORDINATE_ADDR, LM75_PRODUCT_ID, &prod_id);

	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("LM75: Failed to read product ID register\r\n");
		return 0;
	}

	return prod_id;
}

/******************************************************************************/
/* Public Function Definitions                                                */
/******************************************************************************/

/**
 * @brief
 * This is a helper function that is called by tasks other than the temp sensor task
 * when they want to read the temperature from the sensor.
 */
bool system_sensors_get_temp(QueueHandle_t return_queue, float *temperature)
{
	device_request_msg_t request_packet;
	device_response_msg_t response_packet;

	if (return_queue == NULL || temperature == NULL)
	{
		return false;
	}

	// fill out the request packet
	request_packet.device = DEVICE_TEMPERATURE;
	request_packet.operation = DEVICE_OP_READ;
	request_packet.response_queue = return_queue;

	// send the request to the temp sensor task
	xQueueSend(Queue_Temp_Sensor_Requests, &request_packet, portMAX_DELAY);

	// wait for the response from the temp sensor task
	xQueueReceive(return_queue, &response_packet, pdMS_TO_TICKS(100));

	// return the temperature value via the data pointer
	*temperature = response_packet.payload.temperature;

	if (response_packet.status == DEVICE_OPERATION_STATUS_READ_SUCCESS)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * @brief
 * Task used to monitor the reception of command packets sent the temp sensor
 * @param param
 * Unused
 */
void task_temp_sensor(void *param)
{
	device_request_msg_t request_packet;
	device_response_msg_t response_packet;

	printf("Starting Temp Sensor Task\r\n");

	// Take the I2C semaphore to configure the temp sensor
	xSemaphoreTake(*I2C_Semaphore, portMAX_DELAY);

	// Verify that the temp sensor is connected by reading the product ID
	uint8_t prod_id = LM75_get_product_id();
	// give the semaphore back
	xSemaphoreGive(*I2C_Semaphore);

	while (1)
	{
		// Wait for a request from EX13
		temp_sensor_packet_t legacy_request;
		if (xQueueReceive(Queue_Temp_Sensor_Requests, &legacy_request, portMAX_DELAY) == pdTRUE)
		{
			if (legacy_request.operation == TEMP_SENSOR_READ)
			{
				// grab the semaphore for the I2C bus
				xSemaphoreTake(*I2C_Semaphore, portMAX_DELAY);

				// Read temperature from sensor
				float temperature = LM75_get_temp();

				// release the semaphore for the I2C bus
				xSemaphoreGive(*I2C_Semaphore);

				// Send response back to EX13
				legacy_request.operation = TEMP_SENSOR_RESPONSE;
				legacy_request.value = temperature;
				
				if (legacy_request.return_queue != NULL)
				{
					xQueueSend(legacy_request.return_queue, &legacy_request, portMAX_DELAY);
				}
			}
		}
	}
}

/**
 * @brief
 * Initializes software resources related to the operation of
 * the Temp Sensor.  This function expects that the I2C bus had already
 * been initialized prior to the start of FreeRTOS.
 */
bool task_temp_sensor_resources_init(cyhal_i2c_t *i2c_obj, SemaphoreHandle_t *i2c_semaphore)
{
	/* Save the I2C object and semaphore */
	I2C_Obj = i2c_obj;
	I2C_Semaphore = i2c_semaphore;

	if (I2C_Semaphore == NULL || I2C_Obj == NULL)
	{
		return false;
	}

	/* Create the Queue used to receive requests  */
	Queue_Temp_Sensor_Requests = xQueueCreate(1, sizeof(temp_sensor_packet_t));
	if (Queue_Temp_Sensor_Requests == NULL)
	{
		return false;
	}

	/* Create the task that will control the status LED */
	if (xTaskCreate(
			task_temp_sensor,
			"Temp Sensor",
			TASK_TEMP_SENSOR_STACK_SIZE,
			i2c_semaphore,
			TASK_TEMP_SENSOR_PRIORITY,
			NULL) != pdPASS)
	{
		return false;
	}
	else
	{
		return true;
	}
}
#endif