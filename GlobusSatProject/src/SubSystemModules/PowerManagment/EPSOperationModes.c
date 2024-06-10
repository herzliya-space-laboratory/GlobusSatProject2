/*
 * EPSOperationMode.c
 *
 *  Created on: 5 june 2024
 *
 */

#include "EPSOperationModes.h"

EpsState_t currentState;


EpsState_t GetSystemState() {
	return currentState;
}


int EnterOperationalMode() {
	currentState = OperationalMode;
	printf("you are on the operationalMode");
	return 0;

}

int EnterCruiseMode() {
	currentState = CruiseMode;
	printf("you entered cruiseMode");
	return 0;
}

int EnterPowerSafeMode() {
	currentState = PowerSafeMode;
	printf("EnterPowerSafeMode");
	return 0;
}

