/*
 * isis_ants2_demo.c
 *
 *  Created on: Jul 4, 2012
 *      Author: marcoalfer
 *  Updated: Oct 2023
 *  	Author: OBAR
 */

#include "isis_ants2_demo.h"
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

#include <satellite-subsystems/isis_ants_rev2.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

////General Variables
#define	AUTO_DEPLOYMENT_TIME	10
#define MANUAL_DEPLOYMENT_TIME  10


static char *updatedFlagToString(const uint8_t updated_flag)
{
	switch(updated_flag){
	case 0:   return "Updated: no";
	case 255: return "Updated: yes";
	default:  return "Updated: corrupt value";
	}
}

// Function calls to reset both sides of the AntS
static Boolean resetAntSTest(unsigned char index)
{
	unsigned char antennaSystemsIndex = index;

	printf("\r\n Resetting \r\n");
	print_error(isis_ants_rev2__reset(antennaSystemsIndex));

	return TRUE;
}

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

// Function calls to get the current status of both sides of the AntS
static void getStatusAntSTest(unsigned char index)
{
    isis_ants_rev2__deploymenttelemetry_t currentStatus;

	print_error(isis_ants_rev2__get_status(index, &currentStatus));

	printf("\r\nAntS current deployment status 0x%x 0x%x (raw value) \r\n", currentStatus.raw[0], currentStatus.raw[1]);
	printf("Arm status: %s \r\n", currentStatus.fields.arm_state==0?"disarmed":"armed");
	printDeploymentStatus(1, currentStatus.fields.antenna1_undeployed);
	printDeploymentStatus(2, currentStatus.fields.antenna2_undeployed);
	printf("Override: %s \r\n", currentStatus.fields.switch_ignore_state==0?"inactive":"active");
}

// Function calls to get the current temperature on both sides of the AntS
static Boolean tempAntSTest(unsigned char index)
{
	unsigned char antennaSystemsIndex = index;
	isis_ants_rev2__get_temperature__from_t currTemp;
	float eng_value = 0;

	print_error(isis_ants_rev2__get_temperature(antennaSystemsIndex, &currTemp));
	eng_value = ((float)currTemp.fields.temperature * -0.2922) + 190.65;
	printf("\r\n AntS temperature: %f deg. C (%s)\r\n", eng_value, updatedFlagToString(currTemp.fields.update_flag));

	return TRUE;
}

// Function calls to get the current uptime on both sides of the AntS
static Boolean uptimeAntSTest(unsigned char index)
{
	unsigned char antennaSystemsIndex = index;
	uint32_t uptime = 0;

	print_error(isis_ants_rev2__get_uptime(antennaSystemsIndex, &uptime));
	printf("\r\n AntS uptime: %d sec. \r\n", (unsigned int)uptime);

	return TRUE;
}

// Function calls to get a block of telemetry on both sides of the AntS
static Boolean telemAntSTest(unsigned char index)
{
	unsigned char antennaSystemsIndex = index;
	isis_ants_rev2__get_all_telemetry__from_t allTelem;
	float eng_value = 0;

	printf("\r\nAntS\n\r");

	print_error(isis_ants_rev2__get_all_telemetry(antennaSystemsIndex, &allTelem));

	printf("Current deployment status 0x%x 0x%x (raw value) \r\n", allTelem.fields.deployment_status.raw[0], allTelem.fields.deployment_status.raw[1]);
	printDeploymentStatus(1, allTelem.fields.deployment_status.fields.antenna1_undeployed);
	printDeploymentStatus(2, allTelem.fields.deployment_status.fields.antenna2_undeployed);

	eng_value = ((float)allTelem.fields.temperature * -0.2922) + 190.65;
	printf("\r\n AntS temperature: %f deg. C\r\n", eng_value);
	eng_value = ((float)allTelem.fields.voltage * 0.01965444793);
	printf("\r\n AntS voltage: %.2f V\r\n", eng_value);
	printf("\r\n AntS uptime: %d sec. \r\n", (unsigned int)allTelem.fields.uptime);
	printf("\r\n %s \r\n", updatedFlagToString(allTelem.fields.update_flag));

	return TRUE;
}

static Boolean setARMStatus(unsigned char index, Boolean arm)
{
    printf( "DISARMING antenna system\n\r");

    print_error(isis_ants_rev2__disarm(index));

	vTaskDelay(5 / portTICK_RATE_MS);

	if(arm)
	{
		int command = 0;

	    printf( "ARM antenna system? (1=yes, 0=abort): \n\r");

	    while(UTIL_DbguGetIntegerMinMax(&command, 0, 1) == 0);

	    if(command == 1)
	    {
	    	int stat;

		    stat = isis_ants_rev2__arm(index);

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
		    printf( "Aborted ARMING antenna system\n\r");
	    }
	}

	return TRUE;
}

