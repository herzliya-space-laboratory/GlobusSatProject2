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
#include <satellite-subsystems/IsisSolarPanelv2.h>
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
		wod->volt_5V = -1; //TODO
		wod->volt_3V3 = -1; //TODO
		wod->charging_power = -1; //TODO
		wod->current_5V = -1; //TODO
		wod->current_3V3 = -1; //TODO
	}
	else
	{
		wod->voltBattery = -1;
		wod->consumed_power = -1;
		wod->electric_current = -1;
		wod->mcu_temp = -1;
		wod->bat_temp = -1;
		wod->volt_5V = -1;
		wod->volt_3V3 = -1;
		wod->charging_power = -1;
		wod->current_5V = -1;
		wod->current_3V3 = -1;
	}

	IsisSolarPanelv2_wakeup();
	int error_sp;
	uint8_t status = 0;
	int32_t paneltemp = 0;
	float conv_temp;
	for(int panel = 0; panel < ISIS_SOLAR_PANEL_COUNT; panel++ ) //Go for the count of solar panels we have.
	{
		error_sp = IsisSolarPanelv2_getTemperature(panel, &paneltemp, &status); //Gets the temperature of each panel and the error message.
		if( error_sp ) //if there is error
		{
			wod->solar_panels[panel] = -1;
			continue;
		}
		conv_temp = (float)(paneltemp) * ISIS_SOLAR_PANEL_CONV;
		wod->solar_panels[panel] = conv_temp;
	}
	IsisSolarPanelv2_sleep(); //Puts the internal temperature sensor to sleep mode

	if(!error_supervisor)
	{
		wod->number_of_resets = mySupervisor_housekeeping_hk.fields.iobcResetCount;
		wod->sat_uptime = mySupervisor_housekeeping_hk.fields.iobcUptime / portTICK_RATE_MS;
	}
	else
	{
		wod->number_of_resets = -1;
		wod->sat_uptime = -1;
	}

	if(!ret)
	{
		wod->free_memory = space.free;
		wod->corrupt_bytes = space.bad;
		wod->total_memory = space.total;
		wod->used_bytes = space.used;
	}
	else
	{
		wod->free_memory = -1;
		wod->corrupt_bytes = -1;
		wod->total_memory = -1;
		wod->used_bytes = -1;
	}
	if(logError(Time_getUnixEpoch((unsigned int*)&wod->sat_time), "TelemetryCollector - Time_getUnixEpoch"))
	{
		wod->sat_time = -1;
	}
	wod->num_of_cmd_resets = -1;
	/*
	unsigned int num_of_cmd_resets;
	 */
	return 0;
}
