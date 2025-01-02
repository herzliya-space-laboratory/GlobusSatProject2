/*
 * EPS.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "EPS.h"



#define SMOOTHING(currentVolt, alpha) (lastVoltage + (alpha * (currentVolt - lastVoltage)))
#define MAX_VOLTAGE_TO_STATES {7100, 7500, 7200, 7600}
#define MIN_VOLTAGE_TO_STATES {6000, 6500, 6100, 6600}

#define PIN_RESET PIN_GPIO06
#define PIN_INT   PIN_GPIO00

voltage_t lastVoltage;
EpsThreshVolt_t threshold_volts;
float Alpha;

int EPS_And_SP_Init(){
	int errorEPS = 0;
	int errorSP = 0;
	ISISMEPSV2_IVID7_PIU_t stract_1;
	stract_1.i2cAddr = EPS_I2C_ADDR;
	errorEPS = logError(ISISMEPSV2_IVID7_PIU_Init(&stract_1, 1), "EPS_And_SP_Init - ISISMEPSV2_IVID7_PIU_Init");
	if(!errorEPS)
	{
		GetThresholdVoltages(&threshold_volts);
		GetBatteryVoltage(&lastVoltage);
		GetAlpha(&Alpha);
	}
	Pin solarpanelv2_pins[2] = {PIN_RESET, PIN_INT};
	errorSP = logError(IsisSolarPanelv2_initialize(slave0_spi, &solarpanelv2_pins[0], &solarpanelv2_pins[1]), "EPS_And_SP_Init - IsisSolarPanelv2_initialize");
	if(errorSP == 0)
		errorSP = logError(IsisSolarPanelv2_sleep(), "EPS_And_SP_Init - IsisSolarPanelv2_sleep");
	if(errorSP || errorEPS)
			return -1;
		return 0;
}

/*!
 * @brief returns the current voltage on the battery
 * @param[out] vbat he current battery voltage
 * @return	0 on success
 * 			Error code according to <hal/errors.h>
 */
int GetBatteryVoltage(voltage_t *vbat)
{
	isismepsv2_ivid7_piu__gethousekeepingeng__from_t responseEPS; //Create a variable that is the struct we need from EPS_isis
	int error_eps = logError(isismepsv2_ivid7_piu__gethousekeepingeng(0,&responseEPS), "GetBatteryVoltage - isismepsv2_ivid7_piu__gethousekeepingeng"); //Get struct and get kind of error
	if(error_eps) return error_eps;
	*vbat = (voltage_t)responseEPS.fields.batt_input.fields.volt;
	return 0;
}

/*!
 * @brief getting the smoothing factor (alpha) from the FRAM.
 * @param[out] alpha a buffer to hold the smoothing factor
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetAlpha(float *alpha)
{
	if(alpha == NULL) return -1;
	float alphaVal;
	if(logError(FRAM_read((unsigned char*)&alphaVal, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE), "GetAlpha - FRAM_read"))
		return -2;
	*alpha = alphaVal;
	return 0;

}

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
int UpdateAlpha(float alpha)
{
	if(alpha < 0 || alpha > 1) return -2;
	if(logError(FRAM_write((unsigned char*)&alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE),"UpdateAlpha - FRAM_write")) return -1;
	Alpha = alpha;
	float writtenAlpha;
	if(logError(GetAlpha(&writtenAlpha), "UpdateAlpha - GetAlpha")) return -4;
	if(writtenAlpha != alpha) return -5;
	return 0;
}

/*!
 * @brief setting the new EPS logic threshold voltages on the FRAM.
 * @param[in] thresh_volts an array holding the new threshold values
 * @return	0 on success
 * 			-1 on failure setting new threshold voltages
 * 			-2 on invalid thresholds
 * 			-3 on failure to read
 * 			-4 written wrong
 * 			ERR according to <hal/errors.h>
 */
int UpdateThresholdVoltages(EpsThreshVolt_t thresh_volts)
{
	voltage_t maxes[NUMBER_OF_THRESHOLD_VOLTAGES] = MAX_VOLTAGE_TO_STATES;
	voltage_t mins[NUMBER_OF_THRESHOLD_VOLTAGES] = MIN_VOLTAGE_TO_STATES;
	for(int i = 0; i < NUMBER_OF_THRESHOLD_VOLTAGES; i++)
		if(thresh_volts.raw[i] < mins[i] || thresh_volts.raw[i] > maxes[i]) return -4;
	if(logError(FRAM_write((unsigned char*)&thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE), "UpdateThresholdVoltages - FRAM_read")) return -1;

	if(GetThresholdVoltages(&threshold_volts)) return -3;
	for(int i = 0; i < NUMBER_OF_THRESHOLD_VOLTAGES; i++)
		if(thresh_volts.raw[i] != threshold_volts.raw[i]) return -4;
	return 0;
}

/*!
 * @brief getting the EPS logic threshold  voltages on the FRAM.
 * @param[out] thresh_volts a buffer to hold the threshold values
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetThresholdVoltages(EpsThreshVolt_t *thresh_volts)
{
	if(thresh_volts == NULL) return -1;
	if(logError(FRAM_read((unsigned char*)thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE), "GetThresholdVoltages - FRAM_read"))
		return -2;
	return 0;
}

/*!
 * @brief EPS logic. controls the state machine of which subsystem
 * is on or off, as a function of only the battery voltage
 * @return	0 on success
 * 			-1 on failure setting state of channels
 */
int EPS_Conditioning()
{
	voltage_t currentVoltage;
	GetBatteryVoltage(&currentVoltage);
	double smoothing = SMOOTHING(currentVoltage, Alpha);
	if(lastVoltage <= smoothing)
	{
		if(smoothing >= threshold_volts.fields.Vup_operational)
			EnterOperationalMode();
		else if(smoothing >= threshold_volts.fields.Vup_cruise)
			EnterCruiseMode();
	}
	else
	{
		if(smoothing <= threshold_volts.fields.Vdown_operational)
			EnterCruiseMode();
		else if(smoothing <= threshold_volts.fields.Vdown_cruise) //TODO: check why not working or not printing
			EnterPowerSafeMode();
	}
	lastVoltage = smoothing;
	return 0;
}
