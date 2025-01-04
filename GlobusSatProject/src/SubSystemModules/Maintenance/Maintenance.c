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

Boolean flag = FALSE;

/*!
 * @brief checks if the period time has passed
 * @param[in] prev_exec_time last sample time (last execution time)
 * @param[in] period period of execution time
 * @return	TRUE if difference between now and last execution time is longer than 'period'
 * 			FALSE otherwise
 */
Boolean CheckExecutionTime(time_unix prev_time, unsigned int period)
{
	unsigned int timeNow;
	int error = logError(Time_getUnixEpoch(&timeNow), "CheckExecutionTime - Time_getUnixEpoch");
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
 * @brief resets the ground station communication WDT because communication took place.
*/
void KickGroundCommWDT()
{
	time_unix timeNow;
	if(logError(Time_getUnixEpoch((unsigned int*)&timeNow), "KickGroundCommWDT - Time_getUnixEpoch")) return;
	FRAM_writeAndVerify((unsigned char*)&timeNow, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
}

/*!
 * @brief 	Checks if last GS communication time has exceeded its maximum allowed time.
 * @see		NO_COMMUNICATION_WDT_KICK_TIME
 * @return 	TRUE if a comm reset is needed- no communication for a long time
 * 			FALSE no need for a reset. last communication is within range
*/
Boolean IsGroundCommunicationWDTReset()
{
	time_unix lastComm = 0;
	if(logError(FRAM_read((unsigned char*)&lastComm, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE), "IsGroundCommunicationWDTReset - FRAM_read")) return FALSE;
	unsigned int WDTime = 0;
	if(logError(FRAM_read((unsigned char*)&WDTime, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE), "IsGroundCommunicationWDTReset - FRAM_read")) return FALSE;
	return CheckExecutionTime(lastComm, WDTime);
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

	Boolean flagCMDReset;
	logError(FRAM_read((unsigned char*)&flagCMDReset, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "WakeupFromResetCMD - FRAM_read");
	if(!flagCMDReset) return SendAckPacket(ACK_RESET_WAKEUP , NULL, (unsigned char*)&time, sizeof(time));

	flagCMDReset = FALSE;
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
	if(logError(Time_getUnixEpoch((unsigned int*)&timeNow), "MostCurrentTimeToFRAM - Time_getUnixEpoch")) return;
	logError(FRAM_writeAndVerify((unsigned char*)&timeNow, MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE), "MostCurrentTimeToFRAM - FRAM_writeAndVerify");
}

void DeleteOldFiles()
{
	F_SPACE space;
	int sd = f_getdrive();
	if(sd != 0 && sd != 1)
	{
		logError(sd, "Maintenance - f_getdrive");
		return;
	}
	if(logError(f_getfreespace(sd, &space), "Maintenance - f_getfreespace")) return; //gets the variables to the struct
	if(space.free >= ((space.total * MIN_FREE_SPACE_PERCENTAGE) / 100)) return;
	F_FIND find;
	f_findfirst("*.*", &find);
	do
	{
		f_delete(find.filename); //delete
		if(logError(f_getfreespace(sd, &space), "Maintenance - f_getfreespace")) return; //gets the variables to the struct

	}
	while((space.free < (space.total * MIN_FREE_SPACE_PERCENTAGE) / 100) && !f_findnext(&find));

}

void NeedToKillPayload()
{
	if(flag == TRUE) return;
	Boolean false = FALSE;
	supervisor_housekeeping_t mySupervisor_housekeeping_hk; //create a variable that is the struct we need from supervisor
	int err = logError(Supervisor_getHousekeeping(&mySupervisor_housekeeping_hk, SUPERVISOR_SPI_INDEX), "NeedToKillPayload - Supervisor_getHousekeeping"); //gets the variables to the struct and also check error.
	if(err) return;
	if(mySupervisor_housekeeping_hk.fields.iobcUptime / portTICK_RATE_MS > 60)
	{
		logError(FRAM_writeAndVerify((unsigned char*)&false, HAD_RESET_IN_A_MINUTE_ADDR, HAD_RESET_IN_A_MINUTE_SIZE), "NeedToKillPayload - FRAM_writeAndVerify");
		flag = TRUE;
	}
}


/*!
 * @brief Calls the relevant functions in a serial order
 */
void Maintenance()
{
	MostCurrentTimeToFRAM();

	DeleteOldFiles();

	NeedToKillPayload();

	if(IsGroundCommunicationWDTReset())
	{
		KickGroundCommWDT();
		logError(isis_vu_e__reset_hw_rx(0), "Maintenance - isis_vu_e__reset_hw_rx");
		logError(isis_vu_e__reset_hw_tx(0), "Maintenance - isis_vu_e__reset_hw_tx");
		isismepsv2_ivid7_piu__replyheader_t replyheader;
		logError(isismepsv2_ivid7_piu__reset(0, &replyheader), "Maintenance - isismepsv2_ivid7_piu__reset");
	}
}
