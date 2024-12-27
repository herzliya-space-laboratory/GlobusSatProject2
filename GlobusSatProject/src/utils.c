/*
 * utils.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include <stdio.h>
#include <time.h>
#include "GlobalStandards.h"
#include <hal/Timing/Time.h>

int logError(int error ,char* msg)
{
	if(error != 0 && error != 6)
	{
		printf("%s - ERROR %d\r\n", msg, error);
	}
	return error;
}

/*
 * convert unix time to Time struct
 */
void timeU2time(time_unix utime, Time *time)
{
	struct tm *t = localtime((time_t*)&utime);
	time->year = t->tm_year - 100;
	time->month = t->tm_mon + 1;
	time->date = t->tm_mday;
	time->hours = t->tm_hour;
	time->minutes = t->tm_min;
	time->seconds = t->tm_sec;
	time->day = t->tm_wday;
}
