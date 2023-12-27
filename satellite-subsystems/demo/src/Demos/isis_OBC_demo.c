/*
 * isis_OBC_demo.c
 *
 *  Created on: 20 בדצמ 2023
 *      Author: maaya
 */
#include "isis_OBC_demo.h"


#include <satellite-subsystems/GomEPS.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>

#include <hal/supervisor.h>
#include <hal/errors.h>
#include <hal/Utility/util.h>

#include <hcc/api_fat.h>
#include <stdio.h>
#include <freertos/task.h>


Boolean SolarPanelv2_Temperature2()
{
	int error;
	int panel;
	uint8_t status = 0;
	int32_t paneltemp = 0;
	float conv_temp;

	IsisSolarPanelv2_wakeup();

	printf("\t Temperature values \r\n");

	for( panel = 0; panel < ISIS_SOLAR_PANEL_COUNT; panel++ )
	{
		error = IsisSolarPanelv2_getTemperature(panel, &paneltemp, &status);
		if( error )
		{
			printf("\t\t Panel %d : Error (%d), Status (0x%X) \r\n", panel, error, status);
			continue;
		}

		conv_temp = (float)(paneltemp) * ISIS_SOLAR_PANEL_CONV;

		printf("\t\t Panel %d : %f [°C]\n", panel, conv_temp);
	}

	IsisSolarPanelv2_sleep();

	vTaskDelay( 1 / portTICK_RATE_MS );

	return TRUE;
}
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
	supervisor_housekeeping_t mySupervisor_housekeeping_hk;
	gom_eps_hk_t myEpsStatus_hk;
	printf("\n\r EPS: \n\r");
	printf("\t Volt battery [mV]: %d\r\n", (int)(myEpsStatus_hk.fields.vbatt));
	printf("\t Volt 5V [mV]: %d\r\n", (int)myEpsStatus_hk.fields.curout[2]); //curout[2] - 5V זה במילי אמפר לא צריך להיות במילי וולט
	printf("\t Volt 3.3V [mV]: %d\r\n", (int)myEpsStatus_hk.fields.curout[0]); //curout[0] - 3.3V
	//printf("\t Charging power [mV]: %d\r\n", (int)(myEpsStatus_hk.fields.curin[2]));
	printf("\t Consumed power [mA]: %d\r\n", (int)(myEpsStatus_hk.fields.cursys));
	printf("\t electric current [mA]: %d\r\n", (int)(myEpsStatus_hk.fields.curin[0]));
	printf("\t current 3.3V [mA]: %d\r\n", (int)(myEpsStatus_hk.fields.curin[1]));
	printf("\t current 5V [mA]: %d\r\n", (int)(myEpsStatus_hk.fields.curin[2]));
	printf("\t MCU Temperature [°C]: %d\r\n", (int)(myEpsStatus_hk.fields.temp[3]));
	printf("\t battery Temperature [°C]: %d\r\n", (int)(myEpsStatus_hk.fields.temp[4]));

	printf("\n\r Solar panel: \n\r");
	SolarPanelv2_Temperature2();

	printf("\n\r OBC: \n\r");
	printf("\t number of resets: %lu \r\n", mySupervisor_housekeeping_hk.fields.iobcResetCount);
	printf("\t satellite uptime: %lu \r\n", mySupervisor_housekeeping_hk.fields.iobcUptime);

	printf("\n\r SD: \n\r");
	printf("\t free memory [byte]: ");

	printf("\n\r ADC: \n\r");
	int i;
	for(i = 0; i < 10; i++)
		printf("\t ADC channel %d [mV]: %u", i, (unsigned int)mySupervisor_housekeeping_hk.fields.adcData[i]);
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
