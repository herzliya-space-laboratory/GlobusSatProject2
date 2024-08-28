/*
 * isis_OBC_demo.c
 *
 *  Created on: 20 12 2023
 *      Author: maayan
 */
#include "isis_OBC_demo.h"
#include  "common.h"

#include <satellite-subsystems/imepsv2_piu.h>

#include "satellite-subsystems/GomEPS.h"
#include "satellite-subsystems/IsisSolarPanelv2.h"

#include "hal/supervisor.h"
#include "hal/errors.h"
#include "hal/Utility/util.h"
#include "hal/Drivers/ADC.h"
#include "hal/Storage/FRAM.h"

#include <hcc/api_fat.h>
#include <stdio.h>
#include <freertos/task.h>

#include "GomEPSdemo.h"
#include "IsisSPv2demo.h"
#include "isis_eps_demo.h"


#define BEACON_INTERVAL_TIME_ADDR 		0x4590		//<! address of value of the delay between 2 beacons
#define BEACON_INTERVAL_TIME_SIZE 		4			//<! size of parameter in bytes

/*
 * Gets and prints the solar panels temperature.
 * */
static Boolean SolarPanelv2_Temperature2()
{
	int error;
	int panel;
	uint8_t status = 0;
	int32_t paneltemp = 0;
	float conv_temp;

	IsisSolarPanelv2_wakeup(); //Wakes the internal temperature sensor from sleep mode.

	printf("\t Temperature values \r\n");

	for( panel = 0; panel < ISIS_SOLAR_PANEL_COUNT; panel++ ) //Go for the count of solar panels we have.
	{
		error = IsisSolarPanelv2_getTemperature(panel, &paneltemp, &status); //Gets the temperature of each panel and the error message.
		if( error ) //if there is error
		{
			printf("\t\t Panel %d : Error (%d), Status (0x%X) \r\n", panel, error, status); //print what panel, which error and status
			continue;
		}

		conv_temp = (float)(paneltemp) * ISIS_SOLAR_PANEL_CONV;

		printf("\t\t Panel %d : %f [°C]\n", panel, conv_temp); //else prints
	}

	IsisSolarPanelv2_sleep(); //Puts the internal temperature sensor to sleep mode

	vTaskDelay( 1 / portTICK_RATE_MS ); //Delay the program

	return TRUE;
}

/*
 * Prints the beacon we supposed to send every 20 seconds.
 * */
static Boolean PrintBeacon(void)
{
	supervisor_housekeeping_t mySupervisor_housekeeping_hk; //create a variable that is the struct we need from supervisor
	int error = Supervisor_getHousekeeping(&mySupervisor_housekeeping_hk, SUPERVISOR_SPI_INDEX); //gets the variables to the struct and also check error.
	F_SPACE space; //same just to SD
	int ret = f_getfreespace(f_getdrive(), &space); //gets the variables to the struct
	//We need to decide before we run the program if we use Isis EPS or Gom EPS
#ifdef USE_EPS_ISIS //if isis, this define is in the file of isis_OBC_demo.h
	imepsv2_piu__gethousekeepingeng__from_t responseEPS; //Create a variable that is the struct we need from EPS_isis

	int error = imepsv2_piu__gethousekeepingeng(0,&responseEPS); //Get struct and get kind of error
	if( error ) //if something different then 0
		TRACE_ERROR("imepsv2_piu__gethousekeepingeng(...) return error (%d)!\n\r",error);
	else //prints the categories for the EPS isis
	{
		printf("\n\r EPS: \n\r");
		printf("\t Volt battery [mV]: %d\r\n", responseEPS.fields.batt_input.fields.volt);
		//todo:Volt 5V [mV]
		//todo:Volt 3.3V [mV]
		//todo:Charging power [mW]
		printf("\t Consumed power [mW]: %d\r\n", responseEPS.fields.dist_input.fields.power * 10);
		printf("\t Electric current [mA]: %d\r\n", responseEPS.fields.batt_input.fields.current);
		//todo: Current 3.3V [mA]
		//todo:Current 5V [mA]
		printf("\t MCU Temperature [°C]: %2f\r\n",((double)responseEPS.fields.temp) * 0.01);
		printf("\t Battery Temperature [°C]: %2f\r\n", ((double)responseEPS.fields.temp2) * 0.01))
	}

