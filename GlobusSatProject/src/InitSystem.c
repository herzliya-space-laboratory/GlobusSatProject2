/*
 * InitSystem.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "InitSystem.h"
#include "utils.h"

#define FIRST_ACTIVE
//#define FIRST_ACTIVE_DEPLOY

#define I2CBusSpeed_Hz 100000
#define I2CTransferTimeout 10
#define TIME_SYNCINTERVAL  60

/*!
 * @brief	Starts the FRAM using drivers, and checks for errors.
 * @see FRAM.h
 */
int StartFRAM(){
	return FRAM_start();
}

/*!
 * @brief	Starts the I2C using drivers, and checks for errors.
 * @see	I2C.h
 */
int StartI2C(){
	return I2C_start(I2CBusSpeed_Hz, I2CTransferTimeout);
}

/*!
 * @brief	Starts the SPI using drivers, and checks for errors
 * @see	SPI.h
 */
int StartSPI(){
	return SPI_start(bus1_spi, slave1_spi);
}

/*!
 * @brief	Starts the Time module using drivers, and checks for errors.
 * @see Time.h
 */
int StartTIME(){
	const Time time = UNIX_DATE_JAN_D1_Y2000;
	return Time_start(&time, TIME_SYNCINTERVAL);
}

/*
 * update sat time according to most update sat time
 * @return type=int; according to Time_setUnixEpoch errors
 * */
int UpdateTime()
{
	time_unix mostUpdated;
	if(FRAM_read((unsigned char*)&mostUpdated, MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE)) return -1;
	return Time_setUnixEpoch((unsigned int)mostUpdated);
}

/*
 * @brief	Starts the supervisor module using drivers, and checks for errors.
 * @see supervisor.h
 */
int InitSupervisor()
{
	uint8_t po = SUPERVISOR_SPI_INDEX;
	int error = Supervisor_start(&po, 0);
	return logError(error, "InitSupervisor - Supervisor_start");
}

/*!
 * @brief	writes the default flight parameters to the corresponding FRAM addresses
 * @see FRAM_FlightParameters.h
 */
int WriteDefaultValuesToFRAM()
{
	int zero = 0;
	time_unix param = 0;
	int error = 0;
	if(FRAM_writeAndVerify((unsigned char*)&param, TRANSPONDER_END_TIME_ADDR, TRANSPONDER_END_TIME_SIZE)) error += -1;
	if(FRAM_writeAndVerify((unsigned char*)&param, MUTE_END_TIME_ADDR, MUTE_END_TIME_SIZE)) error += -1;
	if(FRAM_writeAndVerify((unsigned char*)&param, IDLE_END_TIME_ADDR, IDLE_END_TIME_SIZE)) error += -1;

	int beacon_interval = DEFAULT_BEACON_INTERVAL_TIME;
	if(FRAM_writeAndVerify((unsigned char*)&beacon_interval, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE)) error += -1;

	int rssi = DEFAULT_RSSI_VALUE;
	if(FRAM_writeAndVerify((unsigned char*)&rssi, TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE)) error += -1;

	float alpha = DEFAULT_ALPHA_VALUE;
	if(FRAM_writeAndVerify((unsigned char*)&alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE)) error += -1;

	int timeDeploy = MIN_2_WAIT_BEFORE_DEPLOY*60;

	if(FRAM_writeAndVerify((unsigned char*)&timeDeploy, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE)) error += -1;

	time_unix mostUpdated = UNIX_SECS_FROM_Y1970_TO_Y2000;
	if(FRAM_writeAndVerify((unsigned char*)&mostUpdated, MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE)) error += -1;

	Boolean false = FALSE;
	if(FRAM_writeAndVerify((unsigned char*)&false, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE)) error += -1;

	int arrPeriod[7] = {DEFAULT_EPS_SAVE_TLM_TIME, DEFAULT_TRXVU_SAVE_TLM_TIME, DEFAULT_ANT_SAVE_TLM_TIME, DEFAULT_SOLAR_SAVE_TLM_TIME, DEFAULT_WOD_SAVE_TLM_TIME, DEFAULT_RADFET_SAVE_TLM_TIME, DEFAULT_SEU_SEL_SAVE_TLM_TIME};
	if(FRAM_writeAndVerify((unsigned char*)arrPeriod, TLM_SAVE_PERIOD_START_ADDR, sizeof(arrPeriod))) error += -1;

	if(FRAM_writeAndVerify((unsigned char*)&zero, TRANS_ABORT_FLAG_ADDR, TRANS_ABORT_FLAG_SIZE)) error += -1;

	if(FRAM_writeAndVerify((unsigned char*)&zero, NUM_OF_CHANGES_IN_MODE_ADDR, NUM_OF_CHANGES_IN_MODE_SIZE)) error += -1;

	if(FRAM_writeAndVerify((unsigned char*)&zero, SD_CARD_USED_ADDR, SD_CARD_USED_SIZE)) error += -1;

	voltage_t defaultThershold[NUMBER_OF_THRESHOLD_VOLTAGES] = DEFAULT_EPS_THRESHOLD_VOLTAGES;
	EpsThreshVolt_t thresh;
	for(int i = 0; i < NUMBER_OF_THRESHOLD_VOLTAGES; i++)
		thresh.raw[i] = defaultThershold[i];
	if(FRAM_writeAndVerify((unsigned char*)&thresh, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE)) error += -1;

	int lastRadfet[5] = {0};
	if(FRAM_writeAndVerify((unsigned char*)&lastRadfet, LAST_RADFET_READ_START, sizeof(lastRadfet))) error += -1;

	if(FRAM_writeAndVerify((unsigned char*)&mostUpdated, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE)) error += -1;

	unsigned int defaultTime = DEFAULT_NO_COMM_WDT_KICK_TIME;
	if(FRAM_writeAndVerify((unsigned char*)&defaultTime, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE)) error += -1;

	Boolean true = TRUE;
	if(FRAM_writeAndVerify((unsigned char*)&true, TRY_TO_DEPLOY_ADDR, TRY_TO_DEPLOY_SIZE)) error += -1;

	if(FRAM_writeAndVerify((unsigned char*)&mostUpdated, LAST_TRY_TO_DEPLOY_TIME_ADDR, LAST_TRY_TO_DEPLOY_TIME_SIZE)) error += -1;

	return error;
}

