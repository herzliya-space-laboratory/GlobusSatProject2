
#ifndef MAINTENANCE_H_
#define MAINTENANCE_H_

#include "GlobalStandards.h"
#include <hcc/api_fat.h>

#include "TLM_management.h"

#define MIN_FREE_SPACE_PERCENTAGE 20
#define MIN_FREE_SPACE_PERCENTAGE_TO_DELETE 25
#define DEPLOY_INTRAVAL 1800 //TBD: RBF every 1800 sec (30 minutes)

#define RESET_KEY 0xA6 // need to send this key to the reset command otherwise reset will not happen

/*
 * Delete old files according to how much space we have left. (20% for start delete until we have 25% free)
 * MIN_FREE_SPACE_PERCENTAGE - the minimum free space in bytes we want to keep in the SD in all times.
 * If free space<minFreeSpace we start deleting old TLM files
 */
void DeleteOldFiles();

/*!
 * @brief checks if the period time has passed
 * @param[in] prev_exec_time last sample time (last execution time)
 * @param[in] period period of execution time
 * @return	TRUE if difference between now and last execution time is longer than 'period'
 * 			FALSE otherwise
 */
Boolean CheckExecutionTime(time_unix prev_time, unsigned int period);

/*!
 * @brief checks if there is a memory corruption in the file system.
 * @return	TRUE if is corrupted.
 * 			FALSE if no corruption (yay!)
 */
Boolean IsFS_Corrupted();

/*!
 * @brief kick the ground station communication WDT because communication took place.
*/
void KickGroundCommWDT();

/*!
 * @brief 	Checks if last GS communication time has exceeded its maximum allowed time.
 * @see		NO_COMMUNICATION_WDT_KICK_TIME
 * @return 	TRUE if a comm reset is needed- no communication for a long time
 * 			FALSE no need for a reset. last communication is within range
*/
Boolean IsGroundCommunicationWDTReset();



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

/*
 * Update most current sat time to FRAM for every time we have reset to have it.
 * */
void MostCurrentTimeToFRAM();

/*
 * Check if we need to kill the payload (turn him off for good)
 * */
void NeedToKillPayload();

#endif /* MAINTENANCE_H_ */
