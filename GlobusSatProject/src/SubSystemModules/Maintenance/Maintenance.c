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
	logError(FRAM_read((unsigned char*)&reset, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "WakeupFromResetCMD - fram_read");
	reset += 1;
	logError(FRAM_writeAndVerify((unsigned char*)&reset, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	int flagCMDReset;
	logError(FRAM_read((unsigned char*)&flagCMDReset, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "WakeupFromResetCMD - fram_read");
	if(!flagCMDReset) return logError(SendAckPacket(ACK_RESET_WAKEUP , NULL, (unsigned char*)&time, sizeof(time)), "WakeupFromResetCMD - SendAckPacket");

	flagCMDReset = 0;
	logError(FRAM_writeAndVerify((unsigned char*)&flagCMDReset, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	int cmdReset = 0;
	logError(FRAM_read((unsigned char*)&cmdReset, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE), "WakeupFromResetCMD - fram_read");
	cmdReset += 1;
	logError(FRAM_writeAndVerify((unsigned char*)&cmdReset, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	return logError(SendAckPacket(ACK_RESET_WAKEUP , NULL, (unsigned char*)&time, sizeof(time)), "WakeupFromResetCMD - SendAckPacket");

}

/*!
 * @brief Calls the relevant functions in a serial order
 */
void Maintenance()
{
	//TODO: need to delete old files here.
}
