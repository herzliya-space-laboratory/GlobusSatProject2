#include "EPSTest.h"
#include "EPS.h"
#define ERR_FRAM_WRITE -2;
#define ERR_OUT_BOUND -3;
int batteryTest() {
	voltage_t volt;
	return GetBatteryVoltage(&volt);
}
int alphaTest() {
	float alpha;
	float updatealpha;
	int error = GetAlpha(&alpha);
	if (error != 0) return error;

	updatealpha = 0.58444465; //diffrent number so i can check. if it works DO NOT enter this value

	error = UpdateAlpha(updatealpha);
	if (error !=0) return error;

	if (alpha != updatealpha) return ERR_FRAM_WRITE;

	alpha = 2; //unvalid value
	error = UpdateAlpha(alpha);
	if (error != E_PARAM_OUTOFBOUNDS) {
		RestoreDefaultAlpha();
		return ERR_OUT_BOUND;
	}
	RestoreDefaultAlpha();
	return 0;
}
int thresholdTest() {
	EpsThreshVolt_t Thresholds;
	EpsThreshVolt_t ThresholdsIndexUpdate;
	int error;

	error = GetEPSThreshold(&Thresholds);
	if (error != 0) return error;

	Thresholds.fields.Vdown_cruise++;
	Thresholds.fields.Vdown_operational++;
	Thresholds.fields.Vup_cruise++;
	Thresholds.fields.Vup_operational++;

	error = SetEPSThreshold(&Thresholds);
	if (error != 0) return error;

	error = GetEPSThreshold(&ThresholdsIndexUpdate);
	if (error != 0) return error;

	if (Thresholds.fields.Vdown_cruise != ThresholdsIndexUpdate.fields.Vdown_cruise) return ERR_FRAM_WRITE;
	if (Thresholds.fields.Vdown_operational != ThresholdsIndexUpdate.fields.Vdown_operational) return ERR_FRAM_WRITE;
	if (Thresholds.fields.Vup_cruise != ThresholdsIndexUpdate.fields.Vup_cruise) return ERR_FRAM_WRITE;
	if (Thresholds.fields.Vup_operational != ThresholdsIndexUpdate.fields.Vup_operational) return ERR_FRAM_WRITE;

	Thresholds.fields.Vdown_cruise = 30;
	Thresholds.fields.Vdown_operational = 2000;
	Thresholds.fields.Vup_cruise = 5;
	Thresholds.fields.Vup_operational = 10; //problematic threshold
	error = SetEPSThreshold(&Thresholds);
	if (error != E_PARAM_OUTOFBOUNDS) {
		RestoreDefaultThresholdVoltages();
		return ERR_OUT_BOUND;
	}
	RestoreDefaultThresholdVoltages();
	return 0;
}
Error_List EPSTest() {
	Error_List errors;
	RestoreDefaultThresholdVoltages();
	RestoreDefaultAlpha();
	errors.fields.alphaError = alphaTest();
	errors.fields.battError = batteryTest();
	errors.fields.TresholdError = thresholdTest();
	return errors;
}
