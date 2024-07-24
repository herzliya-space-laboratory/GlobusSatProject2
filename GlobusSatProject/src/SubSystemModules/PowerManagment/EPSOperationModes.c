/*
 * EPSOperationMode.c
 *
 *  Created on: 5 june 2024
 *
 */

#include "EPSOperationModes.h"
#include <stdio.h>
#include <stdbool.h>

EpsState_t currentState;
EpsMode_t currentMode;

EpsState_t GetSystemState() {
	return currentState;
}
EpsMode_t GetcurrentMode() {
	return currentMode;
}


Boolean GetLowVoltFlag() {
	return GetSystemState() == PowerSafeMode;
}

int EnterOperationalMode() {
	if(currentMode == AutmaticMode) {
		currentState = OperationalMode;
		printf("you are on the operationalMode\r\n");
	}
	return 0;


}

int EnterCruiseMode() {
	if(currentMode == AutmaticMode) {
		currentState = CruiseMode;
		printf("you entered cruiseMode\r\n");
	}
	return 0;
}

int EnterPowerSafeMode() {
	if(currentMode == AutmaticMode) {
		currentState = PowerSafeMode;
		printf("EnterPowerSafeMode\r\n");
	}
	return 0;
}
int EnterManualMode(EpsState_t State) {
	currentMode = ManualMode;
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
	currentMode = AutmaticMode;
	return 0;
}
int EnterAutomaticMode() {
	currentMode = ManualMode;
	return 0;
}
