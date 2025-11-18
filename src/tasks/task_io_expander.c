/**
 * @file io_expander.c
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2023-09-01
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "main.h"
#if defined(ECE353_FREERTOS)
#include "cyhal_gpio.h"
#include "task_io_expander.h"
#include "task_console.h"
#include "rtos_events.h"
#include "devices.h"

#define TASK_IO_EXPANDER_STACK_SIZE (configMINIMAL_STACK_SIZE)
#define TASK_IO_EXPANDER_PRIORITY (tskIDLE_PRIORITY + 1)

/******************************************************************************/
/* Function Declarations                                                      */
/******************************************************************************/
static void task_io_expander(void *param);
static void handler_io_expander_button(void *arg, cyhal_gpio_event_t event);

/******************************************************************************/
/* Global Variables                                                           */
/******************************************************************************/
static cyhal_i2c_t *I2C_Obj;
static SemaphoreHandle_t *I2C_Semaphore = NULL;

/* Queue used to send commands used to io expander */
QueueHandle_t Queue_IO_Expander_Requests;

/******************************************************************************/
/* Static Function Definitions                                                */
/******************************************************************************/

/******************************************************************************/
/* Public Function Definitions                                                */
/******************************************************************************/

bool system_sensors_io_expander_write(QueueHandle_t return_queue, uint8_t address, uint8_t value)
{
	cy_rslt_t rslt;
	bool status = true;

	rslt = i2c_write_u8(I2C_Obj, TCA9534_SUBORDINATE_ADDR, address, value);
	if (rslt != CY_RSLT_SUCCESS)
	{
		status = false;
	}

	return status;
}

bool system_sensors_io_expander_read(QueueHandle_t return_queue, uint8_t address, uint8_t *value)
{
	bool status = true;
	cy_rslt_t rslt;

	rslt = i2c_read_u8(I2C_Obj, TCA9534_SUBORDINATE_ADDR, address, value);
	if (rslt != CY_RSLT_SUCCESS)
	{
		status = false;
	}

	return status;
}

/**
 * @brief
 * Task used to monitor the reception of command packets sent the io expander
 * @param param
 * Unused
 */
void task_io_expander(void *param)
{
	uint32_t read_value = 0;

	task_console_printf("Starting IO Expander Task\r\n");

	while (1)
	{
		device_request_msg_t request_packet;
		device_response_msg_t response_packet;

		// wait for a request packet
		if (xQueueReceive(Queue_IO_Expander_Requests, &request_packet, portMAX_DELAY) == pdTRUE)
		{
			// grab the semaphore to access the bus
			xSemaphoreTake(*I2C_Semaphore, portMAX_DELAY);

			if (request_packet.operation == DEVICE_OP_WRITE)
			{
				system_sensors_io_expander_write(request_packet.response_queue, request_packet.address, request_packet.value);
			}
			else if (request_packet.operation == DEVICE_OP_READ)
			{
				system_sensors_io_expander_read(request_packet.response_queue, request_packet.address, (uint8_t *)&read_value);

				// prepare the response packet
				response_packet.device = DEVICE_IO_EXP;
				response_packet.status = DEVICE_OPERATION_STATUS_READ_SUCCESS;
				response_packet.payload.io_expander = (uint8_t)read_value;

				// send the response back if a return queue is provided
				if (request_packet.response_queue != NULL)
				{
					xQueueSend(request_packet.response_queue, &response_packet, portMAX_DELAY);
				}
			}

			// release the semaphore for the I2C bus
			xSemaphoreGive(*I2C_Semaphore);
		}
	}
}

/**
 * @brief
 * Initializes software resources related to the operation of
 * the IO Expander.  This function expects that the I2C bus had already
 * been initialized prior to the start of FreeRTOS.
 */
bool task_io_expander_resources_init(cyhal_i2c_t *i2c_obj, SemaphoreHandle_t *i2c_semaphore)
{
	/* Save the I2C object and semaphore */
	I2C_Obj = i2c_obj;
	I2C_Semaphore = i2c_semaphore;
	if (I2C_Semaphore == NULL)
	{
		return false;
	}

	/* Create the Queue used to send commands to the IO Expander*/
	Queue_IO_Expander_Requests = xQueueCreate(1, sizeof(device_request_msg_t));
	if (Queue_IO_Expander_Requests == NULL)
	{
		return false;
	}

	/* Create the task that will handle IO Expander requests */
	if (xTaskCreate(
			task_io_expander,
			"Task IO Exp",
			TASK_IO_EXPANDER_STACK_SIZE,
			i2c_semaphore,
			TASK_IO_EXPANDER_PRIORITY,
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