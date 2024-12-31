/*
 * Maintenance.c
 *
 *  Created on: 12 8 2024
 *      Author: maayan
 */

#include "Maintenance.h"
#include <hal/Timing/Time.h>
#include <hal/Timing/WatchDogTimer.h>
#include "SubSystemModules/Communication/AckHandler.h"
#include "utils.h"
#include <String.h>

/*!
 * @brief checks if the period time has passed
 * @param[in] prev_exec_time last sample time (last execution time)
 * @param[in] period period of execution time
 * @return	TRUE if difference between now and last execution time is longer than 'period'
 * 			FALSE otherwise
 */
Boolean CheckExecutionTime(time_unix prev_time, time_unix period)
{
	unsigned int timeNow;
	int error = logError(Time_getUnixEpoch(&timeNow), "Maintenance - Time_getUnixEpoch");
	if(error)
		return FALSE;
	if(timeNow - prev_time >= period)
		return TRUE;
	return FALSE;
}

/*!
 * @brief checks if there is a memory corruption in the file system.
 * @return	TRUE if is corrupted.
 * 			FALSE if no corruption (yay!)
 */
Boolean IsFS_Corrupted()
{
	F_SPACE space;
	int ret = logError(f_getfreespace(f_getdrive(), &space), "IsFS_Corrupted - f_getfreespace");
	if(ret) return TRUE;
	if(space.bad) return TRUE;
	return FALSE;
}

/*!
 * @brief 	if a reset has been commanded and executed, SW reset or otherwise, this function will be active.
 * 			The function will lower the reset flag and send an ACK with the current UNIX time.
 * @return 	0 on success
 * 			Error code according to <hal/errors.h>
 */
int WakeupFromResetCMD()
{
	time_unix time = 0;
	logError(Time_getUnixEpoch((unsigned int*)&time), "WakeupFromResetCMD - Time_getUnixEpoch");

	int reset = 0;
	logError(FRAM_read((unsigned char*)&reset, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "WakeupFromResetCMD - FRAM_read");
	reset += 1;
	logError(FRAM_writeAndVerify((unsigned char*)&reset, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	int flagCMDReset;
	logError(FRAM_read((unsigned char*)&flagCMDReset, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "WakeupFromResetCMD - FRAM_read");
	if(!flagCMDReset) return SendAckPacket(ACK_RESET_WAKEUP , NULL, (unsigned char*)&time, sizeof(time));

	flagCMDReset = 0;
	logError(FRAM_writeAndVerify((unsigned char*)&flagCMDReset, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	int cmdReset = 0;
	logError(FRAM_read((unsigned char*)&cmdReset, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE), "WakeupFromResetCMD - FRAM_read");
	cmdReset += 1;
	logError(FRAM_writeAndVerify((unsigned char*)&cmdReset, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	return SendAckPacket(ACK_RESET_WAKEUP , NULL, (unsigned char*)&time, sizeof(time));

}

void MostCurrentTimeToFRAM()
{
	time_unix timeNow = 0;
	if(logError(Time_getUnixEpoch((unsigned int*)timeNow), "MostCurrentTimeToFRAM - Time_getUnixEpoch")) return;
	logError(FRAM_writeAndVerify((unsigned char*)&timeNow, MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE), "MostCurrentTimeToFRAM - FRAM_writeAndVerify");
}

/*!
 * @brief Calls the relevant functions in a serial order
 */
void Maintenance()
{
	MostCurrentTimeToFRAM();

	F_SPACE space;
	int sd = f_getdrive();
	if(sd != 0 && sd != 1)
	{
		logError(sd, "Maintenance - f_getdrive");
		return;
	}
	if(logError(f_getfreespace(sd, &space), "Maintenance - f_getfreespace")) return; //gets the variables to the struct
	if(space.free >= (space.total * MIN_FREE_SPACE_PERCENTAGE) / 100) return;
	F_FIND find;
	f_findfirst("*.*", &find);
	do
	{
		f_delete(find.filename); //delete
		if(logError(f_getfreespace(sd, &space), "Maintenance - f_getfreespace")) return; //gets the variables to the struct

	}
	while((space.free < (space.total * MIN_FREE_SPACE_PERCENTAGE) / 100) && !f_findnext(&find));

}
