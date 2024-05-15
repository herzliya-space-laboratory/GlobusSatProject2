/*
 * payload_demo.c
 *
 *  Created on: 15 ·Ó‡È 2024
 *      Author: maayan
 */
#include "payload_demo.h"

static Boolean selectAndExecutePayloadDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\rSelect a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
/*	printf("\t 1) Print beacon \n\r");*/
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 2) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between†the†two.

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
/*	case 1:
		offerMoreTests;
		break;*/
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
