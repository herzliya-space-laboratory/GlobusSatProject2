/*
 * TelemetryCollector.c
 *
 *  Created on: 31 7 2024
 *      Author: maayan
 */

#include "TelemetryCollector.h"

#include <hal/supervisor.h>
#include <hcc/api_fat.h>
#include <satellite-subsystems/imepsv2_piu.h>
#include "utils.h"

int GetCurrentWODTelemetry(WOD_Telemetry_t *wod)
{
	//TODO: finish the function
	supervisor_housekeeping_t mySupervisor_housekeeping_hk; //create a variable that is the struct we need from supervisor
	int error_supervisor = logError(Supervisor_getHousekeeping(&mySupervisor_housekeeping_hk, SUPERVISOR_SPI_INDEX), "GetCurrentWODTelemetry - Supervisor_getHousekeeping"); //gets the variables to the struct and also check error.
	F_SPACE space; //same just to SD
	int ret = logError(f_getfreespace(f_getdrive(), &space), "GetCurrentWODTelemetry - f_getfreespace"); //gets the variables to the struct
	imepsv2_piu__gethousekeepingeng__from_t responseEPS; //Create a variable that is the struct we need from EPS_isis
	int error_eps = logError(imepsv2_piu__gethousekeepingeng(0,&responseEPS), "GetCurrentWODTelemetry - imepsv2_piu__gethousekeepingeng"); //Get struct and get kind of error
	if(!error_eps)
	{
		wod->voltBattery = responseEPS.fields.batt_input.fields.volt;
		wod->consumed_power = responseEPS.fields.dist_input.fields.power * 10;
		wod->electric_current = responseEPS.fields.batt_input.fields.current;
		wod->mcu_temp = ((double)responseEPS.fields.temp) * 0.01;
		wod->bat_temp = ((double)responseEPS.fields.temp2) * 0.01;
		/*
		 * TODO: wod->volt_5V
		 * TODO: wod->volt_3V3
		 * TODO: wod->charging_power
		 *
		 */

	}
	else
	{
		wod->voltBattery = -1;
		wod->consumed_power = -1;
		wod->electric_current = -1;
	}
	//TODO: need to add Solar_Panels
	if(!error_supervisor)
	{

	}
	return 0;
}
