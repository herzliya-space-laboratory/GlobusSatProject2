/*
 * InitSystem.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <hal/Storage/FRAM.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <hal/Utility/util.h>
#include <hal/supervisor.h>

#include <hcc/api_fat.h>

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"

#include "TLM_management.h"

#include "GlobalStandards.h"
#include "InitSystem.h"
#include "utils.h"

#define TESTING

#define I2CBusSpeed_Hz 100000
#define I2CTransferTimeout 10
#define TIME_SYNCINTERVAL  60

int antsOn = 1;
int deploy = 1;

int StartFRAM(){
	return logError(FRAM_start(), "FRAM - FRAM_start");
}

int StartI2C(){
	return logError(I2C_start(I2CBusSpeed_Hz, I2CTransferTimeout), "I2C - I2C_start");
}

int StartSPI(){
	return logError(SPI_start(bus1_spi, slave1_spi), "SPI - SPI_start");
}

int StartTIME(){
	const Time time = UNIX_DATE_JAN_D1_Y2000;
	return logError(Time_start(&time, TIME_SYNCINTERVAL), "Time - Time_start");
}

int InitSupervisor()
{
	uint8_t po = SUPERVISOR_SPI_INDEX;
	int error = Supervisor_start(&po, 0);
	return logError(error, "Supervisor - Supervisor_start");
}

int WriteDefaultValuesToFRAM()
{
	int zero = 0;
	time_unix param = 0;
	int error = 0;
	if(FRAM_writeAndVerify((unsigned char*)&param, TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE)) error = -1;
	if(FRAM_writeAndVerify((unsigned char*)&param, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE)) error = -1;
	if(FRAM_writeAndVerify((unsigned char*)&param, IDLE_END_TIME_ADDR, IDLE_END_TIME_SIZE)) error = -1;

	int beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	if(logError(FRAM_writeAndVerify((unsigned char*)&beacon_interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE), "default to FRAM - beacon")) error = -1;

	int rssi = DEFAULT_RSSI_VALUE;
	if(logError(FRAM_writeAndVerify((unsigned char*)&rssi, TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE), "default to FRAM - rssi")) error = -1;

	float alpha = DEFAULT_ALPHA_VALUE;
	if(logError(FRAM_writeAndVerify((unsigned char*)&alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE), "default to FRAM - alpha")) error = -1;

#ifdef TESTING
	int timeDeploy = MIN_2_WAIT_BEFORE_DEPLOY;
#else
	int timeDeploy = MIN_2_WAIT_BEFORE_DEPLOY*60;
#endif
	if(logError(FRAM_writeAndVerify((unsigned char*)&timeDeploy, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE), "default to FRAM - deploy time")) error = -1;

	//if(logError(FRAM_writeAndVerify((unsigned char*)&0, SECONDS_SINCE_DEPLOY_ADDR, SECONDS_SINCE_DEPLOY_SIZE), "default to FRAM - seconds since deploy")) error = -1;
//Need to be written with the firstActivetion that will become 1.

	if(logError(FRAM_writeAndVerify((unsigned char*)&zero, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "default to FRAM - number of resets")) error = -1;

	if(logError(FRAM_writeAndVerify((unsigned char*)&zero, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_ADDR), "default to FRAM - number of cmd resets")) error = -1;

	if(logError(FRAM_writeAndVerify((unsigned char*)&zero, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "default to FRAM - cmd reset flag")) error = -1;

	int arrPeriod[7] = {DEFAULT_EPS_SAVE_TLM_TIME, DEFAULT_TRXVU_SAVE_TLM_TIME, DEFAULT_ANT_SAVE_TLM_TIME, DEFAULT_SOLAR_SAVE_TLM_TIME, DEFAULT_WOD_SAVE_TLM_TIME, DEFAULT_RADFET_SAVE_TLM_TIME, DEFAULT_SEU_SEL_SAVE_TLM_TIME};
	if(logError(FRAM_writeAndVerify((unsigned char*)arrPeriod, TLM_SAVE_PERIOD_START_ADDR, sizeof(arrPeriod)), "default to FRAM - save TLM periods")) error = -1;

	if(logError(FRAM_writeAndVerify((unsigned char*)&zero, TRANS_ABORT_FLAG_ADDR, TRANS_ABORT_FLAG_SIZE), "default to FRAM - transmission abort flag")) error = -1;

	if(logError(FRAM_writeAndVerify((unsigned char*)&zero, NUM_OF_CHANGES_IN_MODE_ADDR, NUM_OF_CHANGES_IN_MODE_SIZE), "default to FRAM - transmission abort flag")) error = -1;

	voltage_t defaultThershold[NUMBER_OF_THRESHOLD_VOLTAGES] = DEFAULT_EPS_THRESHOLD_VOLTAGES;
	EpsThreshVolt_t thresh;
		for(int i = 0; i < NUMBER_OF_THRESHOLD_VOLTAGES; i++)
			thresh.raw[i] = defaultThershold[i];
	if(logError(FRAM_writeAndVerify((unsigned char*)&thresh, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE), "default to FRAM - threshold voltages")) error = -1;
	//TODO: LAST_COMM_TIME_ADDR
	return error;
}
int AntArm()
{
#ifdef TESTING
	antsOn = 0;
	if(antsOn)
	{
		printf("Ants not armed\r\n");
		return -1;
	}
	printf("ants: %d\r\n", !antsOn);
#else
	int rv = isis_ants__arm(0);
	if(rv)
	{
		printf("Ants not armed\r\n");
		return -1;
	}
#endif
	return 0;
}
int AntDeployment()
{
#ifdef TESTING
	deploy = 0;
	if(deploy)
	{
		printf("Ants not deployed\r\n");
		return -1;
	}
	printf("ants: %d\r\n", !deploy);
#else
	int rv = isis_ants__start_auto_deploy(0, 10); //todo: need to check redandent
	if(rv)
	{
		//TODO: use the second function for deploy and then if not work try again, when we are with the new drivers
		printf("Ants not deployed\r\n");
		return -1;
	}
#endif

	return 0;
}

int FirstActivation()
{
	int zero = 0;
	int firstActiveFlag;
	FRAM_read((unsigned char*)&firstActiveFlag, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);
	logError(FRAM_writeAndVerify((unsigned char*)&zero, SECONDS_SINCE_DEPLOY_ADDR, SECONDS_SINCE_DEPLOY_SIZE), "FirstActivition - seconds since deploy");
	if(!firstActiveFlag)
		return 0;
	int error = 0;
	Delete_allTMFilesFromSD();

#ifdef WE_HAVE_ANTS
	int max;
	int time = 0;
	FRAM_read((unsigned char*)&max, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE);
	do
	{
		FRAM_read((unsigned char*)&time, SECONDS_SINCE_DEPLOY_ADDR, SECONDS_SINCE_DEPLOY_SIZE);
		vTaskDelay(5000 / portTICK_RATE_MS);
		time += 5;
		TelemetryCollectorLogic();
		if(logError(FRAM_writeAndVerify((unsigned char*)&time, SECONDS_SINCE_DEPLOY_ADDR, SECONDS_SINCE_DEPLOY_SIZE), "FirstActivition - seconds since deploy")) error = -1;
#ifdef TESTING
		if(time == 60) gracefulReset();
#endif
	}
	while(max > time);
	while(AntArm() == -1);
	while(AntDeployment() == -1);
#endif
	if(logError(FRAM_writeAndVerify((unsigned char*)&zero, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE), "FirstActivition - first activation flag = 0")) error = -1;
	return error;
}

int InitSubsystems(){
	StartI2C();

	StartSPI();

	StartFRAM();

	StartTIME();

	InitializeFS();
	//TODO: DELETE THE LINES OF THE FRAM WRITE BELOW. ONLY TO CHECK FIRST ACTIVETION!!!!!
	int one = 1;
	logError(FRAM_writeAndVerify((unsigned char*)&one, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE), "first activation flag = 1");

	int firstActiveFlag;
	FRAM_read((unsigned char*)&firstActiveFlag, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);
	if(firstActiveFlag)
	{
		WriteDefaultValuesToFRAM();
		//Move before the delete of all files
	}

	InitSupervisor();

	InitTrxvuAndAnts();

	EPS_And_SP_Init();

	WakeupFromResetCMD();

	payloadInit();

	FirstActivation();

	printf("Did init\r\n");
	return 0;
}