#else
	gom_eps_hk_t myEpsStatus_hk; //Create a variable that is the struct we need from EPS_gom


	print_error(GomEpsGetHkData_general(0, &myEpsStatus_hk)); //Get struct and prints error if there is.
	//prints the categories for the EPS Gom
	printf("\n\r EPS: \n\r");
	printf("\t Volt battery [mV]: %d\r\n", myEpsStatus_hk.fields.vbatt);
	//printf("\t Volt 5V [mV]: %d\r\n", (int)myEpsStatus_hk.fields.curout[2]); //curout[2] - 5V mA //not right
	//printf("\t Volt 3.3V [mV]: %d\r\n", (int)myEpsStatus_hk.fields.curout[0]); //curout[0] - 3.3V //not right
	//printf("\t Charging power [mV]: %d\r\n", (int)(myEpsStatus_hk.fields.curin[2]));
	printf("\t Consumed power [mA]: %d\r\n", (int)myEpsStatus_hk.fields.cursys);
	printf("\t Electric current [mA]: %d\r\n", (int)myEpsStatus_hk.fields.curin[0]);
	printf("\t Current 3.3V [mA]: %d\r\n", (int)myEpsStatus_hk.fields.curin[1]);
	printf("\t Current 5V [mA]: %d\r\n", (int)myEpsStatus_hk.fields.curin[2]);
	printf("\t MCU Temperature [°C]: %d\r\n", (int)myEpsStatus_hk.fields.temp[3]);
	printf("\t battery0 Temperature [°C]: %d\r\n", (int)myEpsStatus_hk.fields.temp[4]);
	printf("\t battery1 Temperature [°C]: %d\r\n", (int)myEpsStatus_hk.fields.temp[5]);
	printf("\t number of reboots to EPS: %d\r\n", (int)myEpsStatus_hk.fields.counter_boot);
#endif
	//For both of the EPS.
	printf("\n\r Solar panel: \n\r");
	SolarPanelv2_Temperature2(); // Gets the temperature of the solar panels and print it.

	printf("\n\r OBC: \n\r"); //Prints the categories of the OBC
	if(error)
		printf("ERROR: %d \n\r", error);
	else
	{
		printf("\t number of resets: %lu \r\n", mySupervisor_housekeeping_hk.fields.iobcResetCount);
		printf("\t satellite uptime [sec]: %lu \r\n", mySupervisor_housekeeping_hk.fields.iobcUptime / portTICK_RATE_MS);
	}
	printf("\n\r SD: \n\r");
	if(!ret) //If ret = 0 we prints the categories
	{
		printf("\t There are:\n\t %lu bytes total\n\t %lu bytes free\n\t %lu bytes used\n\t %lu bytes bad.\r\n",space.total, space.free, space.used, space.bad);
	}
	else //else we print the kind of error.
		printf("\t ERROR %d reading drive \r\n", ret);

	printf("\n\r ADC: \n\r"); //Get and prints the ADC channels
	unsigned short adcSamples[8];
	int i;
	int work = ADC_SingleShot(adcSamples); //Initialize the ADC driver
	if(!work) //If the Initialize worked
	{
		for(i = 0; i < 8; i++)
			ADC_ConvertRaw10bitToMillivolt(adcSamples[i]); //Gets the channels
		for(i = 0; i < 8; i++)
			printf("\t ADC channel %d: %d \r\n", i, adcSamples[i]); //Prints the channels
	}
	else
		printf("\t ERROR %d reading drive \r\n", work); //If the Initialize didn't worked it's print error

	return TRUE;
}

static Boolean SetBeaconPeriodPlaceToTwenty()
{
	const unsigned char data[] = "20";
	FRAM_write(data, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE);
	return TRUE;
}

/*
 * Write to the FRAM memory, read from it and check we have put it in the right place and put the right thing
 * */
