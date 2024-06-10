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
#include "satellite-subsystems/GomEPS.h"

#define EPS_INDEX 100 //place holder

#define SMOOTHEN(volt, alpha) (currentVolatage - (alpha * (volt - currentVolatage)))

int UpdateState(voltage_t);
int UpdateStateFirst();

voltage_t currentVolatage;
voltage_t prevVolatage;
float Alpha = DEFAULT_ALPHA_VALUE;


int EPS_Init(void)
{


#ifdef GOMEPS_H_

    unsigned char i2c_address = GOM_I2C_ADRESS;
    int rv;

	rv = GomEpsInitialize(&i2c_address, 1);
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
		logError(rv,"\n\r GomEpsInitialize() failed \n\r");
		return 0;

#endif
	GetBatteryVoltage(&prevVolatage);
	return 0;
}
int EPS_Conditioning() {
	voltage_t temp;
	GetBatteryVoltage(&temp);
	currentVolatage = SMOOTHEN(temp, Alpha);
	UpdateState(currentVolatage);
	prevVolatage = currentVolatage;
	return 0;
}
//EPS_Conditioning
int UpdateState(voltage_t prev) {
	if (prev > currentVolatage) {
		if(prev > 7500) EnterOperationalMode();
		if(prev > 7100) EnterCruiseMode();
	}
	if(prev < currentVolatage) {
		if(prev > 7400) EnterCruiseMode();
		if(prev > 7000) EnterPowerSafeMode();
	}

	return 0;
}
/*
 * there might be an edge case with EPS_Conditioning, if you use it in the reset,
 * @brief, like EPS_Conditioning, but with a symmetric thresholds
 * return 0;
 */
int UpdateStateFirst() {
	if (currentVolatage > 7500)
			EnterOperationalMode();
		else if (currentVolatage < 7500 && currentVolatage > 7000)
			EnterCruiseMode();
		else if (currentVolatage < 7000)
			EnterPowerSafeMode();
	return 0;
}
int GetAlpha(float *alpha) {
	*alpha = Alpha;
	return 0;
}
int RestoreDefaultAlpha() {
	Alpha = DEFAULT_ALPHA_VALUE;
	return 0;
}


int GetBatteryVoltage(voltage_t *vbat) {
	//gom_eps_hk_vi_t
	vbat = vbat + 1;
#ifdef GOMEPS_H_
	gom_eps_hk_vi_t output;
	GomEpsGetHkData_vi(0x02, &output);
	*vbat = (voltage_t)output.fields.vbatt;
#else
	imepsv2_piu__gethousekeepingeng__from_t houseKeeping;
	imepsv2_piu__gethousekeepingeng(EPS_INDEX, &houseKeeping);
	*vbat = (voltage_t)houseKeeping.fields.volt_brdsup;
#endif

	return 0;
}


