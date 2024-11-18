/**
 * @file isis_hstxs_v2_spi.h
 * @brief Driver for the high speed data SPI / LVDS data interface of the HS TXS
 */

#ifndef ISIS_HSTXS_V2_SPI_H_
#define ISIS_HSTXS_V2_SPI_H_

#include <stdint.h>
#include <at91/peripherals/pio/pio.h>
#include <hal/Drivers/SPI.h>
#include <hal/boolean.h>
#include <satellite-subsystems/common_types.h>
#include <satellite-subsystems/isis_hstxs_v2_types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define ISIS_HSTXS_V2_SPI_BUSSPEED		10000000 /*!< Speed in Hz used for the SPI data transfers */

/*!
 * Union for defining a frame transmitted over the SPI interface
 */
typedef union __attribute__((__packed__)) _isis_hstxs_v2_spi__write_and_send_frame__to_t
{
    unsigned char raw[223];
    struct __attribute__ ((__packed__))
    {
        isis_hstxs_v2__ccsds_frameheader_t header; /*!< Header to be transmitted */
        uint8_t data[217]; /*!< Data to be transmitted */
    } fields;
} isis_hstxs_v2_spi__write_and_send_frame__to_t;


/**
*	Initialize high-speed data interface driver for a single ISIS_HSTXS_V2 instance
*
*	@param[in] spi_slave SPI slave / chip select in use for this ISIS_HSTXS_V2
*	@param[in] pin_txready GPIO pin to which the TX ready signal of the ISIS_HSTXS_V2 is connected. This pin will be configured as input with its internal pull-up enabled.
*	@param[in] spispeed_hz Speed to use on the SPI bus in Hz, needs to be <= SPI_MAX_BUS_SPEED and >= SPI_MIN_BUS_SPEED, suggested speed is ISIS_HSTXS_V2_SPI_BUSSPEED
* 	@return Error code as specified in common_types.h
*/
driver_error_t ISIS_HSTXS_V2_SPI_Init(SPIslave spi_slave, Pin pin_txready, unsigned int spispeed_hz);

/**
 * Send data frame to the ISIS_HSTXS_V2 over SPI and block until transfer is completed.
 *
 * @param[in] frame Pointer to a data frame struct containing header and data
 * @return Error code as specified in common_types.h
 */
driver_error_t isis_hstxs_v2_spi__write_and_send_frame_blocking(isis_hstxs_v2_spi__write_and_send_frame__to_t *params);

/**
 * Get the value of the TX ready pin
 *
 * @param[out] pin_value Pointer to memory location where the value of the GPIO pin will be stored
 * @return Error code as specified in common_types.h
 */
driver_error_t isis_hstxs_v2_spi__get_tx_ready(uint8_t* pin_value);

#ifdef __cplusplus
}
#endif

#endif
