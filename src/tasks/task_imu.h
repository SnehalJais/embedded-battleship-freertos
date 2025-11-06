/**
 * @file task_imu.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-09-16
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __TASK_IMU_H__
#define __TASK_IMU_H__

#include "cyhal_hw_types.h"
#include "main.h"
#if defined(ECE353_FREERTOS)
#include "cyhal_spi.h"
#include "imu.h"

#define TASK_IMU_PRIORITY        (tskIDLE_PRIORITY + 2)
#define TASK_IMU_STACK_SIZE      (1024)

bool task_imu_resources_init(void *spi_semaphore, cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin);
void task_imu(void *arg);
#endif /* ECE353_FREERTOS */

#endif /* __TASK_IMU_H__ */