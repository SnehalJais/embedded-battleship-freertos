/**
 * @file devices.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-10-24
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __DEVICES_H__
#define __DEVICES_H__

#include "main.h"
#if defined(ECE353_FREERTOS)

typedef enum {
    DEVICE_IMU = 0,
    DEVICE_TEMPERATURE,
    DEVICE_LIGHT,
    DEVICE_IO_EXP,
    DEVICE_EEPROM,
    DEVICE_UNKNOWN
} device_type_t;

typedef enum {
    DEVICE_OP_READ,
    DEVICE_OP_WRITE
} device_operation_t ;

typedef enum {
    DEVICE_OPERATION_STATUS_READ_SUCCESS,
    DEVICE_OPERATION_STATUS_READ_FAILURE,
    DEVICE_OPERATION_STATUS_WRITE_SUCCESS,
    DEVICE_OPERATION_STATUS_WRITE_FAILURE
} device_operation_status_t;

typedef struct {
    device_type_t device;
    device_operation_t operation;
    uint16_t address;
    uint8_t value;
    QueueHandle_t response_queue;
} device_request_msg_t ;

// Data structure for receiving data from the device gatekeeper tasks
typedef struct {
    device_type_t device;
    device_operation_status_t status;
    union {
        float temperature;
        uint16_t light_sensor;
        uint8_t io_expander;
        uint8_t eeprom;
        uint16_t imu[3];
    } payload;
} device_response_msg_t ;

#endif
#endif /* __DEVICES_H__ */