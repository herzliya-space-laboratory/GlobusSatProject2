/*
 * EPS.c
 *
 *  Created on: 5 6 2024
 *      Author: ben friedamn
 */


/*
 *#TODO check if volt_brdsup is the right way to get volt
 *#TODO finish EPS loop and init
 *#thoughts.
 */
#include "EPSOperationModes.h"
#include "satellite-subsystems/IsisSolarPanelv2.h"
#include "satellite-subsystems/imepsv2_piu.h"
#include "satellite-subsystems/GomEPS.h"
#include "FRAM_FlightParameters.h"
#include "EPS.h"
#include "GlobalStandards.h"
#include "utils.h"
#include "SysI2CAddr.h"
#include <Time.h>
#include <string.h>
#include <hal/errors.h>
#include <hal/Drivers/SPI.h>
/*#define WE_HAVE_SP 1*/
/*#define WE_HAVE_EPS 1*/
Time TimeSinceLastChangeReset;
#define EPS_INDEX 100 //place holder
#define SMOOTHEN(volt, alpha) (currentVolatage - (alpha * (volt - currentVolatage)))
EpsThreshVolt_t ThresholdsIndex = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
voltage_t prevVolatage;
float Alpha = DEFAULT_ALPHA_VALUE;
int UpdateState(voltage_t current) {
	if (current >= prevVolatage) {
		if(current > ThresholdsIndex.fields.Vup_operational) EnterOperationalMode();
		else if(current > ThresholdsIndex.fields.Vup_cruise) EnterCruiseMode();
	}
	if(current < prevVolatage) {
		if(current < ThresholdsIndex.fields.Vdown_operational) EnterCruiseMode();
		else if(current < ThresholdsIndex.fields.Vdown_cruise) EnterPowerSafeMode();
	}
	//printf("%d \r\n", current);
	return 0;
}

int EPS_And_SP_Init(void)
{
	int errorEPS = 0;
	int errorSP = 0;
#ifdef WE_HAVE_EPS
	IMEPSV2_PIU_t stract_1;
	stract_1.i2cAddr = EPS_I2C_ADDR;
	errorEPS = logError(IMEPSV2_PIU_Init(&stract_1, 1), "EPS - IMEPSV2_PIU_Init");
	if(errorEPS != E_NO_SS_ERR)
	{
		GetBatteryVoltage(&prevVolatage);
		float temp;
		GetAlpha(&temp);
		EpsThreshVolt_t temp2;
		GetEPSThreshold(&temp2);
	}

#endif
#ifdef WE_HAVE_SP
	errorSP = logError(IsisSolarPanelv2_initialize(slave0_spi), "Solar panels - IsisSolarPanelv2_initialize");
	if(errorSP == 0)
		errorSP = logError(IsisSolarPanelv2_sleep(), "Solar panels - sleep");
#endif
	if(errorSP || errorEPS)
			return -1;
		return 0;

#ifdef GOMEPS_H_

    unsigned char i2c_address = GOM_I2C_ADRESS;
    int rv;

	rv = GomEpsInitialize(&i2c_address, 1);
	logError(rv,"GomEps");
	if(rv == 0)
		GetBatteryVoltage(&prevVolatage);
		float temp;
		GetAlpha(&temp);
		EpsThreshVolt_t temp2;
		GetEPSThreshold(&temp2);
		GetSystemState();
		GetcurrentMode();
	return rv;

#endif

}
int EPS_Loop() {
	voltage_t temp;
	GetBatteryVoltage(&temp);
	voltage_t currentVolatage = SMOOTHEN(temp, Alpha);
	UpdateState(currentVolatage);
	prevVolatage = currentVolatage;
	CheckAndResetStateChanges();
	return 0;
}


