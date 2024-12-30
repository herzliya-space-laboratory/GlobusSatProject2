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
	logError(FRAM_read((unsigned char*)&reset, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "WakeupFromResetCMD - fram_read");
	reset += 1;
	logError(FRAM_writeAndVerify((unsigned char*)&reset, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	int flagCMDReset;
	logError(FRAM_read((unsigned char*)&flagCMDReset, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "WakeupFromResetCMD - fram_read");
	if(!flagCMDReset) return SendAckPacket(ACK_RESET_WAKEUP , NULL, (unsigned char*)&time, sizeof(time));

	flagCMDReset = 0;
	logError(FRAM_writeAndVerify((unsigned char*)&flagCMDReset, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	int cmdReset = 0;
	logError(FRAM_read((unsigned char*)&cmdReset, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE), "WakeupFromResetCMD - fram_read");
	cmdReset += 1;
	logError(FRAM_writeAndVerify((unsigned char*)&cmdReset, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE), "WakeupFromResetCMD - FRAM_writeAndVerify");

	return SendAckPacket(ACK_RESET_WAKEUP , NULL, (unsigned char*)&time, sizeof(time));

}


/*!
 * @brief Calls the relevant functions in a serial order
 */
void Maintenance()
{
	F_FIND find;
	time_unix timeNow;
	int error = logError(Time_getUnixEpoch((unsigned int*)&timeNow), "Maintenance - Time_getUnixEpoch");
	if(error) return;
	timeNow -= (30 * 24 * 3600);
	if(timeNow < UNIX_SECS_FROM_Y1970_TO_Y2000) return; //check we are not pass 2000
	Time timeTo;
	timeU2time(timeNow, &timeTo); //To get the time in Time struct
	char fileName[8] = {0};
	CalculateFileName(timeTo, fileName, "", 0);  //get a fileName without ending but with .
	char fileTime[6] = {0}; //only take the start of the file without the point or ending.
	memcpy(fileTime, fileName, 6);
	char final[8] = {0}; //the string we want to found
	sprintf(final, "%s.*", fileTime);
	error = f_findfirst(final, &find); // Search and if doesn't have, exit the function
	if(error) return;
	error = f_findfirst("*.*", &find); // find the first file
	if(error) return;
	int flagCon;
	do{
		flagCon = FALSE;
		for(int i = 0; i < 6; i++)
		{
			if(find.filename[i] != fileName[i])
			{
				flagCon = TRUE;
				break;
			}
		}
		f_delete(find.filename); //delete
		if(!flagCon) break; //see if that was the last one to check

	}
	while(!f_findnext(&find)); //get next file and see if we can
	error = f_findfirst(final, &find); // find the first file
	if(error) return;
	do{
		f_delete(find.filename); //delete
	}
	while(!f_findnext(&find));
}
