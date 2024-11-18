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
		while(!CheckExecutionTime(prev_time, 10));
		printf("hello\n\r");
	}
	return TRUE;
}

Boolean CheckExecutionTimeFutureNumber()
{
	time_unix prev_time;
	logError(Time_getUnixEpoch((unsigned int*)&prev_time), "Maintenance - Time_getUnixEpoch");
	prev_time += 100;
	if(!CheckExecutionTime(prev_time, 0))
			return FALSE;
		return TRUE;
	return TRUE;
}

void CheckIsFSCorrupted()
{
	F_SPACE space;
	logError(f_getfreespace(f_getdrive(), &space), "IsFS_Corrupted - f_getfreespace");
	printf("%lu\r\n", space.bad);
	printf("%d\r\n", IsFS_Corrupted());
}


void MainMaintenanceTestBench()
{
	CheckExecutionTimeFiveTimes();
	CheckExecutionTimeFutureNumber();
	CheckIsFSCorrupted();
}

Boolean SelectAndExecuteMaintenance()
{
	int selection = 0;
	printf( "\n\r Select a test to perform: \n\r");
	printf("\t0) Go back to menu\n\r");
	printf("\t1)  Check execution time five times\n\r");
	printf("\t2)  Check execution time future number\n\r");
	printf("\t3)  Check if FS corrupted\n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 3) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection)
	{
		case 0:
			return FALSE;
		case 1:
			CheckExecutionTimeFiveTimes();
			break;
		case 2:
			CheckExecutionTimeFutureNumber();
			break;
		case 3:
			CheckIsFSCorrupted();
			break;
		default:
			break;
	}
	return TRUE;
}

void IsisMaintenanceTestingLoop(void)
{
	Boolean offerMoreTests = TRUE;
	while(offerMoreTests)
	{
		offerMoreTests = SelectAndExecuteMaintenance();
	}
}

