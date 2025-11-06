/**
 * @file eeprom.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief
 * @version 0.1
 * @date 2023-10-24
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "eeprom.h"
#include "cyhal_hw_types.h"
#include <sys/types.h>

/** Determine if the EEPROM is busy writing the last
 *  transaction to non-volatile storage
 *
 * @param
 *
 */
void eeprom_wait_for_write(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin)
{
	uint8_t status = 0;
	do
	{
		uint8_t tx_buffer[2];
		uint8_t rx_buffer[2];
		tx_buffer[0] = EEPROM_CMD_RDSR; // Read Status Register command
		tx_buffer[1] = 0xFF;			// Dummy

		// Assert CS pin
		cyhal_gpio_write(cs_pin, 0);

		// Send SPI transaction to read status register
		cyhal_spi_transfer(spi_obj, tx_buffer, 2, rx_buffer, 2, 0xFF);

		// De-assert CS pin
		cyhal_gpio_write(cs_pin, 1);

		// Extract status from received data
		status = rx_buffer[1];

		// Small delay between status checks
		cyhal_system_delay_ms(1);

	} while (status & 0x01); // Continue while Write In Progress bit is set
}
/** Enables Writes to the EEPROM
 *
 * @param
 *
 */
void eeprom_write_enable(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin)
{
	uint8_t tx_buffer[1];
	uint8_t rx_buffer[1];
	tx_buffer[0] = EEPROM_CMD_WREN; // Write Enable command
	// Assert CS pin
	cyhal_gpio_write(cs_pin, 0);

	// Transmit data over SPI
	cyhal_spi_transfer(spi_obj, tx_buffer, 1, rx_buffer, 1, 0xFF);

	// De-assert CS pin
	cyhal_gpio_write(cs_pin, 1);
}

/** Disable Writes to the EEPROM
 *
 * @param
 *
 */
void eeprom_write_disable(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin)
{
	uint8_t tx_buffer[1];
	uint8_t rx_buffer[1];

	tx_buffer[0] = EEPROM_CMD_WRDI; // Write Disable command
	// Assert CS pin
	cyhal_gpio_write(cs_pin, 0);

	// Transmit data over SPI
	cyhal_spi_transfer(spi_obj, tx_buffer, 1, rx_buffer, 1, 0xFF);

	// De-assert CS pin
	cyhal_gpio_write(cs_pin, 1);
}

/** Writes a single byte to the specified address
 *
 * @param address -- 16 bit address in the EEPROM
 * @param data    -- value to write into memory
 *
 */
void eeprom_write_byte(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin, uint16_t address, uint8_t data)
{
	// Enable writes first
	eeprom_write_enable(spi_obj, cs_pin);

	uint8_t tx_buffer[4];
	tx_buffer[0] = EEPROM_CMD_WRITE;	  // Write command
	tx_buffer[1] = (address >> 8) & 0xFF; // High byte of address
	tx_buffer[2] = address & 0xFF;		  // Low byte of address
	tx_buffer[3] = data;				  // Data byte to write

	// Assert CS pin
	cyhal_gpio_write(cs_pin, 0);

	// Transmit data over SPI
	cyhal_spi_transfer(spi_obj, tx_buffer, 4, NULL, 4, 0xFF);

	// De-assert CS pin
	cyhal_gpio_write(cs_pin, 1);

	// Wait for write to complete
	eeprom_wait_for_write(spi_obj, cs_pin);
}

/** Reads a single byte to the specified address
 *
 * @param address -- 16 bit address in the EEPROM
 *
 */
uint8_t eeprom_read_byte(cyhal_spi_t *spi_obj, cyhal_gpio_t cs_pin, uint16_t address)
{
	uint8_t tx_buffer[4];
	uint8_t rx_buffer[4];

	tx_buffer[0] = EEPROM_CMD_READ;		  // Read command
	tx_buffer[1] = (address >> 8) & 0xFF; // High byte of address
	tx_buffer[2] = address & 0xFF;		  // Low byte of address
	tx_buffer[3] = 0xFF;				  // Dummy byte for reading data

	// Assert CS pin
	cyhal_gpio_write(cs_pin, 0);

	// Transmit data over SPI
	cyhal_spi_transfer(spi_obj, tx_buffer, 4, rx_buffer, 4, 0xFF);

	// De-assert CS pin
	cyhal_gpio_write(cs_pin, 1);

	return rx_buffer[3]; // Return the received data byte
}