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


EpsState_t GetSystemState() {
	return currentState;
}
Boolean GetLowVoltFlag() {
	return GetSystemState() == PowerSafeMode;
}

int EnterOperationalMode() {
	currentState = OperationalMode;
	printf("you are on the operationalMode\r\n");
	return 0;

}

int EnterCruiseMode() {
	currentState = CruiseMode;
	printf("you entered cruiseMode\r\n");
	return 0;
}

int EnterPowerSafeMode() {
	currentState = PowerSafeMode;
	muteTRXVU()
	printf("EnterPowerSafeMode\r\n");
	return 0;
}
