/**
 * @file task_light_sensor.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-09-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */
 #ifndef __TASK_LIGHT_SENSOR_H__
 #define __TASK_LIGHT_SENSOR_H__

#include "main.h"

#if defined (ECE353_FREERTOS)

#include "drivers.h"
#include "devices.h"

#define LTR_SUBORDINATE_ADDR   0x00

#define LTR_REG_CONTR           0x80
#define LTR_REG_MEAS_RATE       0x85
#define LTR_REG_PART_ID         0x86
#define LTR_REG_MANUFAC_ID      0x87
#define LTR_REG_ALS_DATA_CH1_0  0x88
#define LTR_REG_ALS_DATA_CH1_1  0x89
#define LTR_REG_ALS_DATA_CH0_0  0x8A
#define LTR_REG_ALS_DATA_CH0_1  0x8B
#define LTR_REG_ALS_STATUS      0x8C

#define LTR_REG_CONTR_SW_RESET  (1 << 1)
#define LTR_REG_CONTR_ALS_MODE  (1 << 0)

#define LTR_REG_STATUS_NEW_DATA     (1 << 2)
#define LTR_REG_STATUS_VALID_DATA   (1 << 7)

extern QueueHandle_t Queue_Light_Sensor_Requests;

bool system_sensors_get_light(QueueHandle_t return_queue, uint16_t *ambient_light);
bool task_light_sensor_resources_init(cyhal_i2c_t *i2c_obj, SemaphoreHandle_t *i2c_semaphore);
void task_light_sensor(void *arg);

#endif

#endif
