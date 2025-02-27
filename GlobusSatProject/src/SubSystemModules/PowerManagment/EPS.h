/*
 * @file	EPS.h
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */
#ifndef EPS_H_
#define EPS_H_

#include "GlobalStandards.h"
#include "EPSOperationModes.h"
#include "FRAM_FlightParameters.h"
#include "satellite-subsystems/isismepsv2_ivid7_piu.h"
#include "satellite-subsystems/IsisSolarPanelv2.h"

#include "utils.h"
#include "SysI2CAddr.h"

#include "SubSystemModules/Payload/payload_drivers.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include <stdint.h>
#include <hal/Drivers/SPI.h>
/*
 	 	 	 	    ____
			  _____|	|_____
 	 	 	 |				  |
 	 	 	 |OPERATIONAL MODE|
 	 	 	 |- - - - - - -	- |	-> OPERATIONAL UP = 7400
 	 	 	 |- - - - - - - - |	-> OPERATIONAL DOWN = 7300
 	 	 	 |				  |
 	 	 	 |	 CRUISE MODE  |
 	 	 	 |- - - - - - -	- |	-> CRUISE UP = 7000
 	 	 	 |- - - - - - - - |	-> CRUISE DOWN = 6900
 	 	 	 |				  |
 	 	 	 |POWER SAFE MODE |
 	 	 	 |________________|
 */
#define DEFAULT_ALPHA_VALUE 0.3

#define NUMBER_OF_SOLAR_PANELS			6
#define NUMBER_OF_THRESHOLD_VOLTAGES 	4 		///< first 2 are discharging voltages, last 2 are charging voltages
#define DEFAULT_EPS_THRESHOLD_VOLTAGES 	{(voltage_t)6900, (voltage_t)7300,	 \
										  (voltage_t)7000, (voltage_t)7400}

typedef enum __attribute__ ((__packed__)){
	INDEX_DOWN_CRUISE,
	INDEX_DOWN_OPERATIONAL,
	INDEX_UP_CRUISE,
	INDEX_UP_OPERATIONAL
}EpsThresholdsIndex;

typedef union __attribute__ ((__packed__)){
	voltage_t raw[NUMBER_OF_THRESHOLD_VOLTAGES];
	struct {
		voltage_t Vdown_cruise;
		voltage_t Vdown_operational;
		voltage_t Vup_cruise;
		voltage_t Vup_operational;
	}fields;
}EpsThreshVolt_t;

typedef union __attribute__ ((__packed__)){
struct {
	int16_t H1_MIN;
	int16_t H1_MAX;
	int16_t H2_MIN;
	int16_t H2_MAX;
	int16_t H3_MIN;
	int16_t H3_MAX;
}value;
}HeaterValues;
/*!
 * @brief initializes the EPS subsystem and the solar panels.
 * @return	0 on success
 * 			-1 on failure of init
 */
int EPS_And_SP_Init();

/*!
 * @brief EPS logic. controls the state machine of which subsystem
 * is on or off, as a function of only the battery voltage
 * @return	0 on success
 * 			-1 on failure setting state of channels
 */
int EPS_Conditioning();

/*!
 * @brief returns the current voltage on the battery
 * @param[out] vbat he current battery voltage
 * @return	0 on success
 * 			Error code according to <hal/errors.h>
 */
int GetBatteryVoltage(voltage_t *vbat);

/*!
 * @brief setting the new EPS logic threshold voltages on the FRAM.
 * @param[in] thresh_volts an array holding the new threshold values
 * @return	0 on success
 * 			-1 on failure setting new threshold voltages
 * 			-2 on invalid thresholds
 * 			ERR according to <hal/errors.h>
 */
int UpdateThresholdVoltages(EpsThreshVolt_t thresh_volts);

/*!
 * @brief getting the EPS logic threshold  voltages on the FRAM.
 * @param[out] thresh_volts a buffer to hold the threshold values
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetThresholdVoltages(EpsThreshVolt_t *thresh_volts);

/*!
 * @brief getting the smoothing factor (alpha) from the FRAM.
 * @param[out] alpha a buffer to hold the smoothing factor
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetAlpha(float *alpha);

/*!
 * @brief setting the new voltage smoothing factor (alpha) on the FRAM.
 * @param[in] new_alpha new value for the smoothing factor alpha
 * @note new_alpha is a value in the range - (0,1)
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * 			-2 on invalid alpha
 * 			-4 FRAM_read problem
 * 			-5 written wrong data
 * @see LPF- Low Pass Filter at wikipedia: https://en.wikipedia.org/wiki/Low-pass_filter#Discrete-time_realization
 */
int UpdateAlpha(float alpha);

#endif
