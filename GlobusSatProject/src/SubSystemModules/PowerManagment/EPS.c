/*
 * EPS.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "EPS.h"
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include "utils.h"

voltage_t currentVolatage = 7000; //didnt make that function yet;
voltage_t prevVolt;
float ALPHA = DEFAULT_ALPHA_VALUE;

int EPS_Conditioning() {
	if (currentVolatage > 7500)
		EnterOperationalMode();
	else if (currentVolatage > 7400 && currentVolatage < 7100)
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

