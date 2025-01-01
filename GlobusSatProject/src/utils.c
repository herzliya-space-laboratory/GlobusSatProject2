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
#include "utils.h"
#include "TLM_management.h"
#include <string.h>

int logError(int error ,char msg[MAX_LOG_STR]) //TODO check
{
	if(error == 0 || error == 6) return 0; //6 in not a real error
	logData_t data;
	data.error = error;
	memcpy(data.msg, msg, MAX_LOG_STR);
	Write2File(&data, tlm_log);
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
