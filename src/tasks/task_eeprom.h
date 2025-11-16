/**
 * @file task_eeprom.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-09-17
 *
 * @copyright Copyright (c) 2025
 *
 */
#ifndef __TASK_EEPROM_H__
#define __TASK_EEPROM_H__
#include "main.h"

#if defined(ECE353_FREERTOS)

#include "drivers.h"
#include "devices.h"
#include "rtos_events.h"

extern QueueHandle_t Queue_EEPROM_Requests;

/* Functions used to interact with the EEPROM */
bool system_sensors_eeprom_write(QueueHandle_t return_queue, uint16_t address, uint8_t data);
bool system_sensors_eeprom_read(QueueHandle_t return_queue, uint16_t address, uint8_t *data);

/* Function used to initialize resources for the EEPROM task */
bool task_eeprom_resources_init(cyhal_spi_t *spi_obj, SemaphoreHandle_t *spi_semaphore, cyhal_gpio_t cs_pin);

#endif /* ECE353_FREERTOS */

#endif /* __TASK_EEPROM_H__ */