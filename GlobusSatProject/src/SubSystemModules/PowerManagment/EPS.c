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
#include "satellite-subsystems/imepsv2_piu.h"
#include "satellite-subsystems/GomEPS.h"


#include "string.h"
#include "EPS.h"
#include "GlobalStandards.h"
#include "utils.h"

#define EPS_INDEX 100 //place holder

#define SMOOTHEN(volt, alpha) (currentVolatage - (alpha * (volt - currentVolatage)))
voltage_t ThresholdsIndex[NUMBER_OF_THRESHOLD_VOLTAGES] = DEFAULT_EPS_THRESHOLD_VOLTAGES;

voltage_t currentVolatage;
voltage_t prevVolatage;
float Alpha = DEFAULT_ALPHA_VALUE;

int UpdateState(voltage_t current) {
	if (current >= prevVolatage) {
		if(current > ThresholdsIndex[3]) EnterOperationalMode();
		else if(current > ThresholdsIndex[2]) EnterCruiseMode();
	}
	if(current < prevVolatage) {
		if(current < ThresholdsIndex[1]) EnterCruiseMode();
		else if(current < ThresholdsIndex[0]) EnterPowerSafeMode();
	}
	//printf("%d \r\n", current);
	return 0;
}

int EPS_Init(void)
{


#ifdef GOMEPS_H_

    unsigned char i2c_address = GOM_I2C_ADRESS;
    int rv;

	rv = GomEpsInitialize(&i2c_address, 1);
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
	logError(rv,"GomEps");
	if(rv == 0)
		GetBatteryVoltage(&prevVolatage);
	return rv;

#endif

}
int EPS_Conditioning() {
	voltage_t temp;
	GetBatteryVoltage(&temp);
	currentVolatage = SMOOTHEN(temp, Alpha);
	currentVolatage = temp;
	UpdateState(currentVolatage);
	prevVolatage = currentVolatage;
	return 0;
}
//EPS_Conditioning

int GetAlpha(float *alpha) {
	unsigned char data[EPS_ALPHA_FILTER_VALUE_SIZE];
	int error = logError(FRAM_read(data, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE), "GetAlpha, FRAM_read ");
	memcpy(alpha, data, EPS_ALPHA_FILTER_VALUE_SIZE);
	return error;
}
int SetAlpha(float *alpha) {
	unsigned char data[EPS_ALPHA_FILTER_VALUE_SIZE];
	memcpy(data, alpha, EPS_ALPHA_FILTER_VALUE_SIZE);
	int error = logError(FRAM_write(data, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE), "SetAlpha, FRAM_write");
	memcpy(alpha, data, EPS_ALPHA_FILTER_VALUE_SIZE);
	return error;
}
int RestoreDefaultAlpha() {
	Alpha = DEFAULT_ALPHA_VALUE;
	SetAlpha(&Alpha);
	return 0;
}
int GetEPSThreshold(voltage_t Threshold[NUMBER_OF_THRESHOLD_VOLTAGES]) {
//EPS_THRESH_VOLTAGES_ADDR
//EPS_THRESH_VOLTAGES_SIZE
	unsigned char data[EPS_THRESH_VOLTAGES_SIZE];
	int error = logError(FRAM_read(data, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE), "GetEPSThreshold, FRAM READ");
	memcpy(Threshold, data, EPS_THRESH_VOLTAGES_SIZE);
	return error;
}
int SetEPSThreshold(voltage_t Threshold[NUMBER_OF_THRESHOLD_VOLTAGES]) {
	unsigned char data[EPS_THRESH_VOLTAGES_SIZE];
	memcpy(data, Threshold, EPS_THRESH_VOLTAGES_SIZE);
	int error = logError(FRAM_write(data, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE), "SetEPSThreshold, FRAM_write");
	memcpy(Threshold, data, EPS_THRESH_VOLTAGES_SIZE);
	return error;
}

int GetBatteryVoltage(voltage_t *vbat) {
	//gom_eps_hk_vi_t
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



