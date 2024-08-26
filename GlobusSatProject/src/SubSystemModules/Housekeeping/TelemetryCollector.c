/*
 * TelemetryCollector.c
 *
 *  Created on: 31 7 2024
 *      Author: maayan
 */

#include "TelemetryCollector.h"

#include <hal/supervisor.h>
#include <hcc/api_fat.h>


int GetCurrentWODTelemetry(WOD_Telemetry_t *wod)
{
	//TODO: finish the function
	supervisor_housekeeping_t mySupervisor_housekeeping_hk; //create a variable that is the struct we need from supervisor
	int error = Supervisor_getHousekeeping(&mySupervisor_housekeeping_hk, SUPERVISOR_SPI_INDEX); //gets the variables to the struct and also check error.
	F_SPACE space; //same just to SD
	int ret = f_getfreespace(f_getdrive(), &space); //gets the variables to the struct
	return error;
}
