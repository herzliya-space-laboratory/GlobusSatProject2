
#ifndef MAINTENANCE_H_
#define MAINTENANCE_H_

#include "GlobalStandards.h"
#include <hcc/api_fat.h>


#define MIN_FREE_SPACE 300000000
#define DEPLOY_INTRAVAL 1800 //TBD: RBF every 1800 sec (30 minutes)

#define RESET_KEY 0xA6 // need to send this key to the reset command otherwise reset will not happen

/**
 * use the EPS to hard reset the MCU
 */
int HardResetMCU();

/*
 * delete onld files from SD card
 * minFreeSpace - the minimum free space in bytes we want to keep in the SD in all times.
 * If free space<minFreeSpace we start deleting old TLM files
 */
int DeleteOldFiles(int minFreeSpace);

/*!
 * @brief checks if the period time has passed
 * @param[in] prev_exec_time last sample time (last execution time)
 * @param[in] period period of execution time
 * @return	TRUE if difference between now and last execution time is longer than 'period'
 * 			FALSE otherwise
 */
Boolean CheckExecutionTime(time_unix prev_time, time_unix period);

/*
!
 * @brief reads the current UNIX time and writes it into the FRAM for future reference.

void SaveSatTimeInFRAM(unsigned int time_addr, unsigned int time_size);
*/

/*!
 * @brief checks if there is a memory corruption in the file system.
 * @return	TRUE if is corrupted.
 * 			FALSE if no corruption (yay!)
 */
Boolean IsFS_Corrupted();

/*
!
 * @brief resets the ground station communication WDT because communication took place.

void ResetGroundCommWDT();

!
 * @brief 	Checks if last GS communication time has exceeded its maximum allowed time.
 * @see		NO_COMMUNICATION_WDT_KICK_TIME
 * @return 	TRUE if a comm reset is needed- no communication for a long time
 * 			FALSE no need for a reset. last communication is within range

Boolean IsGroundCommunicationWDTKick();


int SetGsWdtKickTime(time_unix new_gs_wdt_kick_time);

time_unix GetGsWdtKickTime();
*/

/*!
 * @brief 	if a reset has been commanded and executed, SW reset or otherwise, this function will be active.
 * 			The function will lower the reset flag and send an ACK with the current UNIX time.
 * @return 	0 on success
 * 			Error code according to <hal/errors.h>
 */
int WakeupFromResetCMD();

/*!
 * @brief Calls the relevant functions in a serial order
 */
void Maintenance();

#endif /* MAINTENANCE_H_ */
