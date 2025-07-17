/**
 * @file isis_hstxs_spi.h
 * @brief Driver for the high speed data SPI / LVDS data interface of the HS TXS
 */

#ifndef ISIS_HSTXS_SPI_H_
#define ISIS_HSTXS_SPI_H_

#include <stdint.h>
#include <at91/peripherals/pio/pio.h>
#include <hal/Drivers/SPI.h>
#include <hal/boolean.h>
#include <satellite-subsystems/common_types.h>
#include <satellite-subsystems/isis_hstxs_types.h>

#define ISIS_HSTXS_SPI_BUSSPEED		10000000 /*!< Speed in Hz used for the SPI data transfers */

/*!
 * Union for defining a frame transmitted over the SPI interface
 */
typedef union __attribute__((__packed__)) _isis_hstxs_spi__sendframe__to_t
{
    unsigned char raw[223];
    struct __attribute__ ((__packed__))
    {
        isis_hstxs__ccsds_frameheader_t header; /*!< Header to be transmitted */
        uint8_t data[217]; /*!< Data to be transmitted */
    } fields;
} isis_hstxs_spi__sendframe__to_t;

/**
*	Initialize high-speed data interface driver for a single ISIS_HSTXS instance
*
*	@param[in] spi_slave SPI slave / chip select in use for this ISIS_HSTXS
*	@param[in] pin_txready GPIO pin to which the TX ready signal of the ISIS_HSTXS is connected. This pin will be configured as input with its internal pull-up enabled.
* 	@return Error code as specified in common_types.h
*/
driver_error_t ISIS_HSTXS_SPI_Init(SPIslave spi_slave, Pin pin_txready);

/**
 * Send data frame to the ISIS_HSTXS
 *
 * @param[in] frame Pointer to a data frame struct containing header and data
 * @param[in] blocking Boolean to indicate whether this call should block until SPI transfer is complete
 * @return Error code as specified in common_types.h
 */
driver_error_t isis_hstxs_spi__sendframe(isis_hstxs_spi__sendframe__to_t *frame, Boolean blocking);

/**
 * Get the value of the TX ready pin
 *
 * @param[out] pin_value Pointer to memory location where the value of the GPIO pin will be stored
 * @return Error code as specified in common_types.h
 */
driver_error_t isis_hstxs_spi__gettxready(uint8_t* pin_value);

#endif
