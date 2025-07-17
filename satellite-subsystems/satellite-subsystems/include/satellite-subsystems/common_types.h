/*
 * common_types.h
 *
 *  Created on: 13 Dec 2019
 *      Author: asau
 */

#ifndef INCLUDE_SATELLITE_SUBSYSTEMS_COMMON_TYPES_H_
#define INCLUDE_SATELLITE_SUBSYSTEMS_COMMON_TYPES_H_

typedef enum _driver_error_t
{
    driver_error_none = 0, /*!< No error occurred */
    driver_error_i2c = 1, /*!< An error occurred while attempting to communicate over I2C */
    driver_error_spi = 2, /*!< An error occurred while attempting to communicate over SPI */
    driver_error_uart = 3, /*!< An error occurred while attempting to communicate over UART */
    driver_error_param = 4, /*!< One of the provided input parameters was invalid */
    driver_error_malloc = 5, /*!< An error occurred while trying to allocate memory */
    driver_error_reinit = 6, /*!< The driver has already been initialized */
    driver_error_param_length = 7, /*!< One of the provided input parameters was too big or too small */
    driver_error_reply_crc = 8, /*!< The CRC in the subsystem's reply did not match the data */
}
driver_error_t;

#endif /* INCLUDE_SATELLITE_SUBSYSTEMS_COMMON_TYPES_H_ */
