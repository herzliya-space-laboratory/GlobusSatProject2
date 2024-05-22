/*
 * payload_demo.c
 *
 *  Created on: 15 MAY 2024
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
typedef struct __attribute__ ((__packed__))
{
    unsigned int counter;
} count_seu_upset;
typedef struct __attribute__ ((__packed__))
{
    unsigned int RED1;
    unsigned int RED2;
} red_telementry;
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
		printf("ERROR - %d \r\n", error);
	}
	else
	{
		printf("RADFAT temperature - %d \r\n", telemetry->temp);
	}
	return TRUE;
}

static Boolean GetSEUCountChangeBits()
{
	I2Ctransfer structCounter;
	unsigned char readData[4] = {0};
	unsigned char writeData[1] = {0x47};

	count_seu_upset* telemetry = (count_seu_upset*)(&readData[0]);

	structCounter.slaveAddress = 0x55;
	structCounter.writeSize = 1;
	structCounter.writeData = writeData;
	structCounter.readData = readData;
	structCounter.readSize = 4;
	structCounter.writeReadDelay = 100 / portTICK_RATE_MS;
	int error = I2C_writeRead(&structCounter);
	if(error)
	{
		printf("ERROR - %d \r\n", error);
	}
	else
	{
		printf("SEU counter upset - %d \r\n", telemetry->counter);
	}
	return TRUE;
}
/*
 * GetREDFromREDFET
 * @short reads the radiation from the payload (I2C opcode 0x47)
 * @param None
 * @return TRUE
 */
static Boolean GetREDromRADFET()
{
	I2Ctransfer structRED;
	unsigned char readData[8] = {0};
	unsigned char writeData[1] = {0x33};

	red_telementry* telemetry = (red_telementry*)(&readData[0]);

	structRED.slaveAddress = 0x55;
	structRED.writeSize = 1;
	structRED.writeData = writeData;
	structRED.readData = readData;
	structRED.readSize = 8;
	structRED.writeReadDelay = 1250 / portTICK_RATE_MS;
	int error = I2C_writeRead(&structRED);
	if(error)
	{
		printf("ERROR - %d \r\n", error);
	}
	else
	{
		printf("RED from payload1 - %d \r\n RED from payload2  %d \r\n", telemetry->RED1, telemetry->RED2);
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
	printf("\t 2) Print count changes bits (SEU) \n\r");
	printf("\t 3) Print RED amount from both payload (total RED check) \n\r");
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 3) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = GetRadfatTemp();
		break;
	case 2:
		offerMoreTests = GetSEUCountChangeBits();
		break;
	case 3:
		offerMoreTests = GetREDromRADFET();
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
