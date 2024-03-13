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

static Boolean SolarPanelv2_Temperature2()
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
	F_SPACE space;
	int ret = f_getfreespace(f_getdrive(), &space);
	print_error(Supervisor_getHousekeeping(&mySupervisor_housekeeping_hk, 0));

#ifdef USE_EPS_ISIS
	imepsv2_piu__gethousekeepingeng__from_t response;

	int error = imepsv2_piu__gethousekeepingeng(0,&response);
	if( error )
		TRACE_ERROR("imepsv2_piu__gethousekeepingeng(...) return error (%d)!\n\r",error);
	else
	{
		printf("\n\r EPS: \n\r");
		printf("\t Volt battery [mV]: %d\r\n", response.fields.batt_input.fields.volt);

		printf("Consumed power [mW]: %d\r\n", response.fields.dist_input.fields.power * 10);
//need finish
		printf("\t MCU Temperature [°C]: %2f\r\n",((double)response.fields.temp) * 0.01);
		printf("\t Battery Temperature [°C]: %2f\r\n", ((double)response.fields.temp2) * 0.01))
	}

#else
	gom_eps_hk_t myEpsStatus_hk;


	print_error(GomEpsGetHkData_general(0, &myEpsStatus_hk));
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
	printf("\n\r Solar panel: \n\r");
	SolarPanelv2_Temperature2();

	printf("\n\r OBC: \n\r");
	printf("\t number of resets: %lu \r\n", mySupervisor_housekeeping_hk.fields.iobcResetCount);
	printf("\t satellite uptime: %lu \r\n", mySupervisor_housekeeping_hk.fields.iobcUptime);

	printf("\n\r SD: \n\r");
	if(!ret)
	{
		/*printf("\t free memory [byte]: %lu \r\n", space.free);
		printf("\t corrupt bytes [byte]: %lu \r\n", space.bad);*/
		printf("\t There are:\n\t %lu bytes total\n\t %lu bytes free\n\t %lu bytes used\n\t %lu bytes bad.\r\n",space.total, space.free, space.used, space.bad);
	}
	else
		printf("\t ERROR %d reading drive \r\n", ret);

	printf("\n\r ADC: \n\r");
	unsigned short adcSamples[8];
	int i;
	int work = ADC_SingleShot(adcSamples);
	if(!work)
	{
		for(i = 0; i < 8; i++)
			ADC_ConvertRaw10bitToMillivolt(adcSamples[i]);
		for(i = 0; i < 8; i++)
			printf("\t ADC channel %d: %d \r\n", i, adcSamples[i]);
	}
	else
		printf("\t ERROR %d reading drive \r\n", work);
	/*int i;
	for(i = 0; i < 10; i++)
		printf("\t ADC channel %d [mV]: %u\r\n", i, (unsigned int)mySupervisor_housekeeping_hk.fields.adcData[i]);*/
	return TRUE;
}

static Boolean WriteAndReadFromFRAM(void){
	print_error(FRAM_start());
	const unsigned char data[] = "hello";
	unsigned char WritenData[];
	unsigned int address = FRAM_getMaxAddress() - sizeof(data) - 10;
	print_error(FRAM_writeAndVerify(data, address, sizeof(data)));
	print_error(FRAM_read(WritenData, address, sizeof(data)));
	if(WritenData == data)
		printf("It write down what you wanted. \r\n");
	else
		printf("It doesn't write down what you wanted. \r\n");
	return TRUE;
}

static Boolean selectAndExecuteOBCDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Print beacon \n\r");
	printf("\t 2) Write and read from FRAM \n\r");
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, 3) == 0);

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
	if(!isis_eps__demo__init() || !InitSDFat())
	{
		return FALSE;
	}
#else
	if(!GomEPSdemoInit() || !InitSDFat())
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
