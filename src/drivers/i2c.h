/**
 * @file i2c.h
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-09-17
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#ifndef __I2C_H__
#define __I2C_H__

#include "main.h"

#if defined(ECE353_FREERTOS)
#include "cyhal_i2c.h"
#include "cyhal_gpio.h"
#include "drivers.h"
#include "ece353-pins.h"

/* Macros */
#define I2C_MASTER_FREQUENCY 100000u

/* Public API */

/** Initialize the I2C bus to the specified module site
 *
 * @param - None
 */
cyhal_i2c_t * i2c_init(cyhal_gpio_t sda, cyhal_gpio_t scl);

/**
 * @brief 
 * Writes a single byte to the specified register of an I2C subordinate device
 * @param obj 
 * @param subordinate_address 
 * @param reg 
 * @param value 
 * @return cy_rslt_t 
 */
cy_rslt_t i2c_write_u8(cyhal_i2c_t *obj, uint8_t subordinate_address, uint8_t reg, uint8_t value);

/**
 * @brief 
 * Reads a single byte from the specified register of an I2C subordinate device
 * @param obj 
 * @param subordinate_address 
 * @param reg 
 * @param value 
 * @return cy_rslt_t 
 */
cy_rslt_t i2c_read_u8(cyhal_i2c_t *obj, uint8_t subordinate_address, uint8_t reg, uint8_t *value);

/**
 * @brief 
 * Reads two bytes from the specified register of an I2C subordinate device
 * @param obj 
 * @param subordinate_address 
 * @param reg 
 * @param value 
 * @return cy_rslt_t 
 */
cy_rslt_t i2c_read_u16(cyhal_i2c_t *obj, uint8_t subordinate_address, uint8_t reg, uint16_t *value);

#endif /* ECE353_FREERTOS */

#endif /* I2C_H_ */
