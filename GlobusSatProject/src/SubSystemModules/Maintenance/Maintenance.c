/*
 * Maintenance.c
 *
 *  Created on: 12 8 2024
 *      Author: maayan
 */

#include "Maintenance.h"
#include <hal/Timing/Time.h>
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
//TODO: need to create testing
