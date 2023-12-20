/*
 * isis_OBC_demo.c
 *
 *  Created on: 20 בדצמ 2023
 *      Author: maaya
 */
#include "isis_OBC_demo.h"

#include <satellite-subsystems/GomEPS.h>

#include <hal/errors.h>
#include <hal/Utility/util.h>
#include <stdio.h>

/*Boolean IsisOBCdemoMain(void)
{
	if(IsisOBCdemoInit())									// initialize of I2C and IsisOBC subsystem drivers succeeded?
	{
		IsisOBCdemoLoop();								// show the main IsisOBC demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}*/

static Boolean PrintBeacon(void)
{
	gom_eps_hk_t myEpsStatus_hk;
	printf("\n\r EPS: \n\r");
	printf("\tVolt battery [mV]:  %lf\r\n", (double)(myEpsStatus_hk.fields.vbatt));
	printf("\tCurrent [mA]: %d\r\n", (int)(myEpsStatus_hk.fields.curin));
	return TRUE;
}

static Boolean selectAndExecuteTRXVUDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Print beacon \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 1) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = PrintBeacon();
		break;
	default:
		break;
	}

	return offerMoreTests;
}

void IsisOBCdemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteTRXVUDemoTest();	// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean OBCtest(void)
{
	IsisOBCdemoLoop();
	//IsisOBCdemoMain();
	return TRUE;
}
