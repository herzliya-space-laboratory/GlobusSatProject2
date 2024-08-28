/*
 * EPSOperationMode.c
 *
 *  Created on: 5 june 2024
 *
 */

#include "EPSOperationModes.h"
#include <stdio.h>
#include <stdbool.h>
#include "FRAM_FlightParameters.h"
#include "utils.h"


EpsState_t currentState;
EpsMode_t currentMode;

EpsState_t GetSystemState() {
	LogError(FRAM_read(&currentState, EPS_STATE_FLAG_ADDR, EPS_STATE_FLAG_SIZE), "GetSystemState, FRAM_Write");
	return currentState;
}
EpsMode_t GetcurrentMode() {
	LogError(FRAM_read(&currentMode, EPS_MODE_FLAG_ADDR, EPS_MODE_FLAG_SIZE), "GetcurrentMode, FRAM_Write");
	return currentMode;
}


Boolean GetLowVoltFlag() {
	return GetSystemState() == PowerSafeMode;
}

int EnterOperationalMode() {
	if(currentMode == AutmaticMode) {
		currentState = OperationalMode;
		LogError(FRAM_write(( unsigned char *)&currentState, EPS_STATE_FLAG_ADDR, EPS_STATE_FLAG_SIZE), "EnterOperationalMode, FRAM_Write");
		LogError(FRAM_read(( unsigned char *)&CHANGES_OPERATIONAL, EPS_CHANGES_OPERATIONAL_ADDR, EPS_CHANGES_OPERATIONAL_SIZE), "EnterOperationalMode, FRAM_Write");
		CHANGES_OPERATIONAL++;
		LogError(FRAM_write(( unsigned char *)&CHANGES_OPERATIONAL, EPS_CHANGES_OPERATIONAL_ADDR, EPS_CHANGES_OPERATIONAL_SIZE), "EnterOperationalMode, FRAM_Write");

	}
	return 0;


}

int EnterCruiseMode() {
	if(currentMode == AutmaticMode) {
		currentState = CruiseMode;
		LogError(FRAM_write(&currentState, EPS_STATE_FLAG_ADDR, EPS_STATE_FLAG_SIZE), "EnterCruiseMode, FRAM_Write");
	}
	return 0;
}

int EnterPowerSafeMode() {
	if(currentMode == AutmaticMode) {
		currentState = PowerSafeMode;
		LogError(FRAM_write(&currentState, EPS_STATE_FLAG_ADDR, EPS_STATE_FLAG_SIZE), "EnterPowerSafeMode, FRAM_Write");
		LogError(FRAM_read((unsigned char *)&CHANGES_POWERSAFE, EPS_CHANGES_POWERSAFE_ADDR, EPS_CHANGES_POWERSAFE_SIZE), "EnterPowerSafeMode, FRAM_Write");
		CHANGES_POWERSAFE++;
		LogError(FRAM_write((unsigned char *)&CHANGES_POWERSAFE, EPS_CHANGES_POWERSAFE_ADDR, EPS_CHANGES_POWERSAFE_SIZE), "EnterPowerSafeMode, FRAM_Write");
	}
	return 0;
}
int EnterManualMode(EpsState_t State) {
	currentMode = ManualMode;
	LogError(FRAM_write(&currentMode, EPS_MODE_FLAG_ADDR, EPS_MODE_FLAG_SIZE), "EnterManuelMode, FRAM_Write");
	switch (State) {
	case (OperationalMode):
		EnterOperationalMode();
		break;
	case (CruiseMode):
		EnterCruiseMode();
		break;
	case (PowerSafeMode):
		EnterPowerSafeMode();
		break;
	default:
		LogError(1, "No valid State");
	}
	return 0;
}
int EnterAutomaticMode() {
	currentMode = AutmaticMode;
	LogError(FRAM_write(&currentMode, EPS_MODE_FLAG_ADDR, EPS_MODE_FLAG_SIZE), "EnterAutomaticMode, FRAM_Write");
	return 0;
}