int GetAlpha(float *alpha) {
	if (alpha == NULL)  {
		logError(E_INPUT_POINTER_NULL, "GetAlpha, alpha is null");
		return E_INPUT_POINTER_NULL;
	}
	int error = logError(FRAM_read((unsigned char *)alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE), "GetAlpha, FRAM_read ");
	if (error == 0) Alpha = *alpha;
	return error;
}
int UpdateAlpha(float newalpha) {
	float alpha2 = newalpha;
	if (0 > alpha2 || alpha2 > 1) {
		printf("%f", newalpha);
		logError(E_PARAM_OUTOFBOUNDS, "UpdateAlpha, alpa is not in valid range");
		return E_PARAM_OUTOFBOUNDS;
	}
	Alpha = alpha2;
	unsigned char convalpha = (unsigned char )alpha2;
	int error = logError(FRAM_write(&convalpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE), "SetAlpha, FRAM_write");
	return error;

}
int RestoreDefaultAlpha() {
	UpdateAlpha(DEFAULT_ALPHA_VALUE);
	return 0;
}
int RestoreDefaultThresholdVoltages() {
	EpsThreshVolt_t ThresholdsIndexs = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	SetEPSThreshold(&ThresholdsIndexs);
	return 0;
}
int CheckAndResetStateChanges() {
	Time temp;
	Time_get(&temp);
	int error = 0;
	if(temp.hours == TimeSinceLastChangeReset.hours && temp.minutes == TimeSinceLastChangeReset.minutes) {
	TimeSinceLastChangeReset = temp;
	error = logError(FRAM_write((unsigned char *)&TimeSinceLastChangeReset, EPS_LAST_STATE_CHANGE_ADDR, EPS_LAST_STATE_CHANGE_SIZE), "CheckAndResetStateChanges, FRAM_write");
	}
	return error;

}
int InitStateChangesValues() {
	FRAM_read((unsigned char *)&TimeSinceLastChangeReset, EPS_LAST_STATE_CHANGE_ADDR, EPS_LAST_STATE_CHANGE_SIZE);
	logError(FRAM_read((unsigned char *)&CHANGES_OPERATIONAL, EPS_CHANGES_OPERATIONAL_ADDR, EPS_CHANGES_OPERATIONAL_SIZE), "EnterOperationalMode, FRAM_Write");
	logError(FRAM_read((unsigned char *)&CHANGES_POWERSAFE, EPS_CHANGES_POWERSAFE_ADDR, EPS_CHANGES_POWERSAFE_SIZE), "EnterPowerSafeMode, FRAM_Write");
	return 0;
}
int GetEPSThreshold(EpsThreshVolt_t *Threshold) {
	if (Threshold == NULL) {
		logError(E_INPUT_POINTER_NULL, "GetEPSThreshold, Threshold is null"); }
		return E_INPUT_POINTER_NULL;
		int error = logError(FRAM_read((unsigned char *)Threshold, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE), "GetEPSThreshold, FRAM READ");
		if (error == 0)
		ThresholdsIndex = *Threshold;
	return error;

}

int SetEPSThreshold(EpsThreshVolt_t *Threshold) {
	if (Threshold == NULL) {
		logError(E_INPUT_POINTER_NULL, "SetEPSThreshold, threshold is null");
		return E_INPUT_POINTER_NULL;
	}
	if(Threshold->fields.Vdown_cruise < Threshold->fields.Vup_cruise && Threshold->fields.Vup_cruise > Threshold->fields.Vdown_operational && Threshold->fields.Vdown_operational < Threshold->fields.Vup_operational) {
		logError(E_PARAM_OUTOFBOUNDS, "SetEPSThreshold, the values are incorrect");
		return E_PARAM_OUTOFBOUNDS;
	}
	ThresholdsIndex = *Threshold;
	return logError(FRAM_write((unsigned char *)Threshold, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE), "SetEPSThreshold, FRAM_write");
}

int GetBatteryVoltage(voltage_t *vbat) {
#ifdef GOMEPS_H_
	gom_eps_hk_t myEpsStatus_hk;
	int error = logError(GomEpsGetHkData_general(0, &myEpsStatus_hk), "GetBatteryVoltage, GomEpsGetHkData_general");
	*vbat = (voltage_t)myEpsStatus_hk.fields.vbatt;
#else
	imepsv2_piu__gethousekeepingeng__from_t houseKeeping;
	imepsv2_piu__gethousekeepingeng(EPS_INDEX, &houseKeeping);
	*vbat = (voltage_t)houseKeeping.fields.volt_brdsup;
#endif

	return error;
}
