/**
 * @file i2c.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2025-08-06
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "i2c.h"

static cyhal_i2c_t i2c_monarch_obj;

// Define the I2C master configuration structure
cyhal_i2c_cfg_t i2c_monarch_config =
	{
		CYHAL_I2C_MODE_MASTER,
		0, // address is not used for master mode
		I2C_MASTER_FREQUENCY};

/** Initialize the I2C bus to the specified module site
 *
 * @param - None
 */
cyhal_i2c_t *i2c_init(cyhal_gpio_t sda, cyhal_gpio_t scl)
{
	cy_rslt_t rslt;

	// Initialize I2C master, set the SDA and SCL pins and assign a new clock
	rslt = cyhal_i2c_init(&i2c_monarch_obj, sda, scl, NULL);

	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("I2C initialization failed!\n\r");
		return NULL;
	}

	// Configure the I2C resource to be master
	rslt = cyhal_i2c_configure(&i2c_monarch_obj, &i2c_monarch_config);
	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("I2C configuration failed!\n\r");
		return NULL;
	}

	return &i2c_monarch_obj;
}

/**
 * @brief
 * Writes a single byte to the specified register of an I2C subordinate device
 * @param obj
 * @param subordinate_address
 * @param reg
 * @param value
 * @return cy_rslt_t
 */
cy_rslt_t i2c_write_u8(cyhal_i2c_t *obj, uint8_t subordinate_address, uint8_t reg, uint8_t value)
{
	cy_rslt_t rslt = CY_RSLT_SUCCESS;

	uint8_t data[2];
	data[0] = reg;
	data[1] = value;

	rslt = cyhal_i2c_master_write(obj, subordinate_address, data, 2, 0, true);

	// return if successful or not
	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("I2C write failed!\n\r");
		return rslt;
	}

	return rslt;
}

/**
 * @brief
 * Reads a single byte from the specified register of an I2C subordinate device
 * @param obj
 * @param subordinate_address
 * @param reg
 * @param value
 * @return cy_rslt_t
 */
cy_rslt_t i2c_read_u8(cyhal_i2c_t *obj, uint8_t subordinate_address, uint8_t reg, uint8_t *value)
{
	cy_rslt_t rslt = CY_RSLT_SUCCESS;

	uint8_t tx_data = reg;
	uint8_t rx_data = 0;

	rslt = cyhal_i2c_master_write(obj, subordinate_address, &tx_data, 1, 0, false);

	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("I2C write (for read) failed!\n\r");
		return rslt;
	}

	rslt = cyhal_i2c_master_read(obj, subordinate_address, &rx_data, 1, 0, true);

	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("I2C read failed!\n\r");
		return rslt;
	}

	*value = rx_data;

	return rslt;
}

/**
 * @brief
 * Reads a two from the specified register of an I2C subordinate device
 * @param obj
 * @param subordinate_address
 * @param reg
 * @param value
 * @return cy_rslt_t
 */
cy_rslt_t i2c_read_u16(cyhal_i2c_t *obj, uint8_t subordinate_address, uint8_t reg, uint16_t *value)
{
	cy_rslt_t rslt = CY_RSLT_SUCCESS;

	uint8_t tx_data = reg;
	uint8_t rx_data[2];

	rslt = cyhal_i2c_master_write(obj, subordinate_address, &tx_data, 1, 0, false);
	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("I2C write (for read) failed!\n\r");
		return rslt;
	}

	rslt = cyhal_i2c_master_read(obj, subordinate_address, rx_data, 2, 0, true);
	if (rslt != CY_RSLT_SUCCESS)
	{
		printf("I2C read failed!\n\r");
		return rslt;
	}

	*value = (uint16)rx_data[0] << 8 | rx_data[1];

	return rslt;
}