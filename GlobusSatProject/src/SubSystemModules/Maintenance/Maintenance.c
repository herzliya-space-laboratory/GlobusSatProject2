/*
 * Maintenance.c
 *
 *  Created on: 12 8 2024
 *      Author: maayan
 */

#include "Maintenance.h"
#include <hal/Timing/Time.h>
#include <hcc/api_fat.h>
#include "utils.h"


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

Boolean IsFS_Corrupted()
{
	F_SPACE space;
	int ret = logError(f_getfreespace(f_getdrive(), &space), "IsFS_Corrupted - f_getfreespace");
	if(ret) return TRUE;
	if(space.bad) return TRUE;
	return FALSE;
}
