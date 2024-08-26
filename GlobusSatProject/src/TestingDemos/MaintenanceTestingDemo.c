/*
 * MaintenanceTestingDemo.c
 *
 *  Created on: 26 8 2024
 *      Author: maayan
 */

#include "MaintenanceTestingDemo.h"
#include <hal/Timing/Time.h>
#include "utils.h"
#include <stdio.h>

Boolean CheckExecutionTimeFiveTimes()
{
	unsigned int time;
	int error;
	time_unix prev_time;
	for(int i = 0; i < 5; i++)
	{
		error = logError(Time_getUnixEpoch(&time), "Maintenance - Time_getUnixEpoch");
				if(error)
					return FALSE;
				prev_time = (time_unix)time;
		while(!CheckExecutionTime(prev_time, 10))
		{
			printf("%d\n\r", i);
		}
		printf("hello\n\r");
	}
	return TRUE;
}

void MainMaintenanceTestBench()
{
	CheckExecutionTimeFiveTimes();
}
