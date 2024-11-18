/**
 * @file IsisSolarPanelv2.h
 * @brief ISIS Solar Panel temperature sensors, version 2 (using an LTC ADC driver)
 */

#ifndef SRC_ISISSOLARPANELV2_H_
#define SRC_ISISSOLARPANELV2_H_

#include <at91/peripherals/pio/pio.h>
#include <hal/Drivers/SPI.h>
#include <freertos/FreeRTOS.h>

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** Macro used for converting temperature to floating point value */
#define ISIS_SOLAR_PANEL_CONV ( 1.0 / 1024.0 )

/**
 * Return values for ISIS solar panel function calls.
 */
typedef enum _IsisSolarPanelv2_Error_t
{
	ISIS_SOLAR_PANEL_ERR_INPUT	 = -6, /*!< One of the input parameters was incorrect */
	ISIS_SOLAR_PANEL_ERR_TIMEOUT = -5, /*!< There was a timeout waiting for the LTC */
	ISIS_SOLAR_PANEL_ERR_ADC     = -4, /*!< The LTC ADC reported an error */
	ISIS_SOLAR_PANEL_ERR_PIN     = -3, /*!< Error configuring the GPIO pins */
	ISIS_SOLAR_PANEL_ERR_SPI     = -2, /*!< Error communicating on the SPI bus */
	ISIS_SOLAR_PANEL_ERR_STATE   = -1, /*!< The internal temperature sensor is not in the appropriate state for this action */
	ISIS_SOLAR_PANEL_ERR_NONE    =  0 /*!< No error */
} IsisSolarPanelv2_Error_t;

/**
 * State of the internal temperature sensor.
 */
typedef enum _IsisSolarPanelv2_State_t
{
	ISIS_SOLAR_PANEL_STATE_NOINIT, /*!< Unitialized */
	ISIS_SOLAR_PANEL_STATE_SLEEP, /*!< Sleeping */
	ISIS_SOLAR_PANEL_STATE_AWAKE, /*!< Active */
}
IsisSolarPanelv2_State_t;

/**
 * Channels available for temperature measurements.
 * Note: Some channels might not be in use
 */
typedef enum _IsisSolarPanel_panel_t
{
	ISIS_SOLAR_PANEL_0     = 0,
	ISIS_SOLAR_PANEL_1     = 1,
	ISIS_SOLAR_PANEL_2     = 2,
	ISIS_SOLAR_PANEL_3     = 3,
	ISIS_SOLAR_PANEL_4     = 4,
	ISIS_SOLAR_PANEL_5     = 5,
	ISIS_SOLAR_PANEL_6     = 6,
	ISIS_SOLAR_PANEL_7     = 7,
	ISIS_SOLAR_PANEL_8     = 8,
	ISIS_SOLAR_PANEL_COUNT = 9
}
IsisSolarPanelv2_Panel_t;

typedef struct {
	portTickType delay_ms; /*!< Delay between polling of status of the LTC chip */
	portTickType timeout_ms; /*!< Timeout to wait for an action on the LTC chip to complete */
}
IsisSolarPanelv2_timeouts_s;

/**
 * Initializes the internal hardware and software required for measuring the
 * temperatures of the ISIS solar panels. This function assumes the SPI driver
 * has already been initialized properly.
 *
 * @param[in] spi_slave Contains the SPI slave on SPI bus 1 to which LTC ADC driver is connected
 * @param[in] reset_pin GPIO pin used to reset the LTC chip, will be configured as output
 * @param[in] int_pin GPIO pin used for interrupts from the LTC chip, will be configured as input
 * @return A value defined by IsisSolarPanelv2_Error_t
 */
IsisSolarPanelv2_Error_t IsisSolarPanelv2_initialize( SPIslave spi_slave, Pin* reset_pin, Pin* int_pin );

/**
 * Sets the ADC sampling delay and timeout.
 * If the sampling is unsuccessful, *delay* is applied and a new sampling is performed until
 * either the sampling is successful or *timeout* is met. By default this driver uses 5 ms
 * for the delay and 500 ms for the timeout.
 *
 * @param[in] p_timeout New timing parameters to be used by the driver
 * @return A value defined by IsisSolarPanelv2_Error_t
 */
IsisSolarPanelv2_Error_t IsisSolarPanelv2_set_interface_timeout( const IsisSolarPanelv2_timeouts_s *p_timeout );

/**
 * Puts the internal temperature sensor to sleep mode. Reducing its power-
 * consumption.
 *
 * @note When in sleep mode, the temperature sensor will need to be woken by a call to
 *   IsisSolarPanelv2_wakeup() before attempting to acquire any temperature
 *   readings. Waking the sensor could take up to 500ms to complete.
 *
 * @return A value defined by IsisSolarPanelv2_Error_t
 */
IsisSolarPanelv2_Error_t IsisSolarPanelv2_sleep( void );

/**
 * Wakes the internal temperature sensor from sleep mode. Allowing it to acquire
 * temperature readings from the solar panels.
 *
 * @note Waking the sensor could take up to 500ms to complete.
 *
 * @return A value defined by IsisSolarPanelv2_Error_t
 */
IsisSolarPanelv2_Error_t IsisSolarPanelv2_wakeup(void);

/**
 * Acquires the temperature from the specified panel as well as a status byte
 * used for fault reporting. The temperature is reported from -273.16 to
 * 8192 degrees Celsius with a resolution of 1/1024.
 *
 * @param[in] panel The specified panel for which the temperature should be
 *   acquired
 * @param[out] panelTemp The acquired temperature for the specified panel
 * @param[out] status The status of the acquired temperature. Used for fault
 *   reporting.
 * @return A value defined by IsisSolarPanelv2_Error_t
 */
IsisSolarPanelv2_Error_t IsisSolarPanelv2_getTemperature( IsisSolarPanelv2_Panel_t panel, int32_t* paneltemp, uint8_t *status );

/**
 * Returns the current state of the internal temperature sensor.
 *
 * @return The state as defined by IsisSolarPanelv2_State_t
 */
IsisSolarPanelv2_State_t IsisSolarPanelv2_getState( void );

#ifdef __cplusplus
}
#endif

#endif /* SRC_ISISSOLARPANELV2_H_ */
