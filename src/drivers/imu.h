/**
 * @file imu.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-09-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __IMU_H__
#define __IMU_H__

#include <stdint.h>
#include <stdbool.h>

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cyhal_hw_types.h"
#include "spi.h"

#include <stdint.h>
#include <stdlib.h>


// LSM6DSMTR register addresses
#define IMU_REG_CTRL1_XL     0x10  // Accelerometer control
#define IMU_REG_CTRL2_G      0x11  // Gyroscope control
#define IMU_REG_OUTX_L_G     0x22  // Gyro output start
#define IMU_REG_OUTX_L_XL    0x28  // Accel X output low byte
#define IMU_REG_OUTX_H_XL    0x29  // Accel X output high byte
#define IMU_REG_OUTY_L_XL    0x2A  // Accel Y output low byte
#define IMU_REG_OUTY_H_XL    0x2B  // Accel Y output high byte
#define IMU_REG_OUTZ_L_XL    0x2C  // Accel Z output low byte
#define IMU_REG_OUTZ_H_XL    0x2D  // Accel Z output high byte
#define IMU_REG_WHO_AM_I     0x0F  // Device ID
#define IMU_REG_CTRL3_C      0x12  // Control register 3

// Configuration values
#define ODR_104HZ    0x40  // Output data rate = 104 Hz
#define FS_XL_2G     0x00  // ±2g
#define FS_G_250DPS  0x00  // ±250 dps

// Conversion factors
#define ACCEL_SENS_2G   (2.0f / 32768.0f)   // g/LSB
#define GYRO_SENS_250DPS (250.0f / 32768.0f) // dps/LSB

/**
 * @brief 
 *  This function initializes the IMU (LSM6DSMTR) sensor.
 *  The assumption is that the SPI peripheral has already been
 *  initialized.
 * @param spi_obj 
 * @param cs_pin 
 * @return true 
 * @return false 
 */
bool imu_init(
    cyhal_spi_t *spi_obj, 
    cyhal_gpio_t cs_pin
);

/**
 * @brief 
 *  This function writes a value to a specific register on the IMU (LSM6DSMTR) sensor.
 * @param spi_obj 
 * @param cs_pin 
 * @param reg 
 * @param value 
 */
void imu_write_reg(
    cyhal_spi_t *spi_obj, 
    cyhal_gpio_t cs_pin, 
    uint8_t reg, 
    uint8_t value
);

/**
 * @brief 
 *  This function reads a value from a specific register on the IMU (LSM6DSMTR) sensor.
 * @param spi_obj 
 * @param cs_pin 
 * @param reg 
 * @return uint8_t 
 */
uint8_t imu_read_reg(
    cyhal_spi_t *spi_obj, 
    cyhal_gpio_t cs_pin, 
    uint8_t reg
);

/**
 * @brief 
 * This function reads multiple registers from the IMU (LSM6DSMTR) sensor.
 * @param spi_obj 
 * @param cs_pin 
 * @param reg 
 * @param buffer 
 * @param length 
 */
void imu_read_registers(
    cyhal_spi_t *spi_obj, 
    cyhal_gpio_t cs_pin, 
    uint8_t reg, 
    uint8_t *buffer, 
    uint8_t length
);

#endif