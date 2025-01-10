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

/*
 * save log (error + error msg) -> in FS
 * @param[in] name=error; type=int; enter the error code for the problem
 * @param[in] name=msg; type=char*; have the msg of which function had error and where. (max length 66)
 * @return type=int; return the error we got.
 * */
int logError(int error ,char* msg)
{
	if(error == 0 || error == 6) return 0; //6 in not a real error
	logData_t data;
	data.error = error;
	memset(data.msg, 0, MAX_LOG_STR);
	sprintf(data.msg, "%s", msg);

	printf("%d ERROR - %s\r\n", error, data.msg);

	Write2File(&data, tlm_log);
	return error;
}

/*
 * convert unix time to Time struct
 * @param[in] name=utime; type=time_unix; unix time we want to convert.
 * @param[out] name=time; type=Time*; where we save the convert time.
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
