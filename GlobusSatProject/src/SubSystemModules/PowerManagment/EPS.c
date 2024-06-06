/*
 * EPS.c
 *
 *  Created on: 5 6 2024
 *      Author: ben friedamn
 */


/*
 *#TODO check if volt_brdsup is the right way to get volt
 *#TODO finish EPS loop and init
 */
#include "satellite-subsystems/imepsv2_piu.h"

#include "EPS.h"
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include "utils.h"

#define EPS_INDEX 100 //place holder
int EPS_Init();
int EPS_Loop();
int GetBatteryVoltage(voltage_t *vbat);
int EPS_Conditioning();
int EPS_First_Conditioning(); //do this before the loop
int GetAlpha(float *alpha);
int RestoreDefaultAlpha();
int smoothen(voltage_t volt, float alpha, voltage_t *output);

voltage_t currentVolatage;

float ALPHA = DEFAULT_ALPHA_VALUE;


int EPS_Init() {
	GetBatteryVoltage(&currentVolatage);
	EPS_First_Conditioning();
	return 0;
}

int EPS_Loop() {
	voltage_t temp;
	GetBatteryVoltage(&temp);
	smoothen(temp, ALPHA, &currentVolatage);
	EPS_Conditioning();
	return 0;
}

int EPS_Conditioning() {
	if (currentVolatage > 7500)
		EnterOperationalMode();
	else if (currentVolatage < 7400 && currentVolatage > 7100)
		EnterCruiseMode();
	else if (currentVolatage < 7000)
		EnterPowerSafeMode();

	return 0;
}
/*
 * there might be an edge case with EPS_Conditioning, if you use it in the reset,
 * @brief, like EPS_Conditioning, but with a symmetric thresholds
 * return 0;
 */
int EPS_First_Conditioning() {
	if (currentVolatage > 7500)
			EnterOperationalMode();
		else if (currentVolatage < 7500 && currentVolatage > 7000)
			EnterCruiseMode();
		else if (currentVolatage < 7000)
			EnterPowerSafeMode();
	return 0;
}
int GetAlpha(float *alpha) {
	*alpha = ALPHA;
	return 0;
}
int RestoreDefaultAlpha() {
	ALPHA = DEFAULT_ALPHA_VALUE;
	return 0;
}
int smoothen(voltage_t volt, float alpha, voltage_t *output) {
	*output = currentVolatage - (alpha * (volt - currentVolatage));
	return 0;
}

int GetBatteryVoltage(voltage_t *vbat) {
	imepsv2_piu__gethousekeepingeng__from_t houseKeeping;
	imepsv2_piu__gethousekeepingeng(EPS_INDEX, &houseKeeping);
	*vbat = (voltage_t)houseKeeping.fields.volt_brdsup;
	return 0;
}

