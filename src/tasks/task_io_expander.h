/*
 * opt3001.h
 *
 *  Created on: Oct 20, 2020
 *      Author: Joe Krachey
 */

#ifndef __TASK_IO_EXPANDER_H__
#define __TASK_IO_EXPANDER_H__

#include "main.h"

#if defined (ECE353_FREERTOS)
#include "drivers.h"

#define TCA9534_SUBORDINATE_ADDR        0x00
#define IOXP_ADDR_INPUT_PORT            0x00
#define IOXP_ADDR_OUTPUT_PORT           0x01
#define IOXP_ADDR_POLARITY              0x02
#define IOXP_ADDR_CONFIG                0x03
#define IOXP_ADDR_INVALID               0xFF


extern QueueHandle_t Queue_IO_Expander_Requests;

/* Functions used to interact with the IO Expander */
bool system_sensors_io_expander_write(QueueHandle_t return_queue, uint8_t address, uint8_t value);
bool system_sensors_io_expander_read(QueueHandle_t return_queue, uint8_t address, uint8_t *value);

/* Function used to initialize resources for the IO Expander task */
bool task_io_expander_resources_init(cyhal_i2c_t *i2c_obj, SemaphoreHandle_t *i2c_semaphore);

#endif

#endif
