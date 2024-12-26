/*
 * TelemetryCollector.c
 *
 *  Created on: 31 7 2024
 *      Author: maayan
 */

#include "TelemetryCollector.h"
#include <hal/supervisor.h>
#include <hcc/api_fat.h>
#include <satellite-subsystems/isismepsv2_ivid7_piu.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include "utils.h"

time_unix lastTimeSave[sizeof(tlm_type_t)] = {0};


PeriodTimes periods;

time_unix GetTime()
{
	time_unix time = 0;
	logError(Time_getUnixEpoch((unsigned int*)&time), "TelemetryCollector - Time_getUnixEpoch");
	return time;
}

/**
 * get all tlm save time periods from FRAM
 */
void InitSavePeriodTimes()
{
	logError(FRAM_read(periods.raw, TLM_SAVE_PERIOD_START_ADDR, sizeof(periods.raw)), "InitSavePeriodTimes - FRAM_read");
}

/*!
 * @brief Gets all necessary telemetry and arranges it into a WOD structure
 * @param[out] name=wod; type=WOD_Telemetry_t*; output WOD telemetry. If an error occurred while getting TLM the fields will be -1
 * @return 0
 */
int GetCurrentWODTelemetry(WOD_Telemetry_t *wod)
{
	//TODO: finish the function
	supervisor_housekeeping_t mySupervisor_housekeeping_hk; //create a variable that is the struct we need from supervisor
	int error_supervisor = logError(Supervisor_getHousekeeping(&mySupervisor_housekeeping_hk, SUPERVISOR_SPI_INDEX), "GetCurrentWODTelemetry - Supervisor_getHousekeeping"); //gets the variables to the struct and also check error.
	F_SPACE space; //same just to SD
	int ret = logError(f_getfreespace(f_getdrive(), &space), "GetCurrentWODTelemetry - f_getfreespace"); //gets the variables to the struct
	isismepsv2_ivid7_piu__gethousekeepingeng__from_t responseEPS; //Create a variable that is the struct we need from EPS_isis
	int error_eps = logError(isismepsv2_ivid7_piu__gethousekeepingeng(0,&responseEPS), "GetCurrentWODTelemetry - imepsv2_piu__gethousekeepingeng"); //Get struct and get kind of error
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
	else // if have error in the eps put everything in that section to -1
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
	IsisSolarPanelv2_Error_t error_sp;
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
		wod->sat_uptime = mySupervisor_housekeeping_hk.fields.iobcUptime / portTICK_RATE_MS;
	else // if have error in the supervisor put everything in that section to -1
		wod->sat_uptime = -1;

	if(!ret)
	{
		wod->free_memory = space.free;
		wod->corrupt_bytes = space.bad;
		wod->total_memory = space.total;
		wod->used_bytes = space.used;
	}
	else // if have error in the file_system put everything in that section to -1
	{
		wod->free_memory = -1;
		wod->corrupt_bytes = -1;
		wod->total_memory = -1;
		wod->used_bytes = -1;
	}
	if(logError(Time_getUnixEpoch((unsigned int*)&wod->sat_time), "TelemetryCollector - Time_getUnixEpoch")) // if have error in geting the sat time put the time to -1
		wod->sat_time = -1;

	unsigned int numberOfResets;
	unsigned int numberOfCMDResets;
	if(logError(FRAM_read((unsigned char*)&numberOfResets, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "GetCurrentWODTelemetry - FRAM_read resets")) wod->number_of_resets = -1;
	else wod->number_of_resets = numberOfResets;
	if(logError(FRAM_read((unsigned char*)&numberOfCMDResets, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_ADDR), "GetCurrentWODTelemetry - FRAM_read cmd resets")) wod->num_of_cmd_resets = -1;
	else wod->num_of_cmd_resets = numberOfCMDResets;

	return 0;
}

void TelemetrySaveWOD()
{
	time_unix time = GetTime();
	if(time == 0) return;
	WOD_Telemetry_t wod;
	if(GetCurrentWODTelemetry(&wod)) return;
	Write2File(&wod, tlm_wod);
	lastTimeSave[tlm_wod] = time;
}

void TelemetrySaveEPS()
{
	imepsv2_piu__gethousekeepingeng__from_t responseEPS; //Create a variable that is the struct we need from EPS_isis
	time_unix time = GetTime();
	if(time == 0) return;
	if(!logError(imepsv2_piu__gethousekeepingeng(0,&responseEPS), "TelemetrySaveEPS - imepsv2_piu__gethousekeepingeng"))
	{
		Write2File(&responseEPS, tlm_eps); //Get struct and get kind of error
		lastTimeSave[tlm_eps] = time;
	}

}

void TelemetrySaveTx()
{
	ISIStrxvuTxTelemetry txTelem;
	time_unix time = GetTime();
	if(time == 0) return;
	if(!logError(IsisTrxvu_tcGetTelemetryAll(0, &txTelem), "TelemetrySaveTRXVU - IsisTrxvu_tcGetTelemetryAll"))
	{
		Write2File(&txTelem, tlm_tx);
		lastTimeSave[tlm_tx] = time;
	}
}

void TelemetrySaveRx()
{
	ISIStrxvuRxTelemetry rxTelem;
	time_unix time = GetTime();
	if(time == 0) return;
	if(!logError(IsisTrxvu_rcGetTelemetryAll(0, &rxTelem), "TelemetrySaveTRXVU - IsisTrxvu_rcGetTelemetryAll"))
	{
		Write2File(&rxTelem, tlm_rx);
		lastTimeSave[tlm_rx] = time;
	}
}

void TelemetrySaveAnt()
{
	ISISantsTelemetry antsTelem;
	time_unix time = GetTime();
	if(time == 0) return;
	if(!logError(IsisAntS_getAlltelemetry(0, isisants_sideA, &antsTelem), "TelemetrySaveANT - IsisAntS_getAlltelemetry"))
	{
		Write2File(&antsTelem, tlm_antenna);
		lastTimeSave[tlm_antenna] = time;
	}
}

void TelemetrySaveSolarPanels()
{
	time_unix time = GetTime();
	if(time == 0) return;
	IsisSolarPanelv2_wakeup();
	int error_sp;
	uint8_t status = 0;
	int32_t paneltemp = 0;
	float conv_temp;
	solar_tlm_t tempSolar;
	for(int panel = 0; panel < ISIS_SOLAR_PANEL_COUNT; panel++ ) //Go for the count of solar panels we have.
	{
		error_sp = IsisSolarPanelv2_getTemperature(panel, &paneltemp, &status); //Gets the temperature of each panel and the error message.
		if(error_sp) //if there is error
		{
			tempSolar.values[panel] = -1;
			continue;
		}
		conv_temp = (float)(paneltemp) * ISIS_SOLAR_PANEL_CONV;
		tempSolar.values[panel] = conv_temp;
	}
	IsisSolarPanelv2_sleep(); //Puts the internal temperature sensor to sleep mode
	Write2File(&tempSolar, tlm_solar);
	lastTimeSave[tlm_solar] = time;
}

void TelemetrySavePayloadRADFET()
{
	time_unix time = GetTime();
	if(time == 0) return;
	PayloadEnvironmentData radfetData;
	if(logError(payloadReadEnvironment(&radfetData), "TelemetrySavePayloadRADFET - payloadReadEnvironment")) return;
	Write2File(&radfetData, tlm_radfet);
	lastTimeSave[tlm_radfet] = time;
}

void GetSEL_telemetry(PayloadEventData eventsData, payloadSEL_data *selData)
{
	selData->count = eventsData.sel_count;
	if(logError(FRAM_read((unsigned char*)&selData->sat_resets_count, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "GetSEL_telemetry - FRAM_read resets")) selData->sat_resets_count = -1;
	if(logError(FRAM_read((unsigned char*)&selData->changes_in_mode, NUM_OF_CHANGES_IN_MODE_ADDR, NUM_OF_CHANGES_IN_MODE_SIZE), "GetSEL_telemetry - FRAM_read resets")) selData->changes_in_mode = -1;
}

void TelemetrySavePayloadSEL(PayloadEventData eventsData, time_unix time)
{
	payloadSEL_data selData;
	GetSEL_telemetry(eventsData, &selData);
	Write2File(&selData, tlm_sel);
	lastTimeSave[tlm_sel] = time;
}

void TelemetrySavePayloadEvents()
{
	time_unix time = GetTime();
	if(time == 0) return;
	PayloadEventData eventsData;
	if(logError(payloadReadEvents(&eventsData), "TelemetrySavePayloadEvents - payloadReadEvents")) return;
	if(CheckExecutionTime(lastTimeSave[tlm_seu], periods.fields.seu_sel))
	{
		Write2File(&eventsData.seu_count, tlm_seu);
		lastTimeSave[tlm_seu] = time;
	}
	if(CheckExecutionTime(lastTimeSave[tlm_sel], periods.fields.seu_sel))
		TelemetrySavePayloadSEL(eventsData, time);
}

void TelemetryCollectorLogic()
{
	if(CheckExecutionTime(lastTimeSave[tlm_eps], periods.fields.eps))
		TelemetrySaveEPS();
	if(CheckExecutionTime(lastTimeSave[tlm_tx], periods.fields.trxvu))
		TelemetrySaveTx();
	if(CheckExecutionTime(lastTimeSave[tlm_rx], periods.fields.trxvu))
		TelemetrySaveRx();
	if(CheckExecutionTime(lastTimeSave[tlm_antenna], periods.fields.ants))
		TelemetrySaveAnt();
	if(CheckExecutionTime(lastTimeSave[tlm_wod], periods.fields.wod))
		TelemetrySaveWOD();
	if(CheckExecutionTime(lastTimeSave[tlm_solar], periods.fields.solar_panels))
		TelemetrySaveSolarPanels();

	if(CheckExecutionTime(lastTimeSave[tlm_radfet], periods.fields.radfet))
		TelemetrySavePayloadRADFET();

	TelemetrySavePayloadEvents();


}
