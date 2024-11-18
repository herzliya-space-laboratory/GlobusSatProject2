/*
 * isis_ants_demo.c
 *
 *  Created on: Nov 2023
 *      Author: obar
 */

#include "isis_ants_demo.h"
#include "common.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_ants.h>
#include <satellite-subsystems/isis_ants_types.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

////General Variables
#define	AUTO_DEPLOYMENT_TIME	10
#define MANUAL_DEPLOYMENT_TIME  10

#define ANTS_PRIMARY	0
#define ANTS_SECONDARY 	1

//Helper Functions
static void printDeploymentStatus(unsigned char antenna_id, unsigned char status)
{
	printf("Antenna %d: ", antenna_id);
	if(status == 0)
	{
		printf("deployed\n\r");
	}
	else
	{
		printf("undeployed\n\r");
	}
}

static void printTimeoutStatus(unsigned char antenna_id, unsigned char status)
{
	printf("Antenna %d: ", antenna_id);
	if(status == 0)
	{
		printf("no timeout\n\r");
	}
	else
	{
		printf("timeout\n\r");
	}
}

static void printBurnStatus(unsigned char antenna_id, unsigned char status)
{
	printf("Antenna %d: ", antenna_id);
	if(status == 0)
	{
		printf("inactive\n\r");
	}
	else
	{
		printf("active\n\r");
	}
}

// Function calls to reset both sides of the AntS
static Boolean reset_ants_test(void)
{
	printf("\r\n Resetting Primary \r\n");
	print_error(isis_ants__reset(ANTS_PRIMARY));

	vTaskDelay(5 / portTICK_RATE_MS);

	printf("\r\n Resetting Secondary \r\n");
	print_error(isis_ants__reset(ANTS_SECONDARY));

	return TRUE;
}

// Function calls to get the current status of both sides of the AntS
static void get_status_ants_test(unsigned char activeSide)
{
	isis_ants__get_status__from_t currentStatus;

	print_error(isis_ants__get_status(activeSide, &currentStatus));

	printf("\r\nAntS\r\n");

	printf("Current deployment status 0x%x 0x%x (raw value) \r\n", currentStatus.raw[0], currentStatus.raw[1]);
	printf("Arm status: %s \r\n", currentStatus.fields.arm_state==0?"disarmed":"armed");
	printDeploymentStatus(1, currentStatus.fields.antenna1_switch_state);
	printBurnStatus(1, currentStatus.fields.antenna1_burn_state);
	printTimeoutStatus(1, currentStatus.fields.antenna1_timeout);

	printDeploymentStatus(2, currentStatus.fields.antenna2_switch_state);
	printBurnStatus(2, currentStatus.fields.antenna2_burn_state);
	printTimeoutStatus(2, currentStatus.fields.antenna2_timeout);

	printDeploymentStatus(3, currentStatus.fields.antenna3_switch_state);
	printBurnStatus(3, currentStatus.fields.antenna3_burn_state);
	printTimeoutStatus(3, currentStatus.fields.antenna3_timeout);

	printDeploymentStatus(4, currentStatus.fields.antenna4_switch_state);
	printBurnStatus(4, currentStatus.fields.antenna4_burn_state);
	printTimeoutStatus(4, currentStatus.fields.antenna4_timeout);
}

// Function calls to get the current temperature on both sides of the AntS
static Boolean temp_ants_test(void)
{
	isis_ants__get_temperature__from_t currTemp;
	float eng_value = 0;

	print_error(isis_ants__get_temperature(ANTS_PRIMARY, &currTemp));

	if(currTemp.fields.update_flag == 255)
	{
		eng_value = ((float)currTemp.fields.temperature * -0.2922) + 190.65;
		printf("\r\n AntS Primary Updated temperature %f deg. C\r\n", eng_value);
	}
	else
	{
		eng_value = ((float)currTemp.fields.temperature * -0.2922) + 190.65;
		printf("\r\n AntS Primary Not Updated temperature %f deg. C\r\n", eng_value);
	}

	print_error(isis_ants__get_temperature(ANTS_SECONDARY, &currTemp));

	if(currTemp.fields.update_flag == 255)
	{
		eng_value = ((float)currTemp.fields.temperature * -0.2922) + 190.65;
		printf("\r\n AntS Secondary Updated temperature %f deg. C\r\n", eng_value);
	}
	else
	{
		eng_value = ((float)currTemp.fields.temperature * -0.2922) + 190.65;
		printf("\r\n AntS Secondary Not Updated temperature %f deg. C\r\n", eng_value);
	}

	return TRUE;
}

// Function calls to get the current uptime on both sides of the AntS
static Boolean uptime_ants_test(void)
{
	uint32_t uptime = 0;

	print_error(isis_ants__get_uptime(ANTS_PRIMARY, &uptime));
	printf("\r\n AntS Primary uptime %u sec. \r\n", (unsigned int)uptime);

	vTaskDelay(5 / portTICK_RATE_MS);

	print_error(isis_ants__get_uptime(ANTS_SECONDARY, &uptime));
	printf("\r\n AntS Secondary uptime %u sec. \r\n", (unsigned int)uptime);

	return TRUE;
}

