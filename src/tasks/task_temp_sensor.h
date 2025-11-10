/**
 * @file task_temp_sensor.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-09-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef __TASK_TEMP_SENSOR_H__
#define __TASK_TEMP_SENSOR_H__

#include "main.h"

#if defined (ECE353_FREERTOS)
#include "drivers.h"
#include "devices.h"

#define LM75_SUBORDINATE_ADDR                 0x4F
#define LM75_TEMP_REG						  0x00

#define LM75_PRODUCT_ID                      0xA1

extern QueueHandle_t Queue_Temp_Sensor_Requests;

/* Functions used to interact with the Temp Sensor */
bool system_sensors_get_temp(QueueHandle_t return_queue, float *temperature);

/* Function used to initialize resources for the Temp Sensor task */
bool task_temp_sensor_resources_init(cyhal_i2c_t *i2c_obj, SemaphoreHandle_t *i2c_semaphore);

#endif

#endif
