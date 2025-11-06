/**
 * @file spi.c
 * @author Joe Krachey (jkrachey@wisc.edu)
 * @brief 
 * @version 0.1
 * @date 2025-09-15
 * 
 * @copyright Copyright (c) 2025
 * 
 */
#include "spi.h"

cyhal_spi_t mSPI;

cyhal_spi_t * spi_init(
	cyhal_gpio_t mosi,
	cyhal_gpio_t miso,
	cyhal_gpio_t clk
)
{
	cy_rslt_t   rslt;

	memset(&mSPI, 0, sizeof(mSPI));

    // Configuring the  SPI master:  Specify the SPI interface pins, frame size, SPI Motorola mode
    // and master mode
    rslt = cyhal_spi_init(
    						&mSPI,
							mosi,			// MOSI Pin
							miso,			// MISO Pin
							clk,			// Clock Pin
							NC,					// CS -- Will control using an IO pin
							NULL,					// Clock Source -- if not provided a new clock will be allocated
							8,						// Bits per frame
							CYHAL_SPI_MODE_00_MSB,		// SPI Mode
							false						// Is Subordinate??
						);

    if (rslt == CY_RSLT_SUCCESS)
    {
        // Set the data rate to 1 Mbps
    	rslt = cyhal_spi_set_frequency(&mSPI, SPI_FREQ);
    }

	return &mSPI;

}