// Function calls to get a block of telemetry on both sides of the AntS
static Boolean telem_ants_test(unsigned char activeSide)
{
	isis_ants__get_all_telemetry__from_t allTelem;
	float eng_value = 0;

	printf("\r\nAntS\r\n");

	print_error(isis_ants__get_all_telemetry(activeSide, &allTelem));

	if(allTelem.fields.update_flag == 255)
	{
		eng_value = ((float)allTelem.fields.temperature * -0.2922) + 190.65;
		printf("\r\n AntS Updated temperature %f deg. C\r\n", eng_value);
	}
	else
	{
		eng_value = ((float)allTelem.fields.temperature * -0.2922) + 190.65;
		printf("\r\n AntS Not Updated temperature %f deg. C\r\n", eng_value);
	}

	printf("Current deployment status 0x%x 0x%x (raw value) \r\n", allTelem.raw[1], allTelem.raw[2]);
	printf("Arm status: %s \r\n", allTelem.fields.arm_state==0?"disarmed":"armed");
	printDeploymentStatus(1, allTelem.fields.antenna1_switch_state);
	printBurnStatus(1, allTelem.fields.antenna1_burn_state);
	printTimeoutStatus(1, allTelem.fields.antenna1_timeout);

	printDeploymentStatus(2, allTelem.fields.antenna2_switch_state);
	printBurnStatus(2, allTelem.fields.antenna2_burn_state);
	printTimeoutStatus(2, allTelem.fields.antenna2_timeout);

	printDeploymentStatus(3, allTelem.fields.antenna3_switch_state);
	printBurnStatus(3, allTelem.fields.antenna3_burn_state);
	printTimeoutStatus(3, allTelem.fields.antenna3_timeout);

	printDeploymentStatus(4, allTelem.fields.antenna4_switch_state);
	printBurnStatus(4, allTelem.fields.antenna4_burn_state);
	printTimeoutStatus(4, allTelem.fields.antenna4_timeout);

	printf("\r\n AntS uptime %u sec. \r\n", (unsigned int)allTelem.fields.uptime);

	return TRUE;
}

// Function call to arm a side of the ANTS
static Boolean set_arm_status(unsigned char activeSide, Boolean arm)
{
    printf( "DISARMING antenna system \n\r");

	print_error(isis_ants__disarm(activeSide));

	vTaskDelay(5 / portTICK_RATE_MS);

	if(arm)
	{
		int command = 0;

	    printf( "ARM antenna system (1=yes, 0=abort): \n\r");

	    while(UTIL_DbguGetIntegerMinMax(&command, 0, 1) == 0);

	    if(command == 1)
	    {
	    	int stat;

		    stat = isis_ants__arm(activeSide);

		    print_error(stat);

		    if(stat == E_NO_SS_ERR)
		    {
		    	printf( "antenna system successfully ARMED \n\r===>>> auto/manual deploy will deploy antennas when commanded <<<===\n\r");
		    }
		    else
		    {
		    	printf( "antenna system arming failed \n\r");
		    }

			vTaskDelay(5 / portTICK_RATE_MS);
	    }
	    else
	    {
		    printf( "Aborted ARMING antenna system \n\r");
	    }
	}

	return TRUE;
}

static Boolean autodeployment_ants_test(unsigned char activeSide)
{
	driver_error_t rv;

	printf("Auto deployment ...\n\r");

	{	// check ARM status; if not ARMed no actual deployment will result
		isis_ants__get_status__from_t status;
		rv = isis_ants__get_status(activeSide, &status);

		if(rv)
		{
			printf( "ERROR: Getting status failed! rv=%d. Arm status unknown. Continuing ... \n\r", rv);
		}
		else
		{
			if(status.fields.arm_state)
			{
				printf( "Arm status: ARMED. Deployment will result. \n\r");
			}
			else
			{
				printf( "Arm status: DISARMED. No deployment will result. ARM first if deployment is desired. \n\r");
			}
		}
	}

	rv = isis_ants__start_auto_deploy(activeSide, AUTO_DEPLOYMENT_TIME);
	if(rv)
	{
		printf( "ERROR: IsisAntS_autoDeployment command failed! rv=%d \n\r", rv);
	}
	else
	{
		printf( "Auto-deployment command successfully issued. \n\r");
	}

	return TRUE;
}