static Boolean autoDeploymentAntSTest(unsigned char index, Boolean redundant)
{
	unsigned char antennaSystemsIndex = index;
	int rv;

	if (redundant)
	{
	    printf("Auto main deployment...\n\r");
	}
	else
	{
	    printf("Auto redundant deployment...\n\r");
	}

	{	// check ARM status; if not ARMed no actual deployment will result
	    isis_ants_rev2__deploymenttelemetry_t status;
		rv = isis_ants_rev2__get_status(index, &status);

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

	if (redundant)
	{
	    rv = isis_ants_rev2__start_auto_deploy(antennaSystemsIndex, AUTO_DEPLOYMENT_TIME);
	}
	else
	{
	    rv = isis_ants_rev2__start_auto_deploy_redundant(antennaSystemsIndex, AUTO_DEPLOYMENT_TIME);
	}
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


static Boolean manualDeploymentAntSTest(unsigned char index, Boolean redundant)
{
    int antennaSelection = 0;
    unsigned char antennaSystemsIndex = index;
    int rv;

	if (redundant)
	{
	    printf("Manual redudant deployment ...\n\r");
	}
	else
	{
	    printf("Manual main deployment ...\n\r");
	}

	{	// check ARM status; if not ARMed no actual deployment will result
	    isis_ants_rev2__deploymenttelemetry_t status;
		rv = isis_ants_rev2__get_status(index, &status);

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

    printf( "Select antenna to deploy (1, 2 or 3 to abort): \n\r");
    while(UTIL_DbguGetIntegerMinMax(&antennaSelection, 1, 3) == 0);

    if (antennaSelection <= 2)
    {
        if (redundant)
        {
            if (antennaSelection == 1)
            {
                print_error(isis_ants_rev2__deploy1_redundant(antennaSystemsIndex, MANUAL_DEPLOYMENT_TIME));
            }
            else
            {
                print_error(isis_ants_rev2__deploy2_redundant(antennaSystemsIndex, MANUAL_DEPLOYMENT_TIME));
            }
        }
        else
        {
            if (antennaSelection == 1)
            {
                print_error(isis_ants_rev2__deploy1(antennaSystemsIndex, MANUAL_DEPLOYMENT_TIME));
            }
            else
            {
                print_error(isis_ants_rev2__deploy2(antennaSystemsIndex, MANUAL_DEPLOYMENT_TIME));
            }
        }
    }

	printf( "Waiting %ds. for deployment of antenna %d\n\r...", MANUAL_DEPLOYMENT_TIME, antennaSelection);
	vTaskDelay(MANUAL_DEPLOYMENT_TIME*1000 / portTICK_RATE_MS);
    
    return TRUE;
}

static Boolean selectAndExecuteAntS2DemoTest(unsigned char index)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\r Select a test to perform: \n\r");
	printf("\t 1) AntS reset\n\r");
	printf("\t 2) Ants status\n\r");
	printf("\t 3) AntS uptime\n\r");
	printf("\t 4) AntS temperature\n\r");
	printf("\t 5) AntS telemetry\n\r");
	printf("\t 6) AntS ARM deployment\n\r");
	printf("\t 7) AntS DISARM deployment\n\r");
	printf("\t 8) AntS autodeployment - main\n\r");
	printf("\t 9) AntS autodeployment - redundant\n\r");
    printf("\t 10) AntS manual deployment - main\n\r");
    printf("\t 11) AntS manual deployment - redundant\n\r");
	printf("\t 12) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 12) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = resetAntSTest(index);
		break;
	case 2:
		getStatusAntSTest(index);
		break;
	case 3:
		offerMoreTests = uptimeAntSTest(index);
		break;
	case 4:
		offerMoreTests = tempAntSTest(index);
		break;
	case 5:
		offerMoreTests = telemAntSTest(index);
		break;
	case 6:
		offerMoreTests = setARMStatus(index, TRUE);
		break;
	case 7:
		setARMStatus(index, FALSE);
		break;
	case 8:
		offerMoreTests = autoDeploymentAntSTest(index, FALSE);
		break;
	case 9:
		offerMoreTests = autoDeploymentAntSTest(index, TRUE);
		break;
    case 10:
        offerMoreTests = manualDeploymentAntSTest(index, FALSE);
        break;
    case 11:
        offerMoreTests = manualDeploymentAntSTest(index, TRUE);
        break;
	case 12:
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

	ISIS_ANTS_REV2_t myAntennaAddress;
	myAntennaAddress.i2cAddr = 0x31;

	//Initialize the I2C
	retValInt = I2C_start(200000, 10);

	if(retValInt != 0)
	{
		TRACE_FATAL("\n\r I2Ctest: I2C_start_Master for Ants test: %d! \n\r", retValInt);
	}

	//Initialize the AntS system
	print_error(ISIS_ANTS_REV2_Init(&myAntennaAddress, 1));
}

static void Ants2_mainDemo(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteAntS2DemoTest(0);

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
}


Boolean AntS2test(void)
{
	initmain();
	Ants2_mainDemo();

	return TRUE;
}

