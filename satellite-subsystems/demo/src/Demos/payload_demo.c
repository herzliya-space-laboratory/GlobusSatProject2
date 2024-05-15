/*
 * payload_demo.c
 *
 *  Created on: 15 ·Ó‡È 2024
 *      Author: maayan
 */
#include "payload_demo.h"
#include <hal/Drivers/I2C.h>
#include "hal/errors.h"
#include "hal/Utility/util.h"

typedef struct __attribute__ ((__packed__))
{
    unsigned int temp;
} temp_telemetry;

static Boolean GetRadfatTemp()
{
	I2Ctransfer stactTemp;
	unsigned char readData[4] = {0};
	unsigned char writeData[1] = {0x77};

	temp_telemetry* telemetry = (temp_telemetry*)(&readData[0]);

	stactTemp.slaveAddress = 0x55;
	stactTemp.writeSize = 1;
	stactTemp.writeData = writeData;
	stactTemp.readData = readData;
	stactTemp.readSize = 4;
	stactTemp.writeReadDelay = 845 / portTICK_RATE_MS;
	int error = I2C_writeRead(&stactTemp);
	if(error)
	{
		printf("ERROR - %d", error);
	}
	else
	{
		printf("RADFAT temperature - %d", telemetry->temp);
	}
	return TRUE;
}

static Boolean selectAndExecutePayloadDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\rSelect a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Print RADFET temperature \n\r");
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 1) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between†the†two.

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = GetRadfatTemp();
		break;
	default:
		break;
	}

	return offerMoreTests;
}

void PayloadDemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecutePayloadDemoTest();	// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean PayloadTest(void)
{
	//need init???
	PayloadDemoLoop();

	return TRUE;
}
