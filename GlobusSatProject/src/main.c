/*
 * main.c
 *
 *  Created on: 5 6 2024
 *      Author: maayan
 */

#include "main.h"
#include "InitSystem.h"

#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/boolean.h>
#include <hal/Utility/util.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>

#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/exithandler.h>
#include <at91/commons.h>

#include <hcc/api_fat.h>

#include "GlobalStandards.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "InitSystem.h"
#include "main.h"
#include <stdlib.h>
#include "SubSystemModules/PowerManagment/EPSTest.h"
#ifdef TESTING
	#include "TestingDemos/MainTest.h"
#else

void taskMain()
{
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);

	InitSubsystems();
//	EpsThreshVolt_t ThresholdsIndex = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
//	SetEPSThreshold(&ThresholdsIndex);
//	printf("sucess part 1 \n  ");
//	GetEPSThreshold(&ThresholdsIndex);
//	printf("Vdown_cruise, %d, Vdown_operational, %d, Vup_cruise, %d, Vup_operational, %d",ThresholdsIndex.fields.Vdown_cruise ,ThresholdsIndex.fields.Vdown_operational ,ThresholdsIndex.fields.Vup_cruise ,ThresholdsIndex.fields.Vup_operational);

	while (TRUE) {
		Error_List errors = EPSTest();
		printf("batt %d FRAM alpha %d FRAM threshold %d", errors.fields.battError, errors.fields.alphaError, errors.fields.TresholdError);
		vTaskDelay(10);

	}
}
#endif

// main operation function. will be called upon software boot.
int main()
{
	xTaskHandle taskMainHandle;

	TRACE_CONFIGURE_ISP(DBGU_STANDARD, 2000000, BOARD_MCK);
	// Enable the Instruction cache of the ARM9 core. Keep the MMU and Data Cache disabled.
	CP15_Enable_I_Cache();

	// The actual watchdog is already started, this only initializes the watchdog-kick interface.
	WDT_start();

	// create the main operation task of the satellite
	#ifdef TESTING
		xTaskGenericCreate(taskTesting, (const signed char*) "taskTesting", 4096,
				NULL,
				configMAX_PRIORITIES - 2, &taskMainHandle, NULL, NULL);
	#else
		xTaskGenericCreate(taskMain, (const signed char*) "taskMain", 4096, NULL,
				configMAX_PRIORITIES - 2, &taskMainHandle, NULL, NULL);
	#endif



	vTaskStartScheduler();
	exit(0);
}