static Boolean WriteAndReadFromFRAM(void){
	const unsigned char data[] = "hello"; //The data we write to the FRAM
	unsigned char writtenData[sizeof(data)]; //Make a same size array for the check after.
	unsigned int address = FRAM_getMaxAddress() - sizeof(data) - 10; //Get max address in the FRAM and subtracting from it 10 and the size of the data.
	print_error(FRAM_writeAndVerify(data, address, sizeof(data))); //Write data to the address and from there to the length of the data.
	print_error(FRAM_read(writtenData, address, sizeof(data))); //Read data from the address and from there to the length of the data and put it in writtenData.
	unsigned int i = 0;
	for(i = 0; i < sizeof(data); i++) //Check if all the chars equal to each other
	{
		if(writtenData[i] != data[i]) //if they not
		{
			printf("It doesn't write down what you wanted. \r\n");
			break;
		}
	}
	if(i > sizeof(data) - 1) //if they are
		printf("It write down what you wanted. \r\n");

	return TRUE;
}

/*
 * Asks the user which test he wants or if he wants to exit the test loop.
 * All the functions returns TRUE while the exit is FALSE.
 * @return type= Boolean; offerMoreTest that get to an infinite loop and the loop ends if the function return FALSE.
 * */
static Boolean selectAndExecuteOBCDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\rSelect a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Print beacon \n\r");
	printf("\t 2) Write and read from FRAM \n\r");
	printf("\t 3) Set beacon period place in FRAM to 20\n\r");
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 3) == 0); //You have to write a number between the two numbers include or else it ask you to enter a number between the two.

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = PrintBeacon();
		break;
	case 2:
		offerMoreTests = WriteAndReadFromFRAM();
		break;
	case 3:
		offerMoreTests = SetBeaconPeriodPlaceToTwenty();
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
		offerMoreTests = selectAndExecuteOBCDemoTest();	// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}
/*
 * Initialize the supervisor by the function from the FRAM.h
 * */
Boolean InitFRAM(void)
{
	int error = FRAM_start();
	if(error != E_NO_SS_ERR)
	{
		print_error(error);
		return FALSE;
	}
	return TRUE;
}
/*
 * Initialize the supervisor by the function from the supervisor.h
 * */
Boolean InitSupervisor(void)
{
	uint8_t po = SUPERVISOR_SPI_INDEX;
	int error = Supervisor_start(&po, 0);
	if(error != E_NO_SS_ERR)
	{
		print_error(error);
		return FALSE;
	}
	return TRUE;
}

Boolean InitSDFat(void)
{
	// in FS init we don't want to use a log file !
	// Initialize the memory for the FS
	int err = hcc_mem_init();
	if (err != E_NO_SS_ERR){
		return FALSE;
	}
	// Initialize the FS
	err = fs_init();
	if (err != E_NO_SS_ERR){
		return FALSE;
	}

	fs_start();

	// Tell the OS (freeRTOS) about our FS
	err = f_enterFS();
	if (err != E_NO_SS_ERR){
		return FALSE;
	}

	// Initialize the volume of SD card 0 (A)
	err = f_initvolume( 0, atmel_mcipdc_initfunc, 0 );
	if (err != E_NO_SS_ERR){
	// erro init SD 0 so de-itnit and init SD 1
	//printf("f_initvolume primary error:%d\n",err);
	hcc_mem_init();
		fs_init();
		f_enterFS();
		err = f_initvolume( 0, atmel_mcipdc_initfunc, 1 );
		if (err != E_NO_SS_ERR){
				//printf("f_initvolume secondary error:%d\n",err);
		}
	}

	return TRUE;
}
Boolean InitOBCtests(void)
{
#ifdef USE_EPS_ISIS
	if(!isis_eps__demo__init() || !InitSDFat() || !InitSupervisor() || !InitFRAM())
	{
		return FALSE;
	}
#else
	if(!GomEPSdemoInit() || !InitSDFat() || !InitSupervisor() || !InitFRAM())
	{
		return FALSE;
	}
#endif
	/*if(!InitSolarPanels())
	{
		return FALSE;
	}*/
	return TRUE;
}

Boolean OBCtest(void)
{
	if (InitOBCtests())
	{
		IsisOBCdemoLoop();
	}

	//IsisOBCdemoMain();
	return TRUE;
}
