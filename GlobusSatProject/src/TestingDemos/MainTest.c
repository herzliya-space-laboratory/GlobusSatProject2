/*
 * MainTest.c
 *
 *  Created on: 30 Sep 2024
 *      Author: maayan
 */

#include "MainTest.h"
#include "InitSystem.h"
#include <hal/Utility/util.h>
#include <hal/Drivers/I2C.h>
#include <hal/Timing/WatchDogTimer.h>

Boolean SelectAndExecuteTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	//Initialize the I2C
	int retValInt = I2C_start(100000, 10);
	if(retValInt != 0)
	{
		TRACE_FATAL("\n\r I2C_start_Master for demo: %d! \n\r", retValInt);
	}

	printf( "\n\r Select the device to be tested to perform: \n\r");
	printf("\t 1) TRXVU test \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 1) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection)
	{
		case 1:
			IsisTRXVUTestingLoop();
			offerMoreTests = TRUE;
			break;
		default:
			break;
	}
	return offerMoreTests;
}

void taskTesting()
{
	Boolean offerMoreTests = TRUE;

	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);
	do
	{
		offerMoreTests = SelectAndExecuteTest();
	}
	while(offerMoreTests);
}
