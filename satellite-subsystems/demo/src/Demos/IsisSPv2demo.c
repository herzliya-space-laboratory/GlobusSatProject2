#include "IsisSPdemo.h"

#include <satellite-subsystems/IsisSolarPanelv2.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/SPI.h>
#include <hal/Utility/util.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <stdint.h>

/*
 * gets and prints the solar panels temperature.
 * */
Boolean SolarPanelv2_Temperature()
{
	int error;
	int panel;
	uint8_t status = 0;
	int32_t paneltemp = 0;
	float conv_temp;

	IsisSolarPanelv2_wakeup(); //Wakes the internal temperature sensor from sleep mode.

	printf("\r\n Temperature values \r\n");

	for( panel = 0; panel < ISIS_SOLAR_PANEL_COUNT; panel++ )  //go for the count of solar panels we have.
	{
		error = IsisSolarPanelv2_getTemperature(panel, &paneltemp, &status);  //gets the temperature of each panel and the error message.
		if( error ) //if there is an error
		{
			printf("Panel %d : Error (%d), Status (0x%X) \r\n", panel, error, status); //print what panel, which error and status
			continue;
		}

		conv_temp = (float)(paneltemp) * ISIS_SOLAR_PANEL_CONV;

		printf("Panel %d : %f \n", panel, conv_temp); //else prints
	}

	IsisSolarPanelv2_sleep(); //Puts the internal temperature sensor to sleep mode

	vTaskDelay( 1 / portTICK_RATE_MS ); //Delay the program

	return TRUE;
}
/**
 * get which states the polar panels in (awake, sleep, no_init)
 * */
Boolean SolarPanelv2_State() {
	IsisSolarPanelv2_State_t state;

	IsisSolarPanelv2_wakeup();

	state = IsisSolarPanelv2_getState(); //gets the states
	switch (state) { //print the states
	case ISIS_SOLAR_PANEL_STATE_NOINIT:
		printf("the current state is: NOINIT \r\n");
		break;
	case ISIS_SOLAR_PANEL_STATE_SLEEP:
		printf("the current state is: SLEEP \r\n");
		break;
	case ISIS_SOLAR_PANEL_STATE_AWAKE:
		printf("the current state is: AWAKE \r\n");
		break;

	default:
		break;
	}
	IsisSolarPanelv2_sleep();

	return TRUE;
}

/*
 * Asks the user which test he wants or if he wants to exit the test loop.
 * all the functions returns TRUE while the exit is FALSE.
 * @return type= Boolean; offerMoreTest that get to an infinite loop and the loop ends if the function return FALSE.
 * */
Boolean selectAndExecuteSolarPanelsv2DemoTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Solar internal temperature sensor State \n\r");
	printf("\t 2) Get solar panels states \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 2) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection)
	{
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = SolarPanelv2_Temperature();
		break;
	case 2:
		offerMoreTests = SolarPanelv2_State();
		break;
	default:
		break;
	}

	return offerMoreTests;
}

void SolarPanelsv2_mainDemo()
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteSolarPanelsv2DemoTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
}


Boolean SolarPanelv2test()
{
	int retValInt = 0;

	retValInt = IsisSolarPanelv2_initialize(slave0_spi);
	if(retValInt != 0)
	{
		TRACE_WARNING("\n\r IsisSolarPaneltest: IsisSolarPanelv2_initialize returned %d! \n\r", retValInt);
	}

	retValInt = IsisSolarPanelv2_sleep();
	if(retValInt != 0)
	{
		TRACE_WARNING("\n\r IsisSolarPaneltest: IsisSolarPanelv2_sleep returned %d! \n\r", retValInt);
	}

	SolarPanelsv2_mainDemo();

	return TRUE;
}

Boolean InitSolarPanels(void){
	if(IsisSolarPanelv2_initialize(slave0_spi)!=0)
	{
		return FALSE;
	}
	if(IsisSolarPanelv2_sleep()!=0)
	{
		return FALSE;
	}
	return TRUE;
}