static Boolean manual_deployment_ants_test(unsigned char activeSide)
{
    int antennaSelection = 0;
    int rv;

	printf("Manual deployment ...\n\r");

	{	// check ARM status; if not ARMed no actual deployment will result
		isis_ants__get_status__from_t status;
		rv = isis_ants__get_status(activeSide, &status);

		if(rv)
		{
			printf( "ERROR: Getting status failed! rv=%d. Arm status unknown. Continuing ... \n\r", rv);
		}
		else
		{
			if(status.fields.arm_state)
			{
				printf( "Arm status: ARMED. Deployment will result. \n\r");
			}
			else
			{
				printf( "Arm status: DISARMED. No deployment will result. ARM first if deployment is desired. \n\r");
			}
		}
	}

    printf( "Select antenna to deploy (1, 2, 3, 4 or 5 to abort): \n\r");
    while(UTIL_DbguGetIntegerMinMax(&antennaSelection, 1, 5) == 0);

    switch(antennaSelection)
    {
    	case 1:
    		print_error(isis_ants__deploy1(activeSide, MANUAL_DEPLOYMENT_TIME));
    		break;
		case 2:
			print_error(isis_ants__deploy2(activeSide, MANUAL_DEPLOYMENT_TIME));
			break;
		case 3:
			print_error(isis_ants__deploy3(activeSide, MANUAL_DEPLOYMENT_TIME));
			break;
		case 4:
			print_error(isis_ants__deploy4(activeSide, MANUAL_DEPLOYMENT_TIME));
			break;
		case 5:
			return TRUE;
			break;
    }

	printf( "Waiting %ds. for deployment of antenna %d\n\r...", MANUAL_DEPLOYMENT_TIME, antennaSelection);
	vTaskDelay(MANUAL_DEPLOYMENT_TIME*1000 / portTICK_RATE_MS);
    
    return TRUE;
}

Boolean selectAndExecuteAntSDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\r Select a test to perform: \n\r");
	printf("\t 1) AntS reset - both sides \n\r");
	printf("\t 2) Ants status - both sides \n\r");
	printf("\t 3) AntS uptime - both sides \n\r");
	printf("\t 4) AntS temperature - both sides \n\r");
	printf("\t 5) AntS telemetry - Primary \n\r");
	printf("\t 6) AntS telemetry - Secondary \n\r");
	printf("\t 7) AntS ARM deployment - Primary \n\r");
	printf("\t 8) AntS ARM deployment - Secondary \n\r");
	printf("\t 9) AntS DISARM deployment - both sides \n\r");
	printf("\t 10) AntS autodeployment - Primary\n\r");
	printf("\t 11) AntS autodeployment - Secondary\n\r");
    printf("\t 12) AntS manual deployment - Primary\n\r");
    printf("\t 13) AntS manual deployment - Secondary\n\r");
	printf("\t 14) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 14) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = reset_ants_test();
		break;
	case 2:
		get_status_ants_test(ANTS_PRIMARY);
		vTaskDelay(5 / portTICK_RATE_MS);
		get_status_ants_test(ANTS_SECONDARY);
		break;
	case 3:
		offerMoreTests = uptime_ants_test();
		break;
	case 4:
		offerMoreTests = temp_ants_test();
		break;
	case 5:
		offerMoreTests = telem_ants_test(ANTS_PRIMARY);
		break;
	case 6:
		offerMoreTests = telem_ants_test(ANTS_SECONDARY);
		break;
	case 7:
		offerMoreTests = set_arm_status(ANTS_PRIMARY, TRUE);
		break;
	case 8:
		offerMoreTests = set_arm_status(ANTS_SECONDARY, TRUE);
		break;
	case 9:
		set_arm_status(ANTS_PRIMARY, FALSE);
		vTaskDelay(5 / portTICK_RATE_MS);
		set_arm_status(ANTS_SECONDARY, FALSE);
		break;
	case 10:
		offerMoreTests = autodeployment_ants_test(ANTS_PRIMARY);
		break;
	case 11:
		offerMoreTests = autodeployment_ants_test(ANTS_SECONDARY);
		break;
    case 12:
        offerMoreTests = manual_deployment_ants_test(ANTS_PRIMARY);
        break;
    case 13:
        offerMoreTests = manual_deployment_ants_test(ANTS_SECONDARY);
        break;
	case 14:
		offerMoreTests = FALSE;
		break;

	default:
		break;
	}

	return offerMoreTests;
}

static void initmain(void)
{
    int retValInt = 0;

    ISIS_ANTS_t myAntennaAddress[2];
    //Primary
	myAntennaAddress[0].i2cAddr = 0x31;
	//Secondary
	myAntennaAddress[1].i2cAddr = 0x32;

	//Initialize the I2C
	retValInt = I2C_start(200000, 10);

	if(retValInt == driver_error_reinit)
	{
		printf("\r\nISIS_AntS a subsystem has already been initialised.\n\r");
	}
	else if(retValInt != 0)
	{
		TRACE_FATAL("\n\r I2Ctest: I2C_start_Master for Ants test: %d! \n\r", retValInt);
	}

	//Initialize the AntS system
	print_error(ISIS_ANTS_Init(myAntennaAddress, 2));
}

static void Ants_mainDemo(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteAntSDemoTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
}


Boolean AntStest(void)
{
	initmain();
	Ants_mainDemo();

	return TRUE;
}