/*
 * arm ants according to which side (0/1)
 * @param[in] name=side; type=uint8_t; side of the ants (0/1)
 * @return type=int; 0 on success -1 on error
 * */
int AntArm(uint8_t side)
{
#ifdef FIRST_ACTIVE_DEPLOY
	int rv = logError(isis_ants__arm(side), "AntArm - isis_ants__arm");
	if(rv)
	{
		printf("Ants not armed\r\n");
		return -1;
	}
	printf("Ant %d arm\r\n", side);
#else
	printf("Armed (test)\r\n");
#endif
	return 0;
}

/*
 * deploy ants according to which side (0/1)
 * @param[in] name=side; type=uint8_t; side of the ants (0/1)
 * @return type=int; 0 on success -1 on error
 * */
int AntDeployment(uint8_t side)
{

#ifdef FIRST_ACTIVE_DEPLOY
	int rv = logError(isis_ants__start_auto_deploy(side, 10), "AntDeployment - isis_ants__start_auto_deploy");
	if(rv)
	{
		printf("Ants not deployed\r\n");
		return -1;
	}
	printf("Ant %d deploy\r\n", side);
#else
	printf("Deploy (test)\r\n");
#endif

	return 0;
}

/*!
 * @brief	deployment procedure
 * @return	0 successful deployment
 * 			-1 failed to deploy
 */
int FirstActivation()
{
	Boolean false = FALSE;
	int firstActiveFlag;
	FRAM_read((unsigned char*)&firstActiveFlag, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);
	if(!firstActiveFlag)
		return 0;
	int error = 0;
#ifdef FIRST_ACTIVE
	int max;
	int time = 0;
	FRAM_read((unsigned char*)&max, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE);
	do
	{
		FRAM_read((unsigned char*)&time, SECONDS_SINCE_DEPLOY_ADDR, SECONDS_SINCE_DEPLOY_SIZE);
		vTaskDelay(5000 / portTICK_RATE_MS);
		time += 5;
		TelemetryCollectorLogic();
		if(logError(FRAM_writeAndVerify((unsigned char*)&time, SECONDS_SINCE_DEPLOY_ADDR, SECONDS_SINCE_DEPLOY_SIZE), "FirstActivition - FRAM_writeAndVerify")) error = -1;
		printf("%d\r\n", time);
	}
	while(max > time);
	for(int i = 0; i < 2; i++)
	{
		AntArm(0);
		AntDeployment(0);
		AntArm(1);
		AntDeployment(1);
	}

	time_unix timeUnix;
	logError(Time_getUnixEpoch((unsigned int*)&timeUnix), "FirstActivition - Time_getUnixEpoch");
	if(logError(FRAM_writeAndVerify((unsigned char*)&timeUnix, LAST_TRY_TO_DEPLOY_TIME_ADDR, LAST_TRY_TO_DEPLOY_TIME_SIZE), "FirstActivition - FRAM_writeAndVerify")) error = -1;
#endif
	if(logError(FRAM_writeAndVerify((unsigned char*)&false, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE), "FirstActivition - FRAM_writeAndVerify - flag = 0")) error = -1;
	SendAckPacket(ACK_FINISH_FIRST_ACTIVE, NULL, NULL, 0);
	return error;
}

void payloadKillOrInit()
{
	Boolean checkPayloadFlag;
	logError(FRAM_read((unsigned char*)&checkPayloadFlag, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "payloadKillOrInit - FRAM_read");
	if(checkPayloadFlag) return;
	Boolean true = TRUE;
	Boolean flagReset;
	logError(FRAM_read((unsigned char*)&flagReset, HAD_RESET_IN_A_MINUTE_ADDR, HAD_RESET_IN_A_MINUTE_SIZE), "payloadKillOrInit - FRAM_read");
	if(!flagReset)
	{
		logError(FRAM_writeAndVerify((unsigned char*)&true, HAD_RESET_IN_A_MINUTE_ADDR, HAD_RESET_IN_A_MINUTE_SIZE), "payloadKillOrInit - FRAM_writeAndVerify");
		payloadInit();
	}
	else
		logError(FRAM_writeAndVerify((unsigned char*)&true, PAYLOAD_IS_DEAD_ADDR, PAYLOAD_IS_DEAD_SIZE), "payloadKillOrInit - FRAM_writeAndVerify");

}

/*!
 * @brief	executes all required initializations of systems, including sub-systems, and checks for errors
 * @return	0
 */
int InitSubsystems(){
	StartI2C();

	StartSPI();

	StartFRAM();

	StartTIME();

	//int firstActiveFlag = 0;
	//FRAM_read((unsigned char*)&firstActiveFlag, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);

	if(TRUE)
		WriteDefaultValuesToFRAM();
	else
		UpdateTime();

	InitializeFS();

	if(TRUE) Delete_allTMFilesFromSD();

	InitSavePeriodTimes();

	InitSupervisor();

/*	EPS_And_SP_Init();*/

	InitTrxvuAndAnts();

	WakeupFromResetCMD();

/*	payloadKillOrInit();*/

	//FirstActivation();

	logError(SAT_RESET, "InitSubsystems - reset");
	printf("Did init\r\n");
	return 0;
}
