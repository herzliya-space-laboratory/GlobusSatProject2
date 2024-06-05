/*
 * main.c
 *
 *  Created on: 5 áéåð 2024
 *      Author: maaya
 */

#include "main.h"
#include "InitSystem.h"

#include <stdio.h>

#include <satellite-subsystems/version/version.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/peripherals/pio/pio_it.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/LED.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/boolean.h>
#include <hal/version/version.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define ENABLE_MAIN_TRACES 1
#if ENABLE_MAIN_TRACES
	#define MAIN_TRACE_INFO			TRACE_INFO
	#define MAIN_TRACE_DEBUG		TRACE_DEBUG
	#define MAIN_TRACE_WARNING		TRACE_WARNING
	#define MAIN_TRACE_ERROR		TRACE_ERROR
	#define MAIN_TRACE_FATAL		TRACE_FATAL
#else
	#define MAIN_TRACE_INFO(...)	{ }
	#define MAIN_TRACE_DEBUG(...)	{ }
	#define MAIN_TRACE_WARNING(...)	{ }
	#define MAIN_TRACE_ERROR		TRACE_ERROR
	#define MAIN_TRACE_FATAL		TRACE_FATAL
#endif

/*
 * Asks the user which test he wants.
 * all the functions returns TRUE.
 * @return type= Boolean; offerMoreTest that get to an infinite loop and the loop ends if the function return FALSE.
 * */
Boolean selectAndExecuteTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select the device to be tested to perform: \n\r");
	printf("\t 1) Start FRAM \n\r");
	printf("\t 2) Start I2C \n\r");
	printf("\t 3) Start SPI \n\r");
	printf("\t 4) Init systems \n\r");
	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 6) == 0); //you have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection)
	{
		case 1:
			FRAM_start();
			offerMoreTests = TRUE;
			break;
		case 2:
			I2C_start();
			offerMoreTests = TRUE;
			break;
		case 3:
			SPI_start();
			offerMoreTests = TRUE;
			break;
		case 4:
			InitSubsystems();
			offerMoreTests = TRUE;
			break;
		default:
			break;
	}

	return offerMoreTests;
}

void taskMain()
{
	Boolean offerMoreTests = FALSE;

	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);

	while(1)
	{
		LED_toggle(led_1);

		offerMoreTests = selectAndExecuteTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}

	while(1) {
		LED_toggle(led_1);
		vTaskDelay(500);
	}

}

int main()
{
	unsigned int i;
	xTaskHandle taskMainHandle;

	TRACE_CONFIGURE_ISP(DBGU_STANDARD, 115200, BOARD_MCK);
	// Enable the Instruction cache of the ARM9 core. Keep the MMU and Data Cache disabled.
	CP15_Enable_I_Cache();

	LED_start();

	// The actual watchdog is already started, this only initializes the watchdog-kick interface.
	WDT_start();

	PIO_InitializeInterrupts(AT91C_AIC_PRIOR_LOWEST+4);

	printf("\n\nDemo applications for ISIS OBC Satellite Subsystems Library built on %s at %s\n", __DATE__, __TIME__);
	printf("\nDemo applications use:\n");
	printf("* Sat Subsys lib version %s.%s.%s built on %s at %s\n",
			SatelliteSubsystemsVersionMajor, SatelliteSubsystemsVersionMinor, SatelliteSubsystemsVersionRevision,
			SatelliteSubsystemsCompileDate, SatelliteSubsystemsCompileTime);
	printf("* HAL lib version %s.%s.%s built on %s at %s\n", HalVersionMajor, HalVersionMinor, HalVersionRevision,
			HalCompileDate, HalCompileTime);

	LED_wave(1);
	LED_waveReverse(1);
	LED_wave(1);
	LED_waveReverse(1);

	MAIN_TRACE_DEBUG("\t main: Starting main task.. \n\r");
	xTaskGenericCreate(taskMain, (const signed char*)"taskMain", 4096, NULL, configMAX_PRIORITIES-2, &taskMainHandle, NULL, NULL);

	MAIN_TRACE_DEBUG("\t main: Starting scheduler.. \n\r");
	vTaskStartScheduler();

	// This part should never be reached.
	MAIN_TRACE_DEBUG("\t main: Unexpected end of scheduling \n\r");

	//Flash some LEDs for about 100 seconds
	for (i=0; i < 2500; i++)
	{
		LED_wave(1);
		MAIN_TRACE_DEBUG("MAIN: STILL ALIVE %d\n\r", i);
	}
	exit(0);
}

